# #!/bin/bash

# create
cleos push action eosio.faucet create '[myaccount, "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]' -p eosio.faucet

# send tokens
cleos push action eosio.faucet send '[myaccount]' -p eosio.faucet
