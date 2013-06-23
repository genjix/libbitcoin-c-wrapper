#include <bitcoin/bitcoin.h>
#include <bitcoin/bitcoin.hpp>
#include <future>

#include "core.hpp"

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

bc_hash_digest_t* bc_create_hash_digest()
{
    return new bc_hash_digest_t[BC_HASH_DIGEST_LENGTH];
}
void bc_destroy_hash_digest(bc_hash_digest_t* self)
{
    delete [] self;
}
char* bc_hash_digest_encode_hex(bc_hash_digest_t* self)
{
    BITCOIN_ASSERT(sizeof(char) == 1);
    char* result = (char*)malloc(BC_HASH_DIGEST_LENGTH * 2 + 1);
    bc::hash_digest tmp_hash;
    std::copy(self, self + BC_HASH_DIGEST_LENGTH, tmp_hash.begin());
    std::string repr = bc::encode_hex(tmp_hash);
    std::copy(repr.begin(), repr.end(), result);
    result[BC_HASH_DIGEST_LENGTH * 2] = '\0';
    return result;
}

bc_data_chunk_t* bc_create_data_chunk()
{
    return new bc_data_chunk_t;
}
void bc_destroy_data_chunk(bc_data_chunk_t* self)
{
    delete self;
}
uint8_t* bc_data_chunk_data(bc_data_chunk_t* self)
{
    return self->data.data();
}
size_t bc_data_chunk_size(bc_data_chunk_t* self)
{
    return self->data.size();
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

bc_threadpool_t* bc_create_threadpool(size_t number_threads)
{
    return new bc_threadpool_t(number_threads);
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

