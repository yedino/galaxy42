#!/usr/bin/env python3

import sys
import subprocess
import socket
import os
import time
import argparse
import multiprocessing
import signal
import threading
from subprocess import check_output, DEVNULL, Popen

PASSWORD = 'lovecraft_freaks_racetrack' # TODO move to runtime

# 1. Computer R1 will spam us. This computer must have running sender-programs in remote mode (waiting on different port numbers for RPC)
# 2. We start local receiver program (netmodel), and tell remote sender(s) to spam us
# 3. We parse result of local receiver to know the speed
# 4. Repeat all above for various parameters (e.g. datagram size)

RPC_TCP_PORT_BASE = 9011 # the remote sender-program(s) that are waiting for RPC command to start spamming us: base port (increasing for each next program if several are available)
LOCAL_P2P_PORT = 9000 # on which port is our receiver program listening for the P2P flood
TEST_TIME_IN_SEC = 10 # how long 1 test should take
#TEST_TIME_IN_SEC = 20
MESSAGE = '{} SEND {} {} 999000 foo {} -1 {} -1\n'
PROGRAM_NETMODEL = '../../build/tunserver.elf'
PROGRAM_NETMODEL_ARGS = [PROGRAM_NETMODEL, '--mode-bench', '192.168.1.107', '2121', 'crypto=0', 'wire_buf=100', 'wire_sock=1',
                'wire_ios=1', 'wire_ios_thr=2', 'tuntap_weld=1', 'tuntap_sock=1', 'tuntap_ios=1', 'tuntap_ios_thr=1',
                'tuntap_weld_sleep=1', 'tuntap_block', 'mt_strand', 'mport']
OUTPUT_FILE = 'speed_results.txt'

RPC_BUFFER_SIZE = 1024 # for RPC text

def run_recv_program():
    print('Running netmodel program with command: ', ' '.join(PROGRAM_NETMODEL_ARGS))
    ret = Popen(PROGRAM_NETMODEL_ARGS, stderr=DEVNULL, stdout=DEVNULL)
    time.sleep(1)
    return ret


def rpc_test_remote_one_sender(thread_nr, size,  this_target_ip, this_target_port, rpc_ip, rpc_port ):
    # for each RPC remote sending program:
    thread_name = "[RCP request thread #" + str(thread_nr) + "]"; 
    # print(thread_name + " thread started");
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # RPC socket
    print ( "RPC: ip=" + rpc_ip + " port=" + str(rpc_port) )
    s.connect( (rpc_ip, rpc_port) )
    msg = MESSAGE.format(PASSWORD, this_target_ip, this_target_port, size, TEST_TIME_IN_SEC)
    print(thread_name + " will send command [" + msg.rstrip() + '] to ' + rpc_ip + ':' + str(rpc_port))
    s.send(msg.encode())
    data = s.recv(RPC_BUFFER_SIZE) # he should reply that he started
    # print(thread_name + " RPC replied (start?) : [" + data.decode('utf-8') + "]" )
    # time.sleep(TEST_TIME_IN_SEC)
    data = s.recv(RPC_BUFFER_SIZE)
    # print(thread_name + " RPC replied (end?) : [" + data.decode('utf-8') + "]" )
    s.close()
    print(thread_name + " RPC connection ended");
    # recv_process.kill()


def rpc_test_remote_all(size, cmd_original):
    # help='my_p2p_addr:port,rpc:rpc_port');
    
    cmdP = [] # parsed command

    for ix,cmd in enumerate(cmd_original):
      parts = cmd.split(',')
      data = [ ['x','x'] , ['x','x'] ]

      for i in range(0,2):
        if parts[i]=='sameend':
          data[i][0]='sameip'
          data[i][1]='sameport'
        else:
           data[i][0] = parts[i].split(':')[0]
           data[i][1] = parts[i].split(':')[1]
      cmdP.append(data)

    # print(cmdP)

    for ix,cmd in enumerate(cmdP):
      for e in range(0,2): # e = endpoint 0 (e.g. taget) or endpoint 1 (e.g. rpc)
        for p in range(0,2): # p = part 0 (ip) or part 1 (port)
          wildcard = ['sameip','sameport'][p]
          if cmdP[ix][e][p]==wildcard:
            back=ix
            while (cmdP[back][e][p] == wildcard):
              back=back-1
            cmdP[ix][e][p] = cmdP[back][e][p]
          
    print("\nParsed:\n")
    print(cmdP)

    thread_count = len(cmdP)
    thread_table = []

    recv_process = run_recv_program() # start local reciving program

    for i in range(0, thread_count ):
	    cmd = cmdP[i]
	    thr = threading.Thread(
        target=rpc_test_remote_one_sender,
        args=(i, size,  cmd[0][0], int(cmd[0][1]), cmd[1][0], int(cmd[1][1])  )
      )
	    thr.start()
	    thread_table.append(thr)	

    for i in range(0, thread_count):
	    #print( "joining thread " + str(i))
	    thread_table[i].join()

    print("All " + str(thread_count) + " RPC client(s) done, sending is done.\n")
    recv_process.send_signal(signal.SIGINT)

    time.sleep(1)
    with open('/tmp/result.txt') as f:
        result = f.read()
        print(result)
        return result # data.decode('utf-8')


def parse_args():
    arg_parser = argparse.ArgumentParser(description='Script to test client/server program speed depends on size '
                                                     'of packet\n Before run script you have to build '
                                                     'program first')
    arg_parser.add_argument('--cmd', action='append', type=str, required=True,
    	help='my_p2p_addr:port,rpc:rpc_port');

    # arg_parser.add_argument('--rpc', type=str, required=True, help='address (or many addresses separated with comma "," character) ipv4 of RPC sender to connect to (the asio send tool waiting for remote command)')
    # arg_parser.add_argument('--rpc_port_base', type=int, default=RPC_TCP_PORT_BASE, help='TCP port number (for RPC, see --rpc)')
    # arg_parser.add_argument('--my_p2p_ip', type=str, required=True, help='my address ipv4 (that will be spammed)')
    # arg_parser.add_argument('--my_p2p_port', '--port', type=int, default=LOCAL_P2P_PORT, help='my UDP port number of asio server (that will be spammed)')
    return arg_parser.parse_args()

def myrange(low,high,mod):
  low2 = (low//mod)*mod
  high2 = (high//mod)*mod
  return range(low2, high2, mod)

if __name__ == "__main__":
    args = parse_args()
    # sizes = sorted(  set(list(range(64, 2000, 16)) + list(range(2000, 9500, 64)))  )
    # sizes = sorted(set(list(range(64, 9500, 64))))
    sizes = sorted(set(
      list(myrange(0, 1500, 64))
      + list(myrange(1500,9000,128))
      + list([1472,8972])
    ))

    sizes = list( reversed( sorted( sizes ) ) )

    print("\n\nWill test sizes: ")
    print(sizes)
    print("\n\n")

    with open(OUTPUT_FILE, 'a') as file:
        for i in sizes:
            response = rpc_test_remote_all(i, args.cmd)
            file.write(str(i))
            file.write('\t')
            file.write(response)
            file.write('\n')
