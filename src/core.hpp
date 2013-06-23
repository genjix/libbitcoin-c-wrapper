#ifndef LIBBITCOIN_CORE_HPP
#define LIBBITCOIN_CORE_HPP

struct bc_error_code_t
{
    std::error_code ec;
    std::string errmsg;
};

struct bc_threadpool_t
{
    bc_threadpool_t(size_t number_threads)
      : pool(number_threads) {}
    bc::threadpool pool;
};

struct bc_data_chunk_t
{
    bc::data_chunk data;
};

#endif

