This is readme:  
/src-tools/netutils/README-bench.txt

This is the main plan for benchmark tests.

Needed source:

* download this main Yedino repository eg https://github.com/yedino/galaxy42
* some scripts are currently on https://github.com/yedinocommunity/pictures/
* results (pictures) will be uploaded to https://github.com/yedinocommunity/... and will be used (linked from) wiki https://github.com/yedinocommunity/galaxy42/wiki

# Configuration

Example ~/.bashrc

```

### PUBLIC configuration of yedino
# normal LAN
YEDINO_BENCH_PEER_NORM_RPC_IP='192.168.1.66'  # peer that can spam you, over normal LAN - his RPC IP
YEDINO_BENCH_SELF_NORM_WIRE_IP='192.168.1.101' # your IP on the normal LAN, that will be target of the spam
# faster LAN - eg 10 Gbps
YEDINO_BENCH_PEER_FAST_RPC_IP='192.168.1.66'   # peer that can spam you, over 10Gigabit LAN - his RPC IP
YEDINO_BENCH_SELF_FAST_WIRE_IP1='192.168.113.16' # your IP on 10Gigabit LAN, that will be target of the spam, IP 1
YEDINO_BENCH_SELF_FAST_WIRE_IP2='192.168.114.16' # your IP on 10Gigabit LAN, that will be target of the spam, IP 2
# other peer to whom you will forward received data
YEDINO_BENCH_OTHER_WIRE_IP='192.168.1.108'      # other peer, to which you will send some data in some tests
YEDINO_BENCH_RPC_PASS1='' # set this password on all computers participating in network lan tests

### SECRET configuration of yedino
YEDINO_BENCH_RPC_PASS1='nektargruszkawilliams' # set this password on all computers participating in network lan tests


```


# High level

There will be one big script to run all other tests and graph the results and all.

---


# Crypto only

This tests require just 1 computer.

## Medium level:

TODO

### Low level:

E.g.:

`./tunserver.elf --mode-bench onlycrypto range=2 samples=30 crypto=-100 thr=-2`

Try higher range for more detailed X-scale (X = msg size).

Try other crypto to test only 1 selected crypto.

Try other thr to test only given number of threads.

then graph the results with **gnuplot** first: script [https://github.com/yedinocommunity/pictures/blob/master/data/crypto_bench/all_crypto_mt_max/Harry_v1/prepare_results.py] then gnuplot **gpl_script_all_crypto_mt_max**
(py and gnuplot are there, will be moved here later)  


---

## Netmodel

This tests require usually to build some network (though also possible localhost only).

## Medium level

`./test_speed_packet_size.py --help`

## Low level:

not documented yet (check source of medium level)


