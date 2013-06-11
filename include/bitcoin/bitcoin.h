#ifndef LIBBITCOIN_H
#define LIBBITCOIN_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bc_error_code_t bc_error_code_t;
bc_error_code_t* create_error_code(int value);
void bc_destroy_error_code(bc_error_code_t* self);
const char* bc_error_code_message(bc_error_code_t* self);

typedef struct bc_future_t bc_future_t;
bc_future_t* bc_create_future();
void bc_destroy_future(bc_future_t* self);
void bc_future_signal(bc_future_t* self);
int bc_future_wait(bc_future_t* self);

typedef struct bc_threadpool_t bc_threadpool_t;
bc_threadpool_t* bc_create_threadpool();
void bc_destroy_threadpool(bc_threadpool_t* self);
void bc_threadpool_spawn(bc_threadpool_t* self);
void bc_threadpool_stop(bc_threadpool_t* self);
void bc_threadpool_shutdown(bc_threadpool_t* self);
void bc_threadpool_join(bc_threadpool_t* self);

typedef struct {
    uint32_t version;
    uint32_t locktime;
    // Used internally.
    void* data;
} bc_transaction_t;
bc_transaction_t* bc_create_transaction();
void bc_destroy_transaction(bc_transaction_t* self);

typedef struct bc_blockchain_t bc_blockchain_t;
typedef void (*bc_leveldb_blockchain_start_handler_t)(
    bc_error_code_t*, void* user_data);
bc_blockchain_t* bc_create_leveldb_blockchain(bc_threadpool_t* pool);
void bc_destroy_leveldb_blockchain(bc_blockchain_t* self);
void bc_leveldb_blockchain_start(bc_blockchain_t* self,
    const char* prefix,
    bc_leveldb_blockchain_start_handler_t handle_start, void* user_data);
void bc_leveldb_blockchain_stop(bc_blockchain_t* self);

typedef struct bc_hosts_t bc_hosts_t;
bc_hosts_t* bc_create_hosts(bc_threadpool_t* pool);
void bc_destroy_hosts(bc_hosts_t* self);

typedef struct bc_handshake_t bc_handshake_t;
bc_handshake_t* bc_create_handshake(bc_threadpool_t* pool);
void bc_destroy_handshake(bc_handshake_t* self);

typedef struct bc_channel_t bc_channel_t;
typedef void (*bc_channel_receive_transaction_handler_t)(
    bc_error_code_t*, bc_transaction_t*, void* user_data);
void bc_destroy_channel(bc_channel_t* self);
void bc_channel_subscribe_transaction(bc_channel_t* self,
    bc_channel_receive_transaction_handler_t handle_receive, void* user_data);

typedef struct bc_network_t bc_network_t;
bc_network_t* bc_create_network(bc_threadpool_t* pool);
void bc_destroy_network(bc_network_t* self);

typedef struct bc_protocol_t bc_protocol_t;
typedef void (*bc_protocol_channel_handler_t)(
    bc_error_code_t*, bc_channel_t*, void* user_data);
bc_protocol_t* bc_create_protocol(bc_threadpool_t* pool,
    bc_hosts_t* hosts, bc_handshake_t* handshake, bc_network_t* network);
void bc_destroy_protocol(bc_protocol_t* self);
void bc_protocol_subscribe_channel(bc_protocol_t* self,
    bc_protocol_channel_handler_t handle_channel, void* user_data);

typedef struct bc_poller_t bc_poller_t;
bc_poller_t* bc_create_poller(bc_threadpool_t* pool,
    bc_blockchain_t* blockchain);
void bc_destroy_poller(bc_poller_t* self);

typedef struct bc_transaction_pool_t bc_transaction_pool_t;
typedef void (*bc_transaction_pool_confirm_handler_t)(
    bc_error_code_t*, void* user_data);
typedef void (*bc_transaction_pool_store_handler_t)(
    bc_error_code_t*, int unconfirmed[], void* user_data);
bc_transaction_pool_t* bc_create_transaction_pool(bc_threadpool_t* pool,
    bc_blockchain_t* blockchain);
void bc_destroy_transaction_pool(bc_transaction_pool_t* self);
void bc_transaction_pool_start(bc_transaction_pool_t* self);
void bc_transaction_pool_store(bc_transaction_pool_t* self,
    bc_transaction_t* tx,
    bc_transaction_pool_confirm_handler_t handle_confirm,
    void* confirm_user_data,
    bc_transaction_pool_store_handler_t handle_store,
    void* store_user_data);

typedef struct bc_session_t bc_session_t;
typedef void (*bc_session_completion_handler_t)(
    bc_error_code_t*, void* user_data);
bc_session_t* bc_create_session(bc_threadpool_t* pool,
    bc_handshake_t* handshake, bc_protocol_t* protocol,
    bc_blockchain_t* blockchain, bc_poller_t* poller,
    bc_transaction_pool_t* txpool);
void bc_destroy_session(bc_session_t* self);
void bc_session_start(bc_session_t* self,
    bc_session_completion_handler_t handle_start, void* user_data);
void bc_session_stop(bc_session_t* self,
    bc_session_completion_handler_t handle_stop, void* user_data);

#ifdef __cplusplus
}
#endif

#endif
