#!/usr/bin/env python3
import sys
import subprocess
import os
import time
import argparse
import multiprocessing
from subprocess import check_output, DEVNULL, Popen
import itertools

CLIENT = './asio_send_block/asio_send_block'
SERVER = './asio_receive_block/asio_receive_block'
HOST_ADDRESS = '127.0.0.1'
TASKSET = 'taskset'
CPU_COUNT = multiprocessing.cpu_count()


def run_client(port, packet_size, taskset_c):
    """
    Run client.
    :param port: port to send udp packets 
    :param packet_size: packet size in bytes
    :param taskset_c: number of system thread on which you want to run client process
    :return: client process
    """
    client_args = (TASKSET, '-c', str(taskset_c), CLIENT, HOST_ADDRESS, str(port), '100000000', 'asdf',
                   str(packet_size), '-1')
    client_process = Popen(client_args, stderr=DEVNULL, stdout=DEVNULL)
    print('Running client with command: ', ' '.join(client_args))
    return client_process


def run_server(port, taskset_c):
    """
    Run server
    :param port: port on which server will listen 
    :param taskset_c: number of system thread on which you want to run server process
    :return: result speed (Mbits/s) from server
    """
    server_args = (TASKSET, '-c', str(taskset_c), SERVER, str(port))
    output = check_output(server_args)
    print('Running server with command: ', ' '.join(server_args))
    print('output:', output.decode())
    ret = output.decode().split('\n')[-2].split()
    return ret


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description='Script to test client/server program speed depends on which '
                                                     'threads programs will run\n Before run script you have to build '
                                                     'program first')
    arg_parser.add_argument('-s', '--size', type=int, default=1500, help='packet size in bytes;')
    arg_parser.add_argument('-p', '--port', type=int, default=9000, help='port number')
    arg_parser.add_argument('-i', '--idle', type=int, default=0, help='number of idle system threads')
    args = arg_parser.parse_args()

    client_processes = []
    results = []
    idle_threads_combinations = tuple(itertools.combinations(tuple(range(CPU_COUNT)), args.idle))
    for idle_threads in idle_threads_combinations:
        available_threads = tuple([x for x in range(CPU_COUNT) if x not in idle_threads])
        for i in available_threads:
            for j in available_threads:
                if j != i:
                    client_processes.append(run_client(args.port, args.size, j))
            time.sleep(1)
            results.append(run_server(args.port, i) + [i, idle_threads])
            for process in client_processes:
                process.kill()

        results.sort(reverse=True)
        print(results)

    with open("results.txt", "a") as result_file:
        result_file.write('packet size: ' + str(args.size) + ' B\n')
        for result in results:
            result_file.write('threads number: ' + str(CPU_COUNT))
            result_file.write('\n')
            result_file.write('server thread: ' + str(result[2]))
            result_file.write('\n')
            result_file.write('idle threads: ' + str(result[3]))
            result_file.write('\n')
            result_file.write('avg speed: ' + str(result[0]) + ' Mbits/s\n')
            result_file.write('max speed: ' + str(result[1]) + ' Mbits/s\n')
            result_file.write('percent of max avg speed: ' + str((int(result[0]) / int(results[0][0])) * 100) + '%')
            result_file.write('\n\n')

        result_file.write('\n\n')
