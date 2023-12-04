#pragma once

#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

#include <string>

using namespace eosio;
using namespace std;

class [[eosio::contract("eosio.faucet")]] faucet : public eosio::contract {
public:
    using contract::contract;

    // CONSTANTS
    const name TOKEN = "eosio.token"_n;
    const symbol EOS = symbol{"EOS", 4};
    const asset QUANTITY = asset{1'0000, EOS};
    const asset DECREMENT = asset{0'1000, EOS};
    const asset GAS_FEE = asset{0'0100, EOS};
    const asset NET = asset{1'0000, EOS};
    const asset CPU = asset{1'0000, EOS};
    const uint32_t RAM = 8000;
    const uint32_t TIMEOUT = 60; // 1 minute
    const uint32_t TTL_HISTORY = 600; // 1 hour
    const uint32_t TTL_RATE_LIMIT = 86400; // 24 hours
    const uint32_t MAX_RECEIVED = 10; // max received
    const string MEMO = "received by https://faucet.testnet.evm.eosnetwork.com";

    /**
     * ## TABLE `ratelimit`
     *
     * - `{uint64_t} id` - (primary key) incremental key
     * - `{string} to` - receiver account (EOS or EVM)
     * - `{uint64_t} counter` - counter used to rate limit total actions allowed per time
     * - `{time_point_sec} next` - next available send
     *
     * ### example
     *
     * ```json
     * {
     *     "id": 1,
     *     "address": "aa2F34E41B397aD905e2f48059338522D05CA534",
     *     "counter": 10,
     *     "last_send_time": "2022-07-24T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("ratelimit")]] ratelimit_row {
        uint64_t            id;
        string              address;
        uint64_t            counter;
        time_point_sec      last_send_time;

        checksum256 by_address() const { return to_checksum(address); }
        uint64_t primary_key() const { return id; }
    };
    typedef eosio::multi_index< "ratelimit"_n, ratelimit_row,
        indexed_by<"by.address"_n, const_mem_fun<ratelimit_row, checksum256, &ratelimit_row::by_address>>
    > ratelimit_table;

    static checksum256 to_checksum( string address )
    {
        if ( address.length() > 40 ) address = address.substr(2);
        return sha256(address.c_str(), address.length());
    }

    /**
     * ## TABLE `stats`
     *
     * - `{time_point_sec} timestamp` - timestamp for the stats
     * - `{uint64_t} counter` - counter total send transactions
     *
     * ### example
     *
     * ```json
     * {
     *     "timestamp": "2022-07-24T00:00:00",
     *     "counter": 10,
     * }
     * ```
     */
    struct [[eosio::table("stats")]] stats_row {
        time_point_sec      timestamp;
        uint64_t            counter;

        uint64_t primary_key() const { return timestamp.sec_since_epoch(); }
    };
    typedef eosio::multi_index< "stats"_n, stats_row> stats_table;

    /**
     * ## TABLE `history`
     *
     * - `{name} to` - receiver account
     * - `{time_point_sec} next` - next available send
     *
     * ### example
     *
     * ```json
     * {
     *     "id": "send",
     *     "total": 1000,
     *     "timestamp": "2022-07-24T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("history")]] history_row {
        uint64_t            id;
        string              receiver;
        time_point_sec      timestamp;

        uint64_t primary_key() const { return id; }
    };
    typedef eosio::multi_index< "history"_n, history_row > history_table;

    /**
     * ## ACTION `send`
     *
     * > Send tokens to {{to}} receiver account.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{string} to` - receiver account (EOS or EVM)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.faucet send '["myaccount"]' -p anyaccount
     * $ cleos push action eosio.faucet send '["0xaa2F34E41B397aD905e2f48059338522D05CA534"]' -p anyaccount
     * ```
     */
    [[eosio::action]]
    void send( const string to );

    [[eosio::action]]
    void nonce( const uint64_t nonce );

    /**
     * ## ACTION `create`
     *
     * > Create account using {{key}} as active & owner permission.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} account` - account to be created
     * - `{public_key} key` - EOSIO public key to be used as active/owner permission
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.faucet create '[myaccount, "EOS5uHeBsURAT6bBXNtvwKtWaiDSDJSdSmc96rHVws5M1qqVCkAm6"]' -p anyaccount
     * ```
     */
    [[eosio::action]]
    void create( const name account, const public_key key );

    // @debug
    [[eosio::action]]
    void test( const string address );

    // @debug
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

    // action wrappers
    using send_action = eosio::action_wrapper<"send"_n, &faucet::send>;

private :
    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );

    void create_account( const name account, const public_key key );

    void transfer( const name from, const name to, const extended_asset value, const string& memo );

    void send_evm( const string address );
    void send_eos( const string address );
    uint64_t add_ratelimit( const string address );
    void add_history( const string address );
    void prune_rate_limits();
    void prune_rate_limit( const string address );
    void prune_history();
    void add_stats();
};