==============
RPC in golang
==============

:Author: Gao Peng <funky.gao@gmail.com>

.. contents:: Table Of Contents
.. section-numbering::

Data Flow
=========

::


            RPC Server                          RPC Client
              |                                     |
              | handle('/_goRPC_')                  |
              | handle('/debug/rpc')                |
              |                                     |
              |                                     |
              |       CONNECT /_goRPC_ HTTP/1.0     |
              |<------------------------------------|
              |                                     |
              | Hijack(http conn)                   |
              |                                     |
              | HTTP/1.0 200 Connected to Go RPC    |
              |------------------------------------>|
              |                                     |
              |             codec.Marshal(request)  |
              |<------------------------------------|
              |                                     |
              |  codec.Unmashal(response)           |
              |------------------------------------>|
              |                                     |

