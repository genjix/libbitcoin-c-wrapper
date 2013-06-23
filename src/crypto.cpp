#include <bitcoin/bitcoin.h>
#include <bitcoin/bitcoin.hpp>

#include "core.hpp"

struct bc_deterministic_wallet_t
{
    bc::deterministic_wallet wallet;
    std::string seed;
};
bc_deterministic_wallet_t* bc_create_deterministic_wallet()
{
    return new bc_deterministic_wallet_t;
}
void bc_destroy_deterministic_wallet(bc_deterministic_wallet_t* self)
{
    delete self;
}
void bc_deterministic_wallet_new_seed(bc_deterministic_wallet_t* self)
{
    self->wallet.new_seed();
}
int bc_deterministic_wallet_set_seed(bc_deterministic_wallet_t* self,
    const char* seed)
{
    assert(strlen(seed) == 32);
    return self->wallet.set_seed(std::string(seed, 32)) ? 1 : 0;
}
const char* bc_deterministic_wallet_seed(bc_deterministic_wallet_t* self)
{
    self->seed = self->wallet.seed();
    return self->seed.c_str();
}
int bc_deterministic_wallet_set_master_public_key(
    bc_deterministic_wallet_t* self, const bc_data_chunk_t* mpk)
{
    return self->wallet.set_master_public_key(mpk->data) ? 1 : 0;
}
bc_data_chunk_t* bc_deterministic_wallet_master_public_key(
    bc_deterministic_wallet_t* self)
{
    bc::data_chunk mpk = self->wallet.master_public_key();
    return new bc_data_chunk_t{mpk};
}
bc_data_chunk_t* bc_deterministic_wallet_generate_public_key(
    bc_deterministic_wallet_t* self, size_t n, int for_change)
{
    bc::data_chunk pubkeydat =
        self->wallet.generate_public_key(n, for_change);
    return new bc_data_chunk_t{pubkeydat};
}

