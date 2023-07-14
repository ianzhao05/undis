# Undis

A persistent key-value store supporting aspects of the [Memcached protocol](https://github.com/memcached/memcached/blob/master/doc/protocol.txt).

Features:

- safe and performant concurrent reading/writing
- a cross-platform TCP server to serve clients
- unlimited concurrent connections with a thread pool that grows and shrinks dynamically according to demand
- ability to specify an expiration time and flags to accompany a string value, like Memcached
- optional persistence to disk via a compact serialization algorithm

This project also makes use of modern metaprogramming techniques to avoid unnecessary copies and allocations while using concepts to maintain semantics.

## Building and Testing

This project builds with CMake and requires the C++20 standard. There are no dependencies other than the standard C++ library and system headers. Unit tests depend on GoogleTest.

```shell
$ git clone https://github.com/ianzhao05/undis.git && cd undis
$ cmake -B ./build
$ cmake --build ./build
$ ctest --test-dir ./build
```

Once built and ran, clients can connect on port `8080` by default, for instance with `telnet`. Data will be read from and written to `undis.db` at startup and shutdown, respectively. The port can be changed with the `-p` flag, and the persistence file can be changed with the `-f` flag.

## Sample Usage

From the [Memcached protocol](https://github.com/memcached/memcached/blob/master/doc/protocol.txt), the `get`, `delete`, `set`, `add`, `replace`, `prepend`, `append`, and `quit` commands are supported.

```
$ telnet localhost 8080
undis > set key 42 0 5
value
STORED
undis > get key
VALUE key 42 5
value
END
undis > add key2 64 0 6
value2
STORED
undis > replace key2 65 0 9
value two
STORED
undis > append key2 0 0 7
_suffix
STORED
undis > get key key2
VALUE key 42 5
value
VALUE key2 65 16
value two_suffix
END
undis > delete key
DELETED
undis > delete key
NOT_FOUND
undis > quit
Connection closed by foreign host.
```
