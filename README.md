# EOSIO Faucet

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/eosnetworkfoundation/eosio.faucet/blob/main/LICENSE-MIT)
[![Antelope CDT](https://github.com/eosnetworkfoundation/eosio.faucet/actions/workflows/release.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.faucet/actions/workflows/release.yml)
[![Blanc++ Vert](https://github.com/eosnetworkfoundation/eosio.faucet/actions/workflows/ci.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.faucet/actions/workflows/ci.yml)

> Faucet to create account & send tokens to receiver account.

## Quickstart

```bash
# create
cleos push action eosio.faucet create '[myaccount, "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]' -p eosio.faucet

# send tokens (EOS or EVM)
cleos push action eosio.faucet send '["myaccount"]' -p eosio.faucet
cleos push action eosio.faucet send '["0xaa2F34E41B397aD905e2f48059338522D05CA534"]' -p eosio.faucet
```