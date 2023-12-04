// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.system/native.hpp>

#include "eosio.faucet.hpp"

[[eosio::action]]
void faucet::send( const string to )
{
    // send EOS tokens to EOS or EVM account
    if ( to.length() <= 12 ) send_eos( to );
    else send_evm( to );

    // track history
    add_history( to );
    prune_history();
    prune_rate_limit();
    add_stats();

}

[[eosio::action]]
void faucet::nonce( const uint64_t nonce )
{
    // do nothing
}

[[eosio::action]]
void faucet::create( const name account, const public_key key )
{
    create_account( account, key );
}

[[eosio::action]]
void faucet::test( const string address )
{
    require_auth( get_self() );
    add_ratelimit(address);
    prune_history();
    prune_rate_limit();
}

void faucet::prune_history()
{
    faucet::history_table history( get_self(), get_self().value );
    int count = 0;
    while ( history.begin() != history.end() ) {
        const auto& row = history.begin();
        const int64_t now = current_time_point().sec_since_epoch();
        const int64_t timestamp = row->timestamp.sec_since_epoch();
        if ( timestamp < (now - TTL_HISTORY) ) {
            history.erase( row );
        } else {
            break;
        }
        count++;
        if ( count >= 10 ) break;
    }
}

void faucet::prune_rate_limit()
{
    faucet::ratelimit_table ratelimit( get_self(), get_self().value );
    int count = 0;
    while ( ratelimit.begin() != ratelimit.end() ) {
        const auto& row = ratelimit.begin();
        const int64_t now = current_time_point().sec_since_epoch();
        const int64_t timestamp = row->last_send_time.sec_since_epoch();
        if ( timestamp < (now - TTL_RATE_LIMIT) ) {
            ratelimit.erase( row );
        } else {
            break;
        }
        count++;
        if ( count >= 10 ) break;
    }
}

void faucet::add_history( const string address )
{
    faucet::history_table history( get_self(), get_self().value );
    auto insert = [&]( auto& row ) {
        row.id = history.available_primary_key();
        row.receiver = address;
        row.timestamp = current_time_point();
    };
    history.emplace( get_self(), insert );
}

void faucet::add_stats()
{
    faucet::stats_table stats( get_self(), get_self().value );
    const int64_t current = (current_time_point().sec_since_epoch() / 3600) * 3600;
    auto insert = [&]( auto& row ) {
        row.timestamp = time_point_sec(current);
        row.counter += 1;
    };
    auto itr = stats.find( current );
    if ( itr == stats.end() ) stats.emplace( get_self(), insert );
    else stats.modify( itr, get_self(), insert );
}

uint64_t faucet::add_ratelimit( const string address )
{
    faucet::ratelimit_table _ratelimit( get_self(), get_self().value );
    auto idx = _ratelimit.get_index<"by.address"_n>();
    auto it = idx.find(to_checksum(address));

    // previous counter used for decrementing quantity
    const uint64_t previous_counter = it->counter;

    auto insert = [&]( auto& row ) {
        const int64_t now = current_time_point().sec_since_epoch();
        // reset last timestamp if exceeds TTL
        if ( TTL_RATE_LIMIT < (now - row.last_send_time.sec_since_epoch()) ) {
            row.last_send_time = current_time_point();
            row.counter = 0;
        }

        const int64_t last = row.last_send_time.sec_since_epoch();
        const int64_t diff = now - last;
        check(diff >= TIMEOUT, "eosio.faucet must wait " + to_string(TIMEOUT) + " seconds");
        check(row.counter < MAX_RECEIVED, "eosio.faucet account has received the maximum allocation of tokens");

        if (it == idx.end() ) row.id = _ratelimit.available_primary_key();
        row.address = address;
        row.counter += 1;
        row.last_send_time = current_time_point();
    };
    auto itr = _ratelimit.find( it->id );
    if ( it == idx.end() ) _ratelimit.emplace( get_self(), insert );
    else _ratelimit.modify( itr, get_self(), insert );
    return previous_counter;
}

void faucet::send_evm( const string address )
{
    const uint64_t counter = add_ratelimit( address );
    const asset quantity = QUANTITY - (DECREMENT * counter) + GAS_FEE;
    const asset balance = token::get_balance( TOKEN, get_self(), EOS.code() );
    check( address.substr(0, 2) == "0x", "eosio.faucet [address] must be a valid EVM address (missing 0x prefix)");
    check( address.length() == 42, "eosio.faucet [address] must be a valid EVM address (too short)");
    check( balance >= quantity, "eosio.faucet is empty, please contact administrator");
    transfer( get_self(), "eosio.evm"_n, {quantity, TOKEN}, address);
}

void faucet::send_eos( const string address )
{
    const uint64_t counter = add_ratelimit( address );
    const name account = name{address};
    const time_point_sec now = current_time_point();
    check( is_account( account ), account.to_string() + " account does not exist" );

    // send assets
    const asset balance = token::get_balance( TOKEN, get_self(), EOS.code() );
    const asset quantity = QUANTITY - (DECREMENT * counter);
    check( balance >= quantity, "eosio.faucet is empty, please contact administrator");
    transfer( get_self(), account, {quantity, TOKEN}, MEMO);
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

// @[[eosio::action]]debug
void faucet::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    faucet::ratelimit_table _ratelimit( get_self(), value );
    faucet::history_table _history( get_self(), value );
    faucet::stats_table _stats( get_self(), value );

    if (table_name == "ratelimit"_n) clear_table( _ratelimit, rows_to_clear );
    else if (table_name == "history"_n) clear_table( _history, rows_to_clear );
    else if (table_name == "stats"_n) clear_table( _stats, rows_to_clear );
    else check(false, "eosio.faucet [table_name] unknown table to clear" );
}

void faucet::create_account( const name account, const public_key key )
{
    std::vector<eosiosystem::key_weight> keys = {{key, 1}};
    eosiosystem::authority owner = {1, keys, {}, {}};
    eosiosystem::native::newaccount_action newaccount( "eosio"_n, { get_self(), "active"_n } );
    eosiosystem::system_contract::buyrambytes_action buyrambytes( "eosio"_n, { get_self(), "active"_n });

    // send assets
    const asset balance = token::get_balance( TOKEN, get_self(), EOS.code() );
    check( balance.amount >= QUANTITY.amount + RAM + NET.amount + CPU.amount, "eosio.faucet is empty, please contact administrator");

    newaccount.send( get_self(), account, owner, owner );
    buyrambytes.send( get_self(), account, RAM );
}

void faucet::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}