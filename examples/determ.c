#include <assert.h>
#include <stdio.h>
#include <bitcoin/bitcoin.h>

int main(int argc, char** argv)
{
    bc_deterministic_wallet_t* wallet = bc_create_deterministic_wallet();
    // Set seed.
    if (!bc_deterministic_wallet_set_seed(wallet,
        "a219213f9b12422aa206d988e3e49607"))
        fprintf(stderr, "Error setting seed.");

    // Get an address from wallet...
    bc_data_chunk_t* pubkey =
        bc_deterministic_wallet_generate_public_key(wallet, 2, 0);
    bc_payment_address_t* addr = bc_create_payment_address();
    if (!bc_payment_address_set_public_key(addr, pubkey))
        fprintf(stderr, "Error setting public key.");
    assert(strcmp(bc_payment_address_encoded(addr),
        "1E4vM9q25xsyDwWwdqHUWnwshdWC9PykmL") == 0);
    bc_destroy_payment_address(addr);
    bc_destroy_data_chunk(pubkey);

    bc_destroy_deterministic_wallet(wallet);
    return 0;
}

