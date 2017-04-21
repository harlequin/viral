# viral
dcc client with a small foot print

## Installation

### Build
...

### IRC Server configuration
For adding a new server connection, insert following construct into `viral.conf`

```text
server0.host = irc.abjects.net
server0.port = 9999
server0.encryption = yes
server0.channels = #moviegods, #mg-chat
```

### SSL web interface
For ssl feature in the web interface it is needed to compile viral with `NS_ENABLE_SSL`

Genereate viral.pem via openssl

```bash
#!/bin/bash
# Generate server key
openssl genrsa -des3 -out server.key 1024

# Extract un-safe version
openssl rsa -in server.key -out server.key.unsecure  

# Certificate Signing Request (CSR) with personal attributes
openssl req -new -key server.key -out server.csr  

# One year self-signed certificate
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

# CSR is not needed anymore
rm server.csr

# Key + certificate (in this order) into one file
cat server.key.unsecure server.crt > viral.pem
```

Copy `viral.pem` to installation directory

## Debugging

# Generate core dump
```
$ ulimit -c
0
```
Is saying that no core dumps will be generated

```
ulimit -c unlimited
```
is generating than dumps in any size


```
arm-none-linux-gnueabi-gdb viral core
set solib-search-path .
bt
```