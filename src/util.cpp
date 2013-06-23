#include <bitcoin/bitcoin.h>
#include <bitcoin/bitcoin.hpp>

#include "core.hpp"

struct bc_payment_address_t
{
    bc::payment_address payaddr;
    std::string encoded;
};
bc_payment_address_t* bc_create_payment_address()
{
    return new bc_payment_address_t;
}
void bc_destroy_payment_address(bc_payment_address_t* self)
{
    delete self;
}
int bc_payment_address_set_public_key(bc_payment_address_t* self,
    bc_data_chunk_t* pubkey_data)
{
    return bc::set_public_key(self->payaddr, pubkey_data->data) ? 1 : 0;
}
const char* bc_payment_address_encoded(bc_payment_address_t* self)
{
    self->encoded = self->payaddr.encoded();
    return self->encoded.c_str();
}

