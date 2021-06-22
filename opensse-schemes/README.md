# GUIDE

## Requirement

### Linux 

```bash
[sudo] apt-get install build-essential autoconf libtool yasm openssl cmake libaio-dev
```
#### Installing grpc

See the [installation guide](https://github.com/grpc/grpc/blob/master/BUILDING.md)

Note : use make -j4 instead of make -j

#### Installing rocksdb

See the [installation guide](https://github.com/facebook/rocksdb/blob/master/INSTALL.md)

#### Installing crypto-tk

See the [installation guide](https://github.com/OpenSSE/crypto-tk#building)

Run the script in folder /install_dependencies 

## Building
 Building is done using CMake. The minimum required version is CMake 3.1.

 Then run the file run.sh
```bash
chmod +x run.sh
./run.sh
```

## Contributing

h3des, Nev, An