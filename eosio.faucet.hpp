#pragma once

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
    const asset QUANTITY = asset{100'0000, EOS};
    const asset NET = asset{1'0000, EOS};
    const asset CPU = asset{100'0000, EOS};
    const uint32_t RAM = 8000;
    const uint32_t DAY = 86400; // 24 hours
    const string MEMO = "received by EOSIO faucet";

    /**
     * ## TABLE `ratelimit`
     *
     * - `{name} to` - receiver account
     * - `{time_point_sec} next` - next available send
     *
     * ### example
     *
     * ```json
     * {
     *     "account": "name",
     *     "send_at": "2022-07-24T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("ratelimit")]] ratelimit_row {
        name                to;
        time_point_sec      send_at;

        uint64_t primary_key() const { return to.value; }
    };
    typedef eosio::multi_index< "ratelimit"_n, ratelimit_row > ratelimit_table;

    /**
     * ## ACTION `send`
     *
     * > Send tokens to {{to}} receiver account.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} to` - receiver account
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.faucet send '[myaccount]' -p anyaccount
     * ```
     */
    [[eosio::action]]
    void send( const name to );

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
     * - `{public_key} key` - public account to use as active/owner permission
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
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

    // action wrappers
    using send_action = eosio::action_wrapper<"send"_n, &faucet::send>;

private :
    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );

    void create_account( const name account, const public_key key );

    void transfer( const name from, const name to, const extended_asset value, const string& memo );
};