#include <bitcoin.h>
#include <bitcoin/bitcoin.hpp>
#include <future>

struct lbc_error_code_t
{
    std::error_code ec;
    std::string errmsg;
};
lbc_error_code_t* create_error_code(int value)
{
    return new lbc_error_code_t;
}
void lbc_destroy_error_code(lbc_error_code_t* self)
{
    delete self;
}
const char* lbc_error_code_message(lbc_error_code_t* self)
{
    self->errmsg = self->ec.message();
    return self->errmsg.c_str();
}

struct lbc_future_t
{
    std::promise<bool> promise;
};
lbc_future_t* lbc_create_future()
{
    return new lbc_future_t;
}
void lbc_destroy_future(lbc_future_t* self)
{
    delete self;
}
void lbc_future_signal(lbc_future_t* self)
{
    self->promise.set_value(true);
}
int lbc_future_wait(lbc_future_t* self)
{
    bool success = self->promise.get_future().get();
    return success ? 1 : 0;
}

struct lbc_threadpool_t
{
    bc::threadpool pool;
};
lbc_threadpool_t* lbc_create_threadpool()
{
    return new lbc_threadpool_t;
}
void lbc_destroy_threadpool(lbc_threadpool_t* self)
{
    delete self;
}
void lbc_threadpool_spawn(lbc_threadpool_t* self)
{
    self->pool.spawn();
}
void lbc_threadpool_stop(lbc_threadpool_t* self)
{
    self->pool.stop();
}
void lbc_threadpool_shutdown(lbc_threadpool_t* self)
{
    self->pool.shutdown();
}
void lbc_threadpool_join(lbc_threadpool_t* self)
{
    self->pool.join();
}

lbc_transaction_t* lbc_create_transaction()
{
    return new lbc_transaction_t;
}
void lbc_destroy_transaction(lbc_transaction_t* self)
{
    delete self;
}

struct lbc_blockchain_t
{
    bc::blockchain* chain;
};
lbc_blockchain_t* lbc_create_leveldb_blockchain(lbc_threadpool_t* pool)
{
    lbc_blockchain_t* self = new lbc_blockchain_t;
    self->chain = new bc::leveldb_blockchain(pool->pool);
    return self;
}
void lbc_destroy_leveldb_blockchain(lbc_blockchain_t* self)
{
    delete self->chain;
    delete self;
}
void lbc_leveldb_blockchain_start(lbc_blockchain_t* self,
    const char* prefix,
    lbc_leveldb_blockchain_start_handler_t handle_start, void* user_data)
{
    auto blockchain_started =
        [handle_start, user_data](const std::error_code& ec)
        {
            lbc_error_code_t* lec = 0;
            if (ec)
                lec = new lbc_error_code_t{ec};
            handle_start(lec, user_data);
        };
    reinterpret_cast<bc::leveldb_blockchain*>(self->chain)->
        start("database", blockchain_started);
}
void lbc_leveldb_blockchain_stop(lbc_blockchain_t* self)
{
    reinterpret_cast<bc::leveldb_blockchain*>(self->chain)->
        stop();
}

struct lbc_hosts_t
{
    bc::hosts* hosts;
};
lbc_hosts_t* lbc_create_hosts(lbc_threadpool_t* pool)
{
    lbc_hosts_t* self = new lbc_hosts_t;
    self->hosts = new bc::hosts(pool->pool);
    return self;
}
void lbc_destroy_hosts(lbc_hosts_t* self)
{
    delete self->hosts;
    delete self;
}

struct lbc_handshake_t
{
    bc::handshake* handshake;
};
lbc_handshake_t* lbc_create_handshake(lbc_threadpool_t* pool)
{
    lbc_handshake_t* self = new lbc_handshake_t;
    self->handshake = new bc::handshake(pool->pool);
    return self;
}
void lbc_destroy_handshake(lbc_handshake_t* self)
{
    delete self->handshake;
    delete self;
}

struct lbc_channel_t
{
    bc::channel_ptr channel;
};
void lbc_destroy_channel(lbc_channel_t* self)
{
    delete self;
}
void lbc_channel_subscribe_transaction(lbc_channel_t* self,
    lbc_channel_receive_transaction_handler_t handle_receive, void* user_data)
{
    auto recv_tx =
        [handle_receive, user_data](
            const std::error_code& ec, const bc::transaction_type& tx)
        {
            lbc_error_code_t* lec = 0;
            if (ec)
                lec = new lbc_error_code_t{ec};
            lbc_transaction_t* ltx = new lbc_transaction_t;
            ltx->data = new bc::transaction_type(tx);
            handle_receive(lec, ltx, user_data);
        };
    self->channel->subscribe_transaction(recv_tx);
}

struct lbc_network_t
{
    bc::network* network;
};
lbc_network_t* lbc_create_network(lbc_threadpool_t* pool)
{
    lbc_network_t* self = new lbc_network_t;
    self->network = new bc::network(pool->pool);
    return self;
}
void lbc_destroy_network(lbc_network_t* self)
{
    delete self->network;
    delete self;
}

struct lbc_protocol_t
{
    bc::protocol* protocol;
};
lbc_protocol_t* lbc_create_protocol(lbc_threadpool_t* pool,
    lbc_hosts_t* hosts, lbc_handshake_t* handshake, lbc_network_t* network)
{
    lbc_protocol_t* self = new lbc_protocol_t;
    self->protocol = new bc::protocol(pool->pool,
        *hosts->hosts, *handshake->handshake, *network->network);
    return self;
}
void lbc_destroy_protocol(lbc_protocol_t* self)
{
    delete self->protocol;
    delete self;
}
void lbc_protocol_subscribe_channel(lbc_protocol_t* self,
    lbc_protocol_channel_handler_t handle_channel, void* user_data)
{
    auto new_channel =
        [handle_channel, user_data](bc::channel_ptr channel)
        {
            handle_channel(new lbc_channel_t{channel}, user_data);
        };
    self->protocol->subscribe_channel(new_channel);
}

struct lbc_poller_t
{
    bc::poller* poller;
};
lbc_poller_t* lbc_create_poller(lbc_threadpool_t* pool,
    lbc_blockchain_t* blockchain)
{
    lbc_poller_t* self = new lbc_poller_t;
    self->poller = new bc::poller(pool->pool, *blockchain->chain);
    return self;
}
void lbc_destroy_poller(lbc_poller_t* self)
{
    delete self->poller;
    delete self;
}

struct lbc_transaction_pool_t
{
    bc::transaction_pool* txpool;
};
lbc_transaction_pool_t* lbc_create_transaction_pool(lbc_threadpool_t* pool,
    lbc_blockchain_t* blockchain)
{
    lbc_transaction_pool_t* self = new lbc_transaction_pool_t;
    self->txpool = new bc::transaction_pool(pool->pool, *blockchain->chain);
    return self;
}
void lbc_destroy_transaction_pool(lbc_transaction_pool_t* self)
{
    delete self->txpool;
    delete self;
}
void lbc_transaction_pool_start(lbc_transaction_pool_t* self)
{
    self->txpool->start();
}
void lbc_transaction_pool_store(lbc_transaction_pool_t* self,
    lbc_transaction_t* tx,
    lbc_transaction_pool_confirm_handler_t handle_confirm,
    void* confirm_user_data,
    lbc_transaction_pool_store_handler_t handle_store,
    void* store_user_data)
{
    auto tx_confirmed =
        [handle_confirm, confirm_user_data](const std::error_code& ec)
        {
            lbc_error_code_t* lec = 0;
            if (ec)
                lec = new lbc_error_code_t{ec};
            handle_confirm(lec, confirm_user_data);
        };
    auto tx_stored =
        [handle_store, store_user_data](
            const std::error_code& ec, const bc::index_list& unconfirmed)
        {
            lbc_error_code_t* lec = 0;
            if (ec)
                lec = new lbc_error_code_t{ec};
            int unconfirm[] = { 0, 110, 4 };
            handle_store(lec, unconfirm, store_user_data);
        };
    bc::transaction_type* base_tx =
        reinterpret_cast<bc::transaction_type*>(tx->data);
    self->txpool->store(*base_tx, tx_confirmed, tx_stored);
}

struct lbc_session_t
{
    bc::session* session;
};
lbc_session_t* lbc_create_session(lbc_threadpool_t* pool,
    lbc_handshake_t* handshake, lbc_protocol_t* protocol,
    lbc_blockchain_t* blockchain, lbc_poller_t* poller,
    lbc_transaction_pool_t* txpool)
{
    lbc_session_t* self = new lbc_session_t;
    self->session = new bc::session(pool->pool, {
        *handshake->handshake, *protocol->protocol, *blockchain->chain,
        *poller->poller, *txpool->txpool});
}
void lbc_destroy_session(lbc_session_t* self)
{
    delete self;
}
void lbc_session_start(lbc_session_t* self,
    lbc_session_completion_handler_t handle_start, void* user_data)
{
    auto session_started =
        [handle_start, user_data](const std::error_code& ec)
        {
            lbc_error_code_t* lec = 0;
            if (ec)
                lec = new lbc_error_code_t{ec};
            handle_start(lec, user_data);
        };
    self->session->start(session_started);
}
void lbc_session_stop(lbc_session_t* self,
    lbc_session_completion_handler_t handle_stop, void* user_data)
{
    auto session_stopped =
        [handle_stop, user_data](const std::error_code& ec)
        {
            lbc_error_code_t* lec = 0;
            if (ec)
                lec = new lbc_error_code_t{ec};
            handle_stop(lec, user_data);
        };
    self->session->stop(session_stopped);
}

