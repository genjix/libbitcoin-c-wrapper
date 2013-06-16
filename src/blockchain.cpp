#include <bitcoin/bitcoin.h>
#include <bitcoin/bitcoin.hpp>

#include "core.hpp"
#include "blockchain.hpp"

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
        start(prefix, blockchain_started);
}
void bc_leveldb_blockchain_stop(bc_blockchain_t* self)
{
    reinterpret_cast<bc::leveldb_blockchain*>(self->chain)->
        stop();
}

void bc_blockchain_import(bc_blockchain_t* self,
    bc_block_t* import_block, size_t depth,
    bc_blockchain_import_handler_t handle_import, void* user_data)
{
    auto import_finished =
        [handle_import, user_data](const std::error_code& ec)
        {
            bc_error_code_t* lec = 0;
            if (ec)
                lec = new bc_error_code_t{ec};
            handle_import(lec, user_data);
        };
    reinterpret_cast<bc::blockchain*>(self->chain)->
        import(
            *reinterpret_cast<bc::block_type*>(import_block->data),
            depth, import_finished);
}

