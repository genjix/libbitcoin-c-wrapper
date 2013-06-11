#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bitcoin.h>

typedef struct {
    // Threadpools
    lbc_threadpool_t* net_pool;
    lbc_threadpool_t* disk_pool;
    lbc_threadpool_t* mem_pool;
    // Services
    lbc_hosts_t* hosts;
    lbc_handshake_t* handshake;
    lbc_network_t* network;
    lbc_protocol_t* protocol;
    lbc_blockchain_t* chain;
    lbc_poller_t* poller;
    lbc_transaction_pool_t* txpool;
    lbc_session_t* session;
} fullnode_t;

fullnode_t* create_fullnode();
void destroy_fullnode(fullnode_t* fullnode);
void fullnode_start(fullnode_t* fullnode);
void fullnode_stop(fullnode_t* fullnode);

// New connection has been started.
// Subscribe to new transaction messages from the network.
void fullnode_connection_started(lbc_channel_t* node, void* user_data);
// New transaction message from the network.
// Attempt to validate it by storing it in the transaction pool.
void fullnode_recv_tx(lbc_error_code_t* ec, lbc_transaction_t* tx,
    void* user_data);
// Result of store operation in transaction pool.
void fullnode_new_unconfirm_valid_tx(
    lbc_error_code_t* ec, int unconfirmed[], void* user_data);

fullnode_t* create_fullnode()
{
    // Threadpools and the number of threads they spawn.
    // 6 threads spawned in total.
    fullnode_t* fullnode = malloc(sizeof(fullnode_t));
    fullnode->net_pool = lbc_create_threadpool();
    fullnode->disk_pool = lbc_create_threadpool();
    fullnode->mem_pool = lbc_create_threadpool();
    lbc_threadpool_spawn(fullnode->net_pool);
    int i;
    for (i = 0; i < 4; ++i)
        lbc_threadpool_spawn(fullnode->disk_pool);
    lbc_threadpool_spawn(fullnode->mem_pool);
    // Networking related services.
    fullnode->hosts = lbc_create_hosts(fullnode->net_pool);
    fullnode->handshake = lbc_create_handshake(fullnode->net_pool);
    fullnode->network = lbc_create_network(fullnode->net_pool);
    fullnode->protocol = lbc_create_protocol(fullnode->net_pool,
        fullnode->hosts, fullnode->handshake, fullnode->network);
    // Blockchain database service.
    fullnode->chain = lbc_create_leveldb_blockchain(fullnode->disk_pool);
    // Poll new blocks, and transaction memory pool.
    fullnode->poller = lbc_create_poller(fullnode->mem_pool,
        fullnode->chain);
    fullnode->txpool = lbc_create_transaction_pool(fullnode->mem_pool,
        fullnode->chain);
    // Session manager service. Convenience wrapper.
    fullnode->session = lbc_create_session(fullnode->net_pool,
        fullnode->handshake, fullnode->protocol, fullnode->chain,
        fullnode->poller, fullnode->txpool);
    return fullnode;
}

void destroy_fullnode(fullnode_t* fullnode)
{
    lbc_destroy_threadpool(fullnode->net_pool);
    lbc_destroy_threadpool(fullnode->disk_pool);
    lbc_destroy_threadpool(fullnode->mem_pool);
    lbc_destroy_hosts(fullnode->hosts);
    lbc_destroy_handshake(fullnode->handshake);
    lbc_destroy_network(fullnode->network);
    lbc_destroy_leveldb_blockchain(fullnode->chain);
    lbc_destroy_poller(fullnode->poller);
    lbc_destroy_transaction_pool(fullnode->txpool);
    lbc_destroy_session(fullnode->session);
    free(fullnode);
}

void fullnode_start(fullnode_t* fullnode)
{
    // Subscribe to new connections.
    lbc_protocol_subscribe_channel(fullnode->protocol,
        fullnode_connection_started, fullnode);
    // Start blockchain. Must finish before any operations
    // are performed on the database (or they will fail).
    lbc_error_code_t* ec;
    lbc_future_t* future = lbc_create_future();
    void blockchain_started(lbc_error_code_t* cec, void* user_data)
    {
        ec = cec;
        lbc_future_signal(future);
    }
    lbc_leveldb_blockchain_start(fullnode->chain,
        "database", blockchain_started, 0);
    lbc_future_wait(future);
    lbc_destroy_future(future);
    if (ec)
    {
        fprintf(stderr, "Problem starting blockchain: %s\n",
            lbc_error_code_message(ec));
        lbc_destroy_error_code(ec);
        return;
    }
    // Start transaction pool
    lbc_transaction_pool_start(fullnode->txpool);
    // Fire off app.
    void handle_start(lbc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "fullnode: %s\n", lbc_error_code_message(ec));
            lbc_destroy_error_code(ec);
        }
    }
    lbc_session_start(fullnode->session, handle_start, 0);
}

void fullnode_stop(fullnode_t* fullnode)
{
    void handle_stop(lbc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "fullnode: %s\n", lbc_error_code_message(ec));
            lbc_destroy_error_code(ec);
        }
    }
    lbc_session_stop(fullnode->session, handle_stop, 0);
    // Stop threadpools.
    lbc_threadpool_stop(fullnode->net_pool);
    lbc_threadpool_stop(fullnode->disk_pool);
    lbc_threadpool_stop(fullnode->mem_pool);
    // Join threadpools. Wait for them to finish.
    lbc_threadpool_join(fullnode->net_pool);
    lbc_threadpool_join(fullnode->disk_pool);
    lbc_threadpool_join(fullnode->mem_pool);
    // Safely close blockchain database.
    lbc_leveldb_blockchain_stop(fullnode->chain);
}

void fullnode_connection_started(lbc_channel_t* node, void* user_data)
{
    fullnode_t* fullnode = user_data;
    void* tx_user_data = malloc(sizeof(node) + sizeof(fullnode));
    memcpy(tx_user_data, &node, sizeof(node));
    memcpy(tx_user_data + sizeof(node), &fullnode, sizeof(fullnode));
    lbc_channel_subscribe_transaction(node, fullnode_recv_tx, tx_user_data);
    // Stay subscribed to new connections.
    lbc_protocol_subscribe_channel(fullnode->protocol,
        fullnode_connection_started, fullnode);
}
void fullnode_recv_tx(lbc_error_code_t* ec, lbc_transaction_t* tx,
    void* user_data)
{
    lbc_channel_t* node = *(lbc_channel_t**)user_data;
    fullnode_t* fullnode = *(fullnode_t**)(user_data + sizeof(node));
    if (ec)
    {
        fprintf(stderr, "Receive transaction: %s\n",
            lbc_error_code_message(ec));
        lbc_destroy_error_code(ec);
        lbc_destroy_channel(node);
        free(user_data);
        return;
    }
    // Called when the transaction becomes confirmed in a block.
    void handle_confirm(lbc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "Confirm error: %s\n",
                lbc_error_code_message(ec));
            lbc_destroy_error_code(ec);
        }
    }
    // Validate the transaction from the network.
    // Attempt to store in the transaction pool and check the result.
    lbc_transaction_pool_store(fullnode->txpool, tx,
        handle_confirm, 0, fullnode_new_unconfirm_valid_tx, tx);
    // Resubscribe to transaction messages from this node.
    lbc_channel_subscribe_transaction(node, fullnode_recv_tx, user_data);
}

void fullnode_new_unconfirm_valid_tx(
    lbc_error_code_t* ec, int unconfirmed[], void* user_data)
{
    lbc_transaction_t* tx = user_data;
    lbc_destroy_transaction(tx);
    if (ec)
    {
        fprintf(stderr, "Error storing memory pool transaction %s: %s\n",
            "", lbc_error_code_message(ec));
        lbc_destroy_error_code(ec);
        return;
    }
}

int main()
{
    fullnode_t* app = create_fullnode();
    fullnode_start(app);
    getchar();
    fullnode_stop(app);
    destroy_fullnode(app);
    return 0;
}

