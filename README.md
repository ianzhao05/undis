# Undis

A persistent key-value store supporting aspects of the [memcached protocol](https://github.com/memcached/memcached/blob/master/doc/protocol.txt).

Supports:

- safe and performant concurrent read/write
- client access through a cross-platform TCP server
- unlimited concurrent connections with a dynamic threadpool
- optional persistence to disk via serialization

There are no dependencies other than the standard C++ library and system headers. Unit tests depend on GoogleTest.
