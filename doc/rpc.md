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
  Request: {"cmd":"ping","msg":"ping","state":"ok"}
  Response: {"cmd":"ping","msg":"pong","state":"ok"}
  ```
* Peer list
  ```
  Request: {"cmd":"peer_list","msg":"[]","state":"ok"}
  Response: {"cmd":"peer_list","peers":"[/**list of connected peers**/]", "msg":"ok:","state":"ok"}
  ```
* Add peer
  ```
  //for old format
  Request: {"cmd":"add_peer","format":"0.1","peer":"<ipv4>:<galaxy_port>-<ipv6>","state":"ok"}
  or
  //for new format
  Request: {"cmd":"add_peer","format":"1.0","peer":"<ipv6>@(udp:<ipv4>:<galaxy_port>)","state":"ok"}
  Response: {"cmd":"add_peer","msg":"ok: Peer added","state":"ok"}
  or
  Response: {"cmd":"add_peer","msg":"fail: Bad peer format","state":"error"}
  ```
* Delete peer
  ```
  Request: {"cmd":"delete_peer","peer":"<ipv6>","state":"ok"}
  Response: {"cmd":"delete_peer","msg":"ok: Peer deleted","state":"ok"}
  or
  Response: {"cmd":"delete_peer","msg":"fail: Bad peer format","state":"error"}
  ```
* Delete all peers
  ```
  Request: {"cmd":"delete_all_peers","state":"ok"}
  Response: {"cmd":"delete_all_peers","msg":"ok: All peers deleted","state":"ok"}
  ```
* Ban peer
  ```
  Request: {"cmd":"ban_peer","peer":"<ipv6>","state":"ok"}
  Response: {"cmd":"ban_peer","msg":"ok: Peer banned","state":"ok"}
  or
  Response: {"cmd":"ban_peer","msg":"fail: Bad peer format","state":"error"}
  ```
* Ban all peers
  ```
  Request: {"cmd":"ban_all_peer","state":"ok"}
  Response: {"cmd":"ban_all_peer","msg":"ok: All peers banned","state":"ok"}
  ```
* Get galaxy ipv6
  ```
  Request: {"cmd":"get_galaxy_ipv6","state":"ok"}
  Response: {"cmd":"get_galaxy_ipv6","ipv6":"<ipv6>", "msg":"ok:","state":"ok"}
  ```
* Get galaxy new format reference
  ```
  Request: {"cmd":"get_galaxy_invitation", "msg":[/*list of ipv4 addresses */],"state":"ok"}
  Response: {"cmd":"get_galaxy_invitaion","inv":"<galaxy-new-format-invitation>", "msg":"ok:","state":"ok"}
  ```
