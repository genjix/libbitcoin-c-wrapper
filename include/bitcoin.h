#ifndef LIBBITCOIN_H
#define LIBBITCOIN_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lbc_error_code_t lbc_error_code_t;
lbc_error_code_t* create_error_code(int value);
void lbc_destroy_error_code(lbc_error_code_t* self);
const char* lbc_error_code_message(lbc_error_code_t* self);

typedef struct lbc_future_t lbc_future_t;
lbc_future_t* lbc_create_future();
void lbc_destroy_future(lbc_future_t* self);
void lbc_future_signal(lbc_future_t* self);
int lbc_future_wait(lbc_future_t* self);

typedef struct lbc_threadpool_t lbc_threadpool_t;
lbc_threadpool_t* lbc_create_threadpool();
void lbc_destroy_threadpool(lbc_threadpool_t* self);
void lbc_threadpool_spawn(lbc_threadpool_t* self);
void lbc_threadpool_stop(lbc_threadpool_t* self);
void lbc_threadpool_shutdown(lbc_threadpool_t* self);
void lbc_threadpool_join(lbc_threadpool_t* self);

typedef struct {
    uint32_t version;
    uint32_t locktime;
    // Used internally.
    void* data;
} lbc_transaction_t;
lbc_transaction_t* lbc_create_transaction();
void lbc_destroy_transaction(lbc_transaction_t* self);

typedef struct lbc_blockchain_t lbc_blockchain_t;
typedef void (*lbc_leveldb_blockchain_start_handler_t)(
    lbc_error_code_t*, void* user_data);
lbc_blockchain_t* lbc_create_leveldb_blockchain(lbc_threadpool_t* pool);
void lbc_destroy_leveldb_blockchain(lbc_blockchain_t* self);
void lbc_leveldb_blockchain_start(lbc_blockchain_t* self,
    const char* prefix,
    lbc_leveldb_blockchain_start_handler_t handle_start, void* user_data);
void lbc_leveldb_blockchain_stop(lbc_blockchain_t* self);

typedef struct lbc_hosts_t lbc_hosts_t;
lbc_hosts_t* lbc_create_hosts(lbc_threadpool_t* pool);
void lbc_destroy_hosts(lbc_hosts_t* self);

typedef struct lbc_handshake_t lbc_handshake_t;
lbc_handshake_t* lbc_create_handshake(lbc_threadpool_t* pool);
void lbc_destroy_handshake(lbc_handshake_t* self);

typedef struct lbc_channel_t lbc_channel_t;
typedef void (*lbc_channel_receive_transaction_handler_t)(
    lbc_error_code_t*, lbc_transaction_t*, void* user_data);
void lbc_destroy_channel(lbc_channel_t* self);
void lbc_channel_subscribe_transaction(lbc_channel_t* self,
    lbc_channel_receive_transaction_handler_t handle_receive, void* user_data);

typedef struct lbc_network_t lbc_network_t;
lbc_network_t* lbc_create_network(lbc_threadpool_t* pool);
void lbc_destroy_network(lbc_network_t* self);

typedef struct lbc_protocol_t lbc_protocol_t;
typedef void (*lbc_protocol_channel_handler_t)(lbc_channel_t*, void* user_data);
lbc_protocol_t* lbc_create_protocol(lbc_threadpool_t* pool,
    lbc_hosts_t* hosts, lbc_handshake_t* handshake, lbc_network_t* network);
void lbc_destroy_protocol(lbc_protocol_t* self);
void lbc_protocol_subscribe_channel(lbc_protocol_t* self,
    lbc_protocol_channel_handler_t handle_channel, void* user_data);

typedef struct lbc_poller_t lbc_poller_t;
lbc_poller_t* lbc_create_poller(lbc_threadpool_t* pool,
    lbc_blockchain_t* blockchain);
void lbc_destroy_poller(lbc_poller_t* self);

typedef struct lbc_transaction_pool_t lbc_transaction_pool_t;
typedef void (*lbc_transaction_pool_confirm_handler_t)(
    lbc_error_code_t*, void* user_data);
typedef void (*lbc_transaction_pool_store_handler_t)(
    lbc_error_code_t*, int unconfirmed[], void* user_data);
lbc_transaction_pool_t* lbc_create_transaction_pool(lbc_threadpool_t* pool,
    lbc_blockchain_t* blockchain);
void lbc_destroy_transaction_pool(lbc_transaction_pool_t* self);
void lbc_transaction_pool_start(lbc_transaction_pool_t* self);
void lbc_transaction_pool_store(lbc_transaction_pool_t* self,
    lbc_transaction_t* tx,
    lbc_transaction_pool_confirm_handler_t handle_confirm,
    void* confirm_user_data,
    lbc_transaction_pool_store_handler_t handle_store,
    void* store_user_data);

typedef struct lbc_session_t lbc_session_t;
typedef void (*lbc_session_completion_handler_t)(
    lbc_error_code_t*, void* user_data);
lbc_session_t* lbc_create_session(lbc_threadpool_t* pool,
    lbc_handshake_t* handshake, lbc_protocol_t* protocol,
    lbc_blockchain_t* blockchain, lbc_poller_t* poller,
    lbc_transaction_pool_t* txpool);
void lbc_destroy_session(lbc_session_t* self);
void lbc_session_start(lbc_session_t* self,
    lbc_session_completion_handler_t handle_start, void* user_data);
void lbc_session_stop(lbc_session_t* self,
    lbc_session_completion_handler_t handle_stop, void* user_data);

#ifdef __cplusplus
}
#endif

#endif

