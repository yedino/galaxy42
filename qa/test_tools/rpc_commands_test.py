#!/usr/bin/env python3
import sys
import subprocess
import os
import time
import socket
import hashlib
import hmac
import json
from subprocess import check_output, DEVNULL, Popen

rpc_server = '127.0.0.1'
rpc_port = 46000

program = '../../build/tunserver.elf'
program_options = dict()
program_options['--rpc-port'] = str(rpc_port)
program_options['--port'] = '29042'
rpc_tester = '../rpc_tester/rpc_tester'
peer = '192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5'

galaxy_env = os.environ.copy()
galaxy_env["LANG"] = "en_US.UTF-8"

rpc_commands = dict()
rpc_commands['add_peer'] = '{"cmd":"add_peer", "peer":"' + peer + '"}'
rpc_commands['delete_peer'] = '{"cmd":"delete_peer", "peer":"' + peer.split('-')[1] + '"}'
rpc_commands['ban_peer'] = '{"cmd":"ban_peer", "peer":"' + peer.split('-')[1] + '"}'
rpc_commands['delete_all_peers'] = '{"cmd":"delete_all_peers"}'
rpc_commands['ban_all_peers'] = '{"cmd":"ban_all_peers"}'
rpc_commands['peer_list'] = '{"cmd":"peer_list","msg":"[]"}'

def _ok(output):
    return '\033[92m{}\033[0m'.format(output)

def _fail(output):
    return '\033[91m{}\033[0m'.format(output)

def ping(ipv6):
    """
    Returns True if ipv6 (string) responds to a ping request.
    """
    return os.system("ping6 -c 3 " + ipv6) == 0

def run_galaxy():
    """
    Run tunserver.elf.
    """
    args = list(sum(program_options.items(), ()))
    args.insert(0, program)
    Popen(args, env=galaxy_env, stderr=DEVNULL, stdout=DEVNULL)
    print('Running galaxy with command: ', ''.join([arg + ' ' for arg in args]))

def run_rpc_command(rpc_command):
    """
    Send rpc_command (string) to galaxy rpc server using rpc_tester.
    """
    args = ['--port', program_options['--rpc-port'], '--message', rpc_commands[rpc_command]]
    args.insert(0, rpc_tester)
    out = check_output(args)
    print('RPC command', rpc_command, 'sent')
    for line in out.decode('utf-8').split('\n'):
        if 'RPC response' in line:
            print(line)

def send_rpc_command(rpc_command):
    """
    Send rpc_command (string) to galaxy rpc server using python libs.
    """
    print('Sending RPC command:', rpc_command)
    buff_size = 256*256
    msg = rpc_commands[rpc_command]
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((rpc_server, rpc_port))
    secret = bytes([0x42]*32)
    sing = hmac.new(secret, msg.encode(), hashlib.sha512).digest()
    s.send(bytes([len(msg)//256, len(msg) % 256]))
    s.send(msg.encode())
    s.send(sing)
    r = s.recv(buff_size)
    data_length = int.from_bytes(r[:2], byteorder='big')
    s.close()
    r_sign = hmac.new(secret, r[2:data_length+2], hashlib.sha512).digest()
    if not hmac.compare_digest(r_sign, r[data_length+2:]):
        print(_fail('Response authentication fail'))
        sys.exit()
    return r[2:data_length+2].decode('utf-8')

if __name__ == "__main__":
    run_galaxy()
    if ping(peer.split('-')[1]):
        print(_ok('Ping working. It is OK'))
    else:
        print(_ok('Ping not working. It is OK'))
    print('Response:', send_rpc_command('ban_all_peers'))
    if ping(peer.split('-')[1]):
        print(_fail('Ping working. ban_all_peers command not working. It is BAD!!!'))
        sys.exit()
    else:
        print(_ok('Ping not working. ban_all_peers command working. It is OK'))
    print('Response:', send_rpc_command('add_peer'))
    response = send_rpc_command('peer_list')
    print('Response:', response)
    peer_list = json.JSONDecoder().decode(response)['msg']
    print('Peer list:', peer_list)
    if len(peer_list) is 1:
        print(_ok('peer_list command working. It is OK'))
    else:
        print(_fail('peer_list command not working. It is BAD!!!'))
    if ping(peer.split('-')[1]):
        print(_ok('Ping working. add_peer command working. It is OK'))
    else:
        print(_fail('Ping not working. add_peer command not working. It is BAD!!!'))
        sys.exit()
    print('Response:', send_rpc_command('ban_peer'))
    if ping(peer.split('-')[1]):
        print(_fail('Ping working. ban_peer command not working. It is BAD!!!'))
        sys.exit()
    else:
        print(_ok('Ping not working. ban_peer command working. It is OK'))
    print('Response:', send_rpc_command('add_peer'))
    if ping(peer.split('-')[1]):
        print(_ok('Ping working. add_peer command working. It is OK'))
    else:
        print(_fail('Ping not working. add_peer command not working. It is BAD!!!'))
        sys.exit()
