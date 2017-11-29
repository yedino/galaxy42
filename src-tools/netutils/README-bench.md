This is readme:
/src-tools/netutils/README-bench.txt

This is the main plan for benchmark tests.

Needed source:

* download this main Yedino repository eg https://github.com/yedino/galaxy42
* some scripts are currently on https://github.com/yedinocommunity/pictures/
* results (pictures) will be uploaded to https://github.com/yedinocommunity/... and will be used (linked from) wiki https://github.com/yedinocommunity/galaxy42/wiki

# Possible tests

# High level

There will be one big script to run all other tests and graph the results and all.

---

## Crypto only


## Medium level:

TODO

### Low level:

`./tunserver.elf --mode-bench onlycrypto`  
select number of msg-size samples: --range --samples [--loops]  
select number of graphs: `--crypto CRYPTO`, see source code file netmodel.cpp use --crypto -100 to test all of them  
then graph the results with **gnuplot** first: script https://github.com/yedinocommunity/pictures/blob/master/data/crypto_bench/all_crypto_mt_max/Harry_v1/prepare_results.py then gnuplot **gpl_script_all_crypto_mt_max**
(py and gnuplot are there, will be moved here later)  





