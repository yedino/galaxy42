# RPC protocol
Galaxy42 can be configured via TCP socket connected on port set by `--rpc-port` argument. Example RPC clients are in src-gui and qa/rpc_tester dirs.

### RPC protocol
```
2 octets of message size
N octets of RPC message
64 octets of HMAC-SHA-512 authenticator
```
First 2 octets is a number of bytes of RPC message on big-endian.
RPC message is a text in JSON format. In "cmd" field should be contained command name. Next fields depends on command field.
Authenticator is a 64 octets of HMAC-SHA-512. Data is in raw format generated i.e. by sodium function crypto_auth_hmacsha512. Temporarily secret key is always 32 bytes of value 0x42.

### Available commands

* Ping

  ```
  Request: {"cmd":"ping","msg":"ping"}
  Response: {"cmd":"ping","msg":"pong"}
  ```
* Peer list
  ```
  Request: {"cmd":"peer_list","msg":"[]"}
  Response: {"cmd":"peer_list","msg":"[/**list of connected peers**/]"}
  ```
* Add peer
  ```
  Request: {"cmd":"add_peer","peer":"<ipv4>:<galaxy_port>-<ipv6>"}
  Response: {"cmd":"add_peer","msg":"peer added"}
  or
  Response: {"cmd":"add_peer","msg":"bad peer format"}
  ```
* Delete peer
  ```
  Request: {"cmd":"delete_peer","peer":"<ipv6>"}
  Response: {"cmd":"delete_peer","msg":"peer deleted"}
  or
  Response: {"cmd":"delete_peer","msg":"bad peer format"}
  ```
* Delete all peers
  ```
  Request: {"cmd":"delete_all_peer"}
  Response: {"cmd":"delete_all_peer","msg":"All peers deleted"}
  ```
* Ban peer
  ```
  Request: {"cmd":"ban_peer","peer":"<ipv6>"}
  Response: {"cmd":"ban_peer","msg":"peer banned"}
  or
  Response: {"cmd":"ban_peer","msg":"bad peer format"}
  ```
* Ban all peers
  ```
  Request: {"cmd":"ban_all_peer"}
  Response: {"cmd":"ban_all_peer","msg":"All peers banned"}
  ```
