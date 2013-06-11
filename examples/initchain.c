#include <stdio.h>
#include <bitcoin/bitcoin.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;
    const char* dbpath = argv[1];
    // Threadpool context containing 1 thread.
    bc_threadpool_t* pool = bc_create_threadpool(1);
    // leveldb_blockchain operations execute in pool's thread.
    bc_blockchain_t* chain = bc_create_leveldb_blockchain(pool);
    // Completion handler for starting the leveldb_blockchain.
    // Does nothing.
    void blockchain_start(bc_error_code_t* ec, void* user_data)
    {
        if (ec)
        {
            fprintf(stderr, "Problem starting blockchain: %s\n",
                bc_error_code_message(ec));
            bc_destroy_error_code(ec);
        }
    }
    // Start blockchain with a database path.
    bc_leveldb_blockchain_start(chain, dbpath, blockchain_start, 0);
    // First block is the genesis block.
    bc_block_t* first_block = bc_genesis_block();
    // Completion handler for import method.
    bc_error_code_t* ec;
    bc_future_t* future = bc_create_future();
    void import_finished(bc_error_code_t* cec, void* user_data)
    {
        ec = cec;
        bc_future_signal(future);
    }
    // Import the genesis block at depth 0.
    // Doesn't validate or perform checks on the block.
    bc_blockchain_import(chain, first_block, 0, import_finished, 0);
    // Wait until error_code is set by
    // import_finished completion handler.
    bc_future_wait(future);
    bc_destroy_future(future);
    if (ec)
    {
        fprintf(stderr, "Importing genesis block failed: %s\n",
            bc_error_code_message(ec));
        bc_destroy_error_code(ec);
    }
    bc_hash_digest_t* block_hash = bc_hash_block_header(first_block);
    bc_destroy_block(first_block);
    char* hash_repr = bc_hash_digest_encode_hex(block_hash);
    printf("Imported genesis block %s\n", hash_repr);
    free(hash_repr);
    bc_destroy_hash_digest(block_hash);
    // All threadpools stopping in parallel...
    bc_threadpool_stop(pool);
    // ... Make them all join main thread and wait until they finish.
    bc_threadpool_join(pool);
    // Now safely close leveldb_blockchain.
    bc_leveldb_blockchain_stop(chain);
    bc_destroy_leveldb_blockchain(chain);
    bc_destroy_threadpool(pool);
    return 0;
}

