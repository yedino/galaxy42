#!/usr/bin/env python3
import sys
import subprocess
import os
import time
from subprocess import PIPE, Popen
from threading import Thread
from queue import Queue, Empty

program = '../../build/tunserver.elf'
program_options = dict()
program_options['--port'] = '5042'
program_options['--rpc-port'] = '45000'

ON_POSIX = 'posix' in sys.builtin_module_names

galaxy_env = os.environ.copy()
galaxy_env["LANG"] = "en_US.UTF-8"


def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
        # if 'ERROR' in str(line):
        #   print(line.decode('utf-8', end=''))
    out.close()


def run_galaxy():
    """
    Run tunserver.elf.
    """
    if '-c' not in sys.argv and '-s' not in sys.argv:
        print("Use with option -s (to run as server) or -c [YOURIP-GALAXYIP] (to run as client)")
        sys.exit()
    if '-c' in sys.argv:
        peer = sys.argv[sys.argv.index('-c') + 1].split('-')
        galaxy_peer = '{}:{}-{}'.format(peer[0], program_options['--port'], peer[1])
        program_options['--peer'] = galaxy_peer

    args = list(sum(program_options.items(), ()))
    args.insert(0, program)
    galaxy = Popen(args, env=galaxy_env, stderr=PIPE, stdout=PIPE, bufsize=1, close_fds=ON_POSIX)
    print('Running galaxy with command: ', ''.join([arg + ' ' for arg in args]))
    q = Queue()
    t = Thread(target=enqueue_output, args=(galaxy.stdout, q))
    t.daemon = True  # thread dies with the program
    t.start()

    # read line without blocking
    while True:
        try:
            # line = q.get_nowait()
            line = q.get(timeout=0.1)
        except Empty:
            pass
            # print('no output yet')
        else:  # got line
            if 'Your hash-IPv6 address is :' in str(line) and '-s' in sys.argv:
                galaxy_ip = str(line).split(' : ')[1][:-3]
                print('You should run on the other computer this program, with following options: ')
                print(
                    './' + os.path.basename(__file__), '-c', 'YOURIP-' + galaxy_ip, '[ iperf options (see man iperf)]')
                break
            elif 'OK: Started virtual network card interface (TUN) with proper IPv6 and other settings' in str(line):
                break


def run_iperf():
    """
    Run iperf with arguments from command line.
    """
    sys.argv[0] = 'iperf'
    sys.argv.insert(1, '-V')
    if '-c' in sys.argv:
        index = sys.argv.index('-c') + 1
        sys.argv[index] = sys.argv[index][sys.argv[index].index('-') + 1:]
    print('Running iperf with command: ', ''.join([arg + ' ' for arg in sys.argv]))
    subprocess.call(sys.argv)


if __name__ == "__main__":
    run_galaxy()
    run_iperf()
