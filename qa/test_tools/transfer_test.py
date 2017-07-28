#!/usr/bin/env python3
import sys, subprocess, os, time
from subprocess import PIPE, Popen
from threading  import Thread

from queue import Queue, Empty  # python 3.x

program='../../build/tunserver.elf'
program_options={}
program_options['--port']='5042'
program_options['--rpc-port']='45000'

ON_POSIX = 'posix' in sys.builtin_module_names

def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
    out.close()

def run_galaxy():
    if '-s' in sys.argv:
        args = list(sum(program_options.items(), ()))
        args.insert(0, program)
        galaxy = Popen(args , stderr=PIPE, stdout=PIPE, bufsize=1, close_fds=ON_POSIX)
        q = Queue()
        t = Thread(target=enqueue_output, args=(galaxy.stdout, q))
        t.daemon = True # thread dies with the program
        t.start()

        # read line without blocking
        while True:
            #try:  line = q.get_nowait()
            try: line = q.get(timeout=0.1)
            except Empty:
                pass
                #print('no output yet')
            else: # got line
                if 'Your hash-IPv6 address is :' in str(line):
                    galaxy_ip = str(line).split(' : ')[1][:-3]
                    print('Should run on the other computer this program, with following options: ')
                    print(os.path.basename(__file__), '-c', 'YOURIP-'+galaxy_ip, '[ options ]')
                    break
    elif '-c' in sys.argv:
        peer = sys.argv[sys.argv.index('-c')+1]
        print('-'.join([':', program_options['--port']]))
        peer.replace('-', ':'.join([program_options['--port'], '-']))
        program_options['--peer'] = peer
        print(program_options['--peer'])
        args = list(sum(program_options.items(), ()))
        args.insert(0, program)
        galaxy = Popen(args , stderr=PIPE, stdout=PIPE, bufsize=1, close_fds=ON_POSIX)
        q = Queue()
        t = Thread(target=enqueue_output, args=(galaxy.stdout, q))
        t.daemon = True # thread dies with the program
        t.start()

        # read line without blocking
        while True:
            #try:  line = q.get_nowait()
            try: line = q.get(timeout=0.1)
            except Empty:
                pass
                #print('no output yet')
            else: # got line
                print(line)
                if 'OK: Started virtual network card interface (TUN) with proper IPv6 and other settings' in str(line):
                    break
    else:
        print("Use with option -s (to run as server) or -c [YOURIP-GALAXYIP] (to run as client)")

def run_iperf():
    sys.argv[0] = 'iperf'
    sys.argv.insert(1, '-V')
    if '-c' in sys.argv:
        index = sys.argv.index('-c')+1
        sys.argv[index] = sys.argv[:sys.argv[index].index('-')]
        print(sys.argv[index])
    subprocess.call(sys.argv)

if __name__ == "__main__":
    run_galaxy()
    run_iperf()
