#!/bin/bash

# compile
blanc++ eosio.faucet.cpp -I include

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract eosio.faucet . eosio.faucet.wasm eosio.faucet.abi
