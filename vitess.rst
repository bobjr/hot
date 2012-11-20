======================
youtube's vitess/vtocc
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


logical vs physical database

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

- Amoeba

- hbase

  ::

            rowKey
              |
            ZooKeeper
              |
              | -ROOT- rs
              |
            RegionServer
              |
              | .META. rs
              |
            RS of this rowKey


Vitess
======

Metrics
-------

- 25572 line of code

- 135 files

Installation
------------

::

        export GOPATH=$HOME
        mkdir -p $HOME/src/code.google.com/p/vitess
        hg clone -u weekly https://code.google.com/p/vitess/ $HOME/src/code.google.com
        cd $HOME/src/code.google.com/p/vitess
        export MYSQL_CONFIG=/usr/local/mysql/bin/mysql_config
        export LD_LIBRARY_PATH
        ./bootstrap.sh
        source dev.env

        cd go
        make


Intro
-----

Open source 2012-2

Features
--------

- self management

- external replication

- range based sharding

  auto_increment will not work, split key should be distributed randomly

- auto split a shard into 2 when it is hot

  auto merge shards into 1

- online alter schema

  deploy DDL to offline replicas and reparenting because it can elect a new master

- caching

- zero downtime restarts

- embedded sql parser
  
  auto anti-sql-inject/bind vars for query to reuse query plans

- tansaction

- fail-safe


Performance
-----------

- 10k qps

  GC tuned

Assumption
----------

mysql
^^^^^

- good at storage

- bad at scaling

  not able to coordinate many instances of a single logical schema 

- not good at random access table query cache

::

                    client
                      |
                      | RPC with bson/gob/json codec over tranport tcp/http
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


connection pool
---------------

.. image:: http://wiki.vitess.googlecode.com/hg/vtpools.png

::


        reserved_pool

        conn_pool

        active_tx_pool

        active_pool

zk
--

::

    /vt
     |
     |- tablets
     |     |
     |     |- <uid>
     |          |
     |          |- action
     |
     |- keyspaces
           |
           |- <keyspace>
                 |
                 |- shards
                      |
                      |- <shard id>
      

Tablet
------
in zk

- vtocc

  Query server

  RPC front-end to mysql


- vttablet

  local

  Serves queries and performs housekeeping jobs

  -tablet-path /vt/tablets/<uid>

  pathParts := strings.Split(zkTabletPath, "/")
  pathParts[len(pathParts)-2] === "tablets"




- vtctl

  global 
