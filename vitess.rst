======================
youtube's vitess/vtocc
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

DBMan
=======

Call Path
---------

coordinator?

::

                                     php
                                      |
                           LCache - kproxy ---------------------------------
                                      |                                     |
                              ------------------------                      |
                             |        |               |                     |   
                          visitman  friendman     userman                   |
                             |        |               |                     |
                              ------------------------                      |
                                          |                                 |
                                          | persistant connection #?        |
                                          V                                 |
                            ------------------------------                  |
                           |                dbman cluster |<----------------
                           |                              |
                           |    dbman   dbman    dbman    | stateless
                           |                              |
                           |                 mysql client |
                            ------------------------------
                                        |
                                pthread | each dbman has 6 persistant conn with each mysql instance
                    libmysqlclient_r.so | totals 6 * 700 = 4300 tcp conn on each dbman
                                        |
                               -----------------------------
                              |       |       |             |
                            mysql1  mysql2  mysql...     mysql700
                              |
                         conn pool size = 6 * #dbman = 1000 ?


Roles
-----

- shard routing

- execute sql query

Routining
---------

::

    (kind, split_key)
        |
        | lookup(kind)
        V
    kind_setting
        |
        | lookup(kind, no=split_key % table_num)
        V
    table_setting
        |
        | lookup(sid)
        V
    server_setting
        |
    (host, port, user, pass)


Problem
-------

- hard to rebalance

  - can only scale up to 2**N shards

  - need 50% relocate data when N=1

  - has stop-the-world

- key not sorted


Related Projects
================

- gizzard by twitter

- vitess by youtube

- 变形虫

Vitess
======

Intro
-----

Open source 2012-2

Features
--------

- self management

- external replication

- range based shards

- zero downtime restarts

- embedded sql parser
  
  auto anti-sql-inject/bind vars for query to reuse query plans

- tansaction

- 

Assumption
----------

mysql
^^^^^

- good at storage

- bad at scaling

  not able to coordinate many instances of a single logical schema 

::

                    client
                      |
                      | RPC/bson
                      |
            ---------------------------- 
         v |  Connection handler        |
         t |----------------------------|
         o |  QueryCache | SqlParser    |
         c |----------------------------|
         c |  Optimizer                 |
            ---------------------------- 
                      |
                      |------------------------------------------
                      |                         |         |      |
            ----------------------------      -----     -----   -----
           |  Connection handler        |     mysql     mysql   mysql
         m |----------------------------|
         y |  QueryCache | SqlParser    |
         s |----------------------------|
         q |  Optimizer                 |
         l |----------------------------|
           |  StorageEngines            |
            ---------------------------- 


