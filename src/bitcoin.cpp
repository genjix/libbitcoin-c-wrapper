#include <bitcoin.h>
#include <bitcoin/bitcoin.hpp>
#include <future>

struct bc_error_code_t
{
    std::error_code ec;
    std::string errmsg;
};
bc_error_code_t* create_error_code(int value)
{
    return new bc_error_code_t;
}
void bc_destroy_error_code(bc_error_code_t* self)
{
    delete self;
}
const char* bc_error_code_message(bc_error_code_t* self)
{
    self->errmsg = self->ec.message();
    return self->errmsg.c_str();
}

struct bc_future_t
{
    std::promise<bool> promise;
};
bc_future_t* bc_create_future()
{
    return new bc_future_t;
}
void bc_destroy_future(bc_future_t* self)
{
    delete self;
}
void bc_future_signal(bc_future_t* self)
{
    self->promise.set_value(true);
}
int bc_future_wait(bc_future_t* self)
{
    bool success = self->promise.get_future().get();
    return success ? 1 : 0;
}

struct bc_threadpool_t
{
    bc::threadpool pool;
};
bc_threadpool_t* bc_create_threadpool()
{
    return new bc_threadpool_t;
}
void bc_destroy_threadpool(bc_threadpool_t* self)
{
    delete self;
}
void bc_threadpool_spawn(bc_threadpool_t* self)
{
    self->pool.spawn();
}
void bc_threadpool_stop(bc_threadpool_t* self)
{
    self->pool.stop();
}
void bc_threadpool_shutdown(bc_threadpool_t* self)
{
    self->pool.shutdown();
}
void bc_threadpool_join(bc_threadpool_t* self)
{
    self->pool.join();
}

bc_transaction_t* bc_create_transaction()
{
    return new bc_transaction_t;
}
void bc_destroy_transaction(bc_transaction_t* self)
{
    delete self;
}

struct bc_blockchain_t
{
    bc::blockchain* chain;
};
bc_blockchain_t* bc_create_leveldb_blockchain(bc_threadpool_t* pool)
{
    bc_blockchain_t* self = new bc_blockchain_t;
    self->chain = new bc::leveldb_blockchain(pool->pool);
    return self;
}
void bc_destroy_leveldb_blockchain(bc_blockchain_t* self)
{
    delete self->chain;
    delete self;
}
void bc_leveldb_blockchain_start(bc_blockchain_t* self,
    const char* prefix,
    bc_leveldb_blockchain_start_handler_t handle_start, void* user_data)
{
    auto blockchain_started =
        [handle_start, user_data](const std::error_code& ec)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            handle_start(lec, user_data);
        };
    reinterpret_cast<bc::leveldb_blockchain*>(self->chain)->
        start("database", blockchain_started);
}
void bc_leveldb_blockchain_stop(bc_blockchain_t* self)
{
    reinterpret_cast<bc::leveldb_blockchain*>(self->chain)->
        stop();
}

struct bc_hosts_t
{
    bc::hosts* hosts;
};
bc_hosts_t* bc_create_hosts(bc_threadpool_t* pool)
{
    bc_hosts_t* self = new bc_hosts_t;
    self->hosts = new bc::hosts(pool->pool);
    return self;
}
void bc_destroy_hosts(bc_hosts_t* self)
{
    delete self->hosts;
    delete self;
}

struct bc_handshake_t
{
    bc::handshake* handshake;
};
bc_handshake_t* bc_create_handshake(bc_threadpool_t* pool)
{
    bc_handshake_t* self = new bc_handshake_t;
    self->handshake = new bc::handshake(pool->pool);
    return self;
}
void bc_destroy_handshake(bc_handshake_t* self)
{
    delete self->handshake;
    delete self;
}

struct bc_channel_t
{
    bc::channel_ptr channel;
};
void bc_destroy_channel(bc_channel_t* self)
{
    delete self;
}
void bc_channel_subscribe_transaction(bc_channel_t* self,
    bc_channel_receive_transaction_handler_t handle_receive, void* user_data)
{
    auto recv_tx =
        [handle_receive, user_data](
            const std::error_code& ec, const bc::transaction_type& tx)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            bc_transaction_t* ltx = new bc_transaction_t;
            ltx->data = new bc::transaction_type(tx);
            handle_receive(lec, ltx, user_data);
        };
    self->channel->subscribe_transaction(recv_tx);
}

struct bc_network_t
{
    bc::network* network;
};
bc_network_t* bc_create_network(bc_threadpool_t* pool)
{
    bc_network_t* self = new bc_network_t;
    self->network = new bc::network(pool->pool);
    return self;
}
void bc_destroy_network(bc_network_t* self)
{
    delete self->network;
    delete self;
}

struct bc_protocol_t
{
    bc::protocol* protocol;
};
bc_protocol_t* bc_create_protocol(bc_threadpool_t* pool,
    bc_hosts_t* hosts, bc_handshake_t* handshake, bc_network_t* network)
{
    bc_protocol_t* self = new bc_protocol_t;
    self->protocol = new bc::protocol(pool->pool,
        *hosts->hosts, *handshake->handshake, *network->network);
    return self;
}
void bc_destroy_protocol(bc_protocol_t* self)
{
    delete self->protocol;
    delete self;
}
void bc_protocol_subscribe_channel(bc_protocol_t* self,
    bc_protocol_channel_handler_t handle_channel, void* user_data)
{
    auto new_channel =
        [handle_channel, user_data](
            const std::error_code& ec, bc::channel_ptr channel)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            handle_channel(lec, new bc_channel_t{channel}, user_data);
        };
    self->protocol->subscribe_channel(new_channel);
}

struct bc_poller_t
{
    bc::poller* poller;
};
bc_poller_t* bc_create_poller(bc_threadpool_t* pool,
    bc_blockchain_t* blockchain)
{
    bc_poller_t* self = new bc_poller_t;
    self->poller = new bc::poller(pool->pool, *blockchain->chain);
    return self;
}
void bc_destroy_poller(bc_poller_t* self)
{
    delete self->poller;
    delete self;
}

struct bc_transaction_pool_t
{
    bc::transaction_pool* txpool;
};
bc_transaction_pool_t* bc_create_transaction_pool(bc_threadpool_t* pool,
    bc_blockchain_t* blockchain)
{
    bc_transaction_pool_t* self = new bc_transaction_pool_t;
    self->txpool = new bc::transaction_pool(pool->pool, *blockchain->chain);
    return self;
}
void bc_destroy_transaction_pool(bc_transaction_pool_t* self)
{
    delete self->txpool;
    delete self;
}
void bc_transaction_pool_start(bc_transaction_pool_t* self)
{
    self->txpool->start();
}
void bc_transaction_pool_store(bc_transaction_pool_t* self,
    bc_transaction_t* tx,
    bc_transaction_pool_confirm_handler_t handle_confirm,
    void* confirm_user_data,
    bc_transaction_pool_store_handler_t handle_store,
    void* store_user_data)
{
    auto tx_confirmed =
        [handle_confirm, confirm_user_data](const std::error_code& ec)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            handle_confirm(lec, confirm_user_data);
        };
    auto tx_stored =
        [handle_store, store_user_data](
            const std::error_code& ec, const bc::index_list& unconfirmed)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            int unconfirm[] = { 0, 110, 4 };
            handle_store(lec, unconfirm, store_user_data);
        };
    bc::transaction_type* base_tx =
        reinterpret_cast<bc::transaction_type*>(tx->data);
    self->txpool->store(*base_tx, tx_confirmed, tx_stored);
}

struct bc_session_t
{
    bc::session* session;
};
bc_session_t* bc_create_session(bc_threadpool_t* pool,
    bc_handshake_t* handshake, bc_protocol_t* protocol,
    bc_blockchain_t* blockchain, bc_poller_t* poller,
    bc_transaction_pool_t* txpool)
{
    bc_session_t* self = new bc_session_t;
    self->session = new bc::session(pool->pool, {
        *handshake->handshake, *protocol->protocol, *blockchain->chain,
        *poller->poller, *txpool->txpool});
}
void bc_destroy_session(bc_session_t* self)
{
    delete self;
}
void bc_session_start(bc_session_t* self,
    bc_session_completion_handler_t handle_start, void* user_data)
{
    auto session_started =
        [handle_start, user_data](const std::error_code& ec)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            handle_start(lec, user_data);
        };
    self->session->start(session_started);
}
void bc_session_stop(bc_session_t* self,
    bc_session_completion_handler_t handle_stop, void* user_data)
{
    auto session_stopped =
        [handle_stop, user_data](const std::error_code& ec)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            handle_stop(lec, user_data);
        };
    self->session->stop(session_stopped);
}

