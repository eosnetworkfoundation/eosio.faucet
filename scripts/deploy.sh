#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio eosio.faucet EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# deploy
cleos set contract eosio.faucet . eosio.faucet.wasm eosio.faucet.abi
cleos set contract eosio.token ./include/eosio.token eosio.token.wasm eosio.token.abi

# permissions
cleos set account permission eosio.faucet active --add-code

# create tokens
cleos push action eosio.token create '["eosio", "1000000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "1000000000.0000 EOS", "init"]' -p eosio

# init contract
cleos transfer eosio eosio.faucet "10000 EOS"
