# EOSIO Faucet

> Faucet to create account & send tokens to receiver account.

## Quickstart

```bash
# create
cleos push action eosio.faucet create '[myaccount, "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]' -p eosio.faucet

# send tokens (EOS or EVM)
cleos push action eosio.faucet send '["myaccount"]' -p eosio.faucet
cleos push action eosio.faucet send '["0xaa2F34E41B397aD905e2f48059338522D05CA534"]' -p eosio.faucet
```