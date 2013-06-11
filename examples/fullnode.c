#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bitcoin/bitcoin.h>

typedef struct {
    // Threadpools
    bc_threadpool_t* net_pool;
    bc_threadpool_t* disk_pool;
    bc_threadpool_t* mem_pool;
    // Services
    bc_hosts_t* hosts;
    bc_handshake_t* handshake;
    bc_network_t* network;
    bc_protocol_t* protocol;
    bc_blockchain_t* chain;
    bc_poller_t* poller;
    bc_transaction_pool_t* txpool;
    bc_session_t* session;
} fullnode_t;

fullnode_t* create_fullnode();
void destroy_fullnode(fullnode_t* fullnode);
void fullnode_start(fullnode_t* fullnode);
void fullnode_stop(fullnode_t* fullnode);

// New connection has been started.
// Subscribe to new transaction messages from the network.
void fullnode_connection_started(
    bc_error_code_t* ec, bc_channel_t* node, void* user_data);
// New transaction message from the network.
// Attempt to validate it by storing it in the transaction pool.
void fullnode_recv_tx(bc_error_code_t* ec, bc_transaction_t* tx,
    void* user_data);
// Result of store operation in transaction pool.
void fullnode_new_unconfirm_valid_tx(
    bc_error_code_t* ec, int unconfirmed[], void* user_data);

fullnode_t* create_fullnode()
{
    // Threadpools and the number of threads they spawn.
    // 6 threads spawned in total.
    fullnode_t* fullnode = malloc(sizeof(fullnode_t));
    fullnode->net_pool = bc_create_threadpool();
    fullnode->disk_pool = bc_create_threadpool();
    fullnode->mem_pool = bc_create_threadpool();
    bc_threadpool_spawn(fullnode->net_pool);
    int i;
    for (i = 0; i < 4; ++i)
        bc_threadpool_spawn(fullnode->disk_pool);
    bc_threadpool_spawn(fullnode->mem_pool);
    // Networking related services.
    fullnode->hosts = bc_create_hosts(fullnode->net_pool);
    fullnode->handshake = bc_create_handshake(fullnode->net_pool);
    fullnode->network = bc_create_network(fullnode->net_pool);
    fullnode->protocol = bc_create_protocol(fullnode->net_pool,
        fullnode->hosts, fullnode->handshake, fullnode->network);
    // Blockchain database service.
    fullnode->chain = bc_create_leveldb_blockchain(fullnode->disk_pool);
    // Poll new blocks, and transaction memory pool.
    fullnode->poller = bc_create_poller(fullnode->mem_pool,
        fullnode->chain);
    fullnode->txpool = bc_create_transaction_pool(fullnode->mem_pool,
        fullnode->chain);
    // Session manager service. Convenience wrapper.
    fullnode->session = bc_create_session(fullnode->net_pool,
        fullnode->handshake, fullnode->protocol, fullnode->chain,
        fullnode->poller, fullnode->txpool);
    return fullnode;
}

void destroy_fullnode(fullnode_t* fullnode)
{
    bc_destroy_threadpool(fullnode->net_pool);
    bc_destroy_threadpool(fullnode->disk_pool);
    bc_destroy_threadpool(fullnode->mem_pool);
    bc_destroy_hosts(fullnode->hosts);
    bc_destroy_handshake(fullnode->handshake);
    bc_destroy_network(fullnode->network);
    bc_destroy_leveldb_blockchain(fullnode->chain);
    bc_destroy_poller(fullnode->poller);
    bc_destroy_transaction_pool(fullnode->txpool);
    bc_destroy_session(fullnode->session);
    free(fullnode);
}

void fullnode_start(fullnode_t* fullnode)
{
    // Subscribe to new connections.
    bc_protocol_subscribe_channel(fullnode->protocol,
        fullnode_connection_started, fullnode);
    // Start blockchain. Must finish before any operations
    // are performed on the database (or they will fail).
    bc_error_code_t* ec;
    bc_future_t* future = bc_create_future();
    void blockchain_started(bc_error_code_t* cec, void* user_data)
    {
        ec = cec;
        bc_future_signal(future);
    }
    bc_leveldb_blockchain_start(fullnode->chain,
        "database", blockchain_started, 0);
    bc_future_wait(future);
    bc_destroy_future(future);
    if (ec)
    {
        fprintf(stderr, "Problem starting blockchain: %s\n",
            bc_error_code_message(ec));
        bc_destroy_error_code(ec);
        return;
    }
    // Start transaction pool
    bc_transaction_pool_start(fullnode->txpool);
    // Fire off app.
    void handle_start(bc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "fullnode: %s\n", bc_error_code_message(ec));
            bc_destroy_error_code(ec);
        }
    }
    bc_session_start(fullnode->session, handle_start, 0);
}

void fullnode_stop(fullnode_t* fullnode)
{
    void handle_stop(bc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "fullnode: %s\n", bc_error_code_message(ec));
            bc_destroy_error_code(ec);
        }
    }
    bc_session_stop(fullnode->session, handle_stop, 0);
    // Stop threadpools.
    bc_threadpool_stop(fullnode->net_pool);
    bc_threadpool_stop(fullnode->disk_pool);
    bc_threadpool_stop(fullnode->mem_pool);
    // Join threadpools. Wait for them to finish.
    bc_threadpool_join(fullnode->net_pool);
    bc_threadpool_join(fullnode->disk_pool);
    bc_threadpool_join(fullnode->mem_pool);
    // Safely close blockchain database.
    bc_leveldb_blockchain_stop(fullnode->chain);
}

void fullnode_connection_started(
    bc_error_code_t* ec, bc_channel_t* node, void* user_data)
{
    if (ec)
    {
        fprintf(stderr, "Receive transaction: %s\n",
            bc_error_code_message(ec));
        bc_destroy_error_code(ec);
        return;
    }
    fullnode_t* fullnode = user_data;
    void* tx_user_data = malloc(sizeof(node) + sizeof(fullnode));
    memcpy(tx_user_data, &node, sizeof(node));
    memcpy(tx_user_data + sizeof(node), &fullnode, sizeof(fullnode));
    bc_channel_subscribe_transaction(node, fullnode_recv_tx, tx_user_data);
    // Stay subscribed to new connections.
    bc_protocol_subscribe_channel(fullnode->protocol,
        fullnode_connection_started, fullnode);
}
void fullnode_recv_tx(bc_error_code_t* ec, bc_transaction_t* tx,
    void* user_data)
{
    bc_channel_t* node = *(bc_channel_t**)user_data;
    fullnode_t* fullnode = *(fullnode_t**)(user_data + sizeof(node));
    if (ec)
    {
        fprintf(stderr, "Receive transaction: %s\n",
            bc_error_code_message(ec));
        bc_destroy_error_code(ec);
        bc_destroy_channel(node);
        free(user_data);
        return;
    }
    // Called when the transaction becomes confirmed in a block.
    void handle_confirm(bc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "Confirm error: %s\n",
                bc_error_code_message(ec));
            bc_destroy_error_code(ec);
        }
    }
    // Validate the transaction from the network.
    // Attempt to store in the transaction pool and check the result.
    bc_transaction_pool_store(fullnode->txpool, tx,
        handle_confirm, 0, fullnode_new_unconfirm_valid_tx, tx);
    // Resubscribe to transaction messages from this node.
    bc_channel_subscribe_transaction(node, fullnode_recv_tx, user_data);
}

void fullnode_new_unconfirm_valid_tx(
    bc_error_code_t* ec, int unconfirmed[], void* user_data)
{
    bc_transaction_t* tx = user_data;
    bc_destroy_transaction(tx);
    if (ec)
    {
        fprintf(stderr, "Error storing memory pool transaction %s: %s\n",
            "", bc_error_code_message(ec));
        bc_destroy_error_code(ec);
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

