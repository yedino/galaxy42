#!/usr/bin/env python3
import sys
import subprocess
import socket
import os
import time
import argparse
import multiprocessing
import signal
from subprocess import check_output, DEVNULL, Popen


PASSWORD = 'secret1'
TCP_PORT = 9011
UDP_PORT = 9000
BUFFER_SIZE = 1024
TEST_TIME_IN_SEC = 5
MESSAGE = '{} SEND {} {} 999000 foo {} -1 {} -1\n'
PROGRAM = '../../build/tunserver.elf'
PROGRAM_ARGS = [PROGRAM, '--mode-bench', '192.168.1.107', '2121', 'crypto=0', 'wire_buf=100', 'wire_sock=1',
                'wire_ios=1', 'wire_ios_thr=2', 'tuntap_weld=1', 'tuntap_sock=1', 'tuntap_ios=1', 'tuntap_ios_thr=1',
                'tuntap_weld_sleep=1', 'tuntap_block', 'mt_strand', 'mport']
OUTPUT_FILE = 'speed_results.txt'


def run_program():
    print('Running program with command: ', ' '.join(PROGRAM_ARGS))
    return Popen(PROGRAM_ARGS, stderr=DEVNULL, stdout=DEVNULL)


def send_tcp_message(ip, port, size, host, host_port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, host_port))
    program_process = run_program()
    msg = MESSAGE.format(PASSWORD, ip, port, size, TEST_TIME_IN_SEC)
    print(msg)
    s.send(msg.encode())
    data = s.recv(BUFFER_SIZE)
    # time.sleep(TEST_TIME_IN_SEC)
    data = s.recv(BUFFER_SIZE)
    s.close()
    # program_process.kill()
    program_process.send_signal(signal.SIGINT)
    time.sleep(1)
    with open('/tmp/result.txt') as f:
        result = f.read()
        print(result)
        return result # data.decode('utf-8')


def parse_args():
    arg_parser = argparse.ArgumentParser(description='Script to test client/server program speed depends on size '
                                                     'of packet\n Before run script you have to build '
                                                     'program first')
    arg_parser.add_argument('--host', type=str, required=True, help='address ipv4 of host')
    arg_parser.add_argument('--my_ip', type=str, required=True, help='my address ipv4')
    arg_parser.add_argument('-p', '--port', type=int, default=UDP_PORT, help='port number of asio server')
    arg_parser.add_argument('--host_port', type=int, default=TCP_PORT, help='tcp port number')
    return arg_parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    # sizes = set(list(range(10, 2000, 10)) + list(range(2000, 9200, 100)))
    sizes = sorted(set(list(range(100, 1501, 100))))
    with open(OUTPUT_FILE, 'a') as file:
        for i in sizes:
            response = send_tcp_message(args.my_ip, args.port, i, args.host, args.host_port)
            file.write(str(i))
            file.write('\t')
            file.write(response)
            file.write('\n')
