#include <bitcoin/bitcoin.h>
#include <bitcoin/bitcoin.hpp>

bc_transaction_t* bc_create_transaction()
{
    bc_transaction_t* self = new bc_transaction_t;
    self->data = new bc::transaction_type;
    return self;
}
void bc_destroy_transaction(bc_transaction_t* self)
{
    delete reinterpret_cast<bc::transaction_type*>(self->data);
    delete self;
}

bc_block_t* bc_create_block()
{
    bc_block_t* self = new bc_block_t;
    self->data = new bc::block_type;
    return self;
}
void bc_destroy_block(bc_block_t* self)
{
    delete reinterpret_cast<bc::block_type*>(self->data);
    delete self;
}
bc_block_t* bc_genesis_block()
{
    bc_block_t* self = new bc_block_t;
    self->data = new bc::block_type;
    *reinterpret_cast<bc::block_type*>(self->data) =
        bc::genesis_block();
    return self;
}
bc_hash_digest_t* bc_hash_block_header(bc_block_t* block)
{
    bc_hash_digest_t* result = bc_create_hash_digest();
    bc::hash_digest block_hash = bc::hash_block_header(
        *reinterpret_cast<bc::block_type*>(block->data));
    std::copy(block_hash.begin(), block_hash.end(), result);
    return result;
}

