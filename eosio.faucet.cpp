// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.system/native.hpp>

#include "eosio.faucet.hpp"

[[eosio::action]]
void faucet::send( const name to )
{
    faucet::ratelimit_table _ratelimit( get_self(), get_self().value );
    const time_point_sec now = current_time_point();
    auto insert = [&]( auto& row ) {
        check( row.send_at + DAY <= now, "faucet::send: must wait 24 hours");
        row.to = to;
        row.send_at = now;
    };

    // modify or create
    auto itr = _ratelimit.find( to.value );
    if ( itr == _ratelimit.end() ) _ratelimit.emplace( get_self(), insert );
    else _ratelimit.modify( itr, get_self(), insert );

    // send assets
    const asset balance = token::get_balance( TOKEN, get_self(), EOS.code() );
    check( balance >= QUANTITY, "faucet::send: is empty, please contact administrator");
    transfer( get_self(), to, {QUANTITY, TOKEN}, MEMO);
}

[[eosio::action]]
void faucet::create( const name account, const public_key key )
{
    create_account( account, key );
    send( account );
}

// @debug
template <typename T>
void faucet::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

// @debug
[[eosio::action]]
void faucet::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    faucet::ratelimit_table _ratelimit( get_self(), value );

    if (table_name == "ratelimit"_n) clear_table( _ratelimit, rows_to_clear );
    else check(false, "eosio.faucet::cleartable: [table_name] unknown table to clear" );
}

void faucet::create_account( const name account, const public_key key )
{
    std::vector<eosiosystem::key_weight> keys = {{key, 1}};
    eosiosystem::authority owner = {1, keys, {}, {}};
    eosiosystem::native::newaccount_action newaccount( "eosio"_n, { get_self(), "active"_n } );
    eosiosystem::system_contract::buyrambytes_action buyrambytes( "eosio"_n, { get_self(), "active"_n });
    eosiosystem::system_contract::delegatebw_action delegatebw( "eosio"_n, { get_self(), "active"_n });

    // send assets
    const asset balance = token::get_balance( TOKEN, get_self(), EOS.code() );
    check( balance.amount >= QUANTITY.amount + RAM + NET.amount + CPU.amount, "faucet::send: is empty, please contact administrator");

    newaccount.send( get_self(), account, owner, owner );
    buyrambytes.send( get_self(), account, RAM );
    delegatebw.send( get_self(), account, NET, CPU, true );
}

void faucet::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}