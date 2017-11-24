#!/usr/bin/env python3
import sys
import subprocess
import socket
import os
import time
import argparse
import multiprocessing
import signal
from subprocess import DEVNULL, Popen

UDP_PORT = 9000
TEST_TIME_IN_SEC = 20
OUTPUT_FILE = 'tuntap_results.txt'
TMP_FILE = '/tmp/tuntap_test.txt'
RECEIVER = '../../build/tunserver.elf'
RECEIVER_ARGS = [RECEIVER, '--mode-bench', '192.168.1.107', '2121', 'crypto=0', 'wire_buf=100', 'wire_sock=1',
                'wire_ios=1', 'wire_ios_thr=2', 'tuntap_sock=1', 'tuntap_ios=1', 'tuntap_weld_sleep=1', 'tuntap_block',
                'mt_strand', 'mport', 'tuntap_use_real=1', 'tuntap_async=0']
SENDER = '../netutils/asio_send_block_ipv6/asio_send_block_ipv6'

def run_receiver(stdout_file, number_of_welds, number_of_threads):
    receiver_args = RECEIVER_ARGS[:]
    receiver_args.append('tuntap_weld=' + str(number_of_welds))
    receiver_args.append('tuntap_ios_thr=' + str(number_of_threads))
    print('Running program with command: ', ' '.join(receiver_args))
    return Popen(receiver_args, stderr=DEVNULL, stdout=stdout_file)


def run_sender(packet_size):
    sender_args = [SENDER, str(packet_size)]
    print('Running program with command: ', ' '.join(sender_args))
    return Popen(sender_args, stderr=DEVNULL, stdout=DEVNULL)


def parse_args():
    arg_parser = argparse.ArgumentParser(description='Script to test client/server program speed depends on size '
                                                     'of packet, threads count and weld count\n Before run script you '
                                                     'have to build program first')
    arg_parser.add_argument('-t', '--time', type=int, default=TEST_TIME_IN_SEC, help='Time of single test in seconds')
    return arg_parser.parse_args()


def single_test(output_file, packet_size, number_of_welds, number_of_threads):
    with open(TMP_FILE, 'w') as tmp_file:
        receiver = run_receiver(tmp_file, number_of_welds, number_of_threads)
        time.sleep(2)
        sender = run_sender(packet_size)
        time.sleep(args.time)
        sender.send_signal(signal.SIGINT)
        receiver.send_signal(signal.SIGINT)
        receiver.send_signal(signal.SIGTERM)
        time.sleep(1)
    with open(TMP_FILE, 'r') as tmp_file:
        iter = 0
        results = []
        for line in tmp_file.readlines():
            iter += 1
            if iter > 5 and float(line) != 0:
                results.append(float(line))
        print(results)
        output_file.write(str(size))
        output_file.write('\t')
        output_file.write(str(sum(results[:-2]) / (len(results[:-2]))))
        output_file.write('\t')
        output_file.write(str(number_of_welds))
        output_file.write('\t')
        output_file.write(str(number_of_threads))
        output_file.write('\n')


if __name__ == "__main__":
    args = parse_args()
    sizes = [100, 1500, 9000, 65000]
    welds = list(range(1,10))
    threads = list(range(1,10))
    with open(OUTPUT_FILE, 'w') as file:
        for size in sizes:
            for weld in welds:
                for thread in threads:
                    single_test(file, size, weld, thread)