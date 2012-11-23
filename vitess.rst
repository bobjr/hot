======================
youtube's vitess/vtocc
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


Features
Design
Implementation

Usage
=====

=> SqlQuery.GetSessionId(dbname)
<= sessionId

=> SqlQuery.Begin(sessionId)
<= transactionId

=> SqlQuery.Commit(sessionId, transactionId)
<= err

=> SqlQuery.Rollback(sessionId, transactionId)
<= err

=> SqlQuery.Execute(sql, bindVars, sessionId, transactionId)
<= result


Test
====

::

    export VTROOT=/Users/gaopeng/src/code.google.com/p/vitess

    # my.cnf server-id=tablet uid
    # 1. create dirs and my.cnf on local fs(extra info of vt in my.cnf)
    # 2. deploy bootstrap mysql db to datadir
    # 3. startup mysqld  $VTROOT/dist/vt-mysql/bin/mysqld_safe
    # 4. import _vt_schema.sql
    mysqlctl -tablet-uid 62344 -port 6700 -mysql-port 3700 init


    # <server_id>@<hostname>:<leader_port>:<election_port>:<client_port>
    # create zoo.cfg and startup zk server
    zkctl -zk.cfg 1@`hostname`:3801:3802:3803 init

    # just create znode in zk 
    vtctl -force CreateKeyspace /zk/global/vt/keyspaces/test_keyspace

    # create znode in zk
    #                path                              hostname  mysqlPort vtPort keyspace      shardId tableType parent
    vtctl InitTablet /zk/test_nj/vt/tablets/0000062344 localhost 3700      6700   test_keyspace 0       master    ""
    vtctl InitTablet /zk/test_nj/vt/tablets/0000062044 localhost 3701 6701 test_keyspace 0 replica /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-62344
    vtctl InitTablet /zk/test_nj/vt/tablets/0000041983 localhost 3702 6702 test_keyspace 0 replica /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-62344

    # their rpc server
    vttablet -port 6700 -tablet-path /zk/test_nj/vt/tablets/0000062344
    vttablet -port 6701 -tablet-path /zk/test_nj/vt/tablets/0000062044

    # 
    vtctl RebuildShard /zk/global/vt/keyspaces/test_keyspace/shards/0

    # 

    vtctl Ping /zk/test_nj/vt/tablets/0000062344





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


概念
---------

vt = tablet
keyspace = DatabaseName
uid = tablet uid

某个keyspace下的tabletserver的uid都不同

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

    On file system:

        vt
         |
         |- zk_global_<uid>
         |
         |- zk_<uid>
         |    |
         |    |- logs
         |    |- zoo.cfg
         |    |- zk.pid
         |    |- myid
         |
         |- vt_<uid>
              |
              |- data/
              |
              |- bin-logs
              |     |
              |     |- vt-<uid>-bin.index
              |
              |- relay-logs
              |     |
              |     |- relay.info
              |     |- vt-<uid>-relay-bin.index
              |
              |- slow-query.log
              |- error.log
              |- master.info
              |
              |- mysql.pid
              |- my.cnf
              |- mysql.sock
              |- innodb
                    |
                    |- data
                    |- log



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
.. image:: http://zookeeper.apache.org/doc/r3.1.2/images/zkperfRW.jpg

::


        reserved_pool

        conn_pool

        active_tx_pool

        active_pool

zk
--

uid = mysql server id


'global' is a special cell


keyspace: /zk/global/vt/keyspaces/test_keyspace

zk

- queue for action

- directory lookup

- lock



`*` is EPHEMERAL

::




                                zk              
                                 |
                                 |
                                  ------------
                                              |
                                              | watch action
                                              |
                             vttablet ----- agent ------ vtaction ---- actor
                                |     start       invoke          call
                                |
                                |


    /zk
     |
     |- <cell>
     |     |
     |     |- vt
     |        |
     |        |- tablets
     |              |
     |              |---- <uid> => json(Tablet)
     |                      |
     |                      |- action
     |                      |    |
     |                      |    |- SEQUENCE => json(ActionNode)
     |                      |
     |                      |- pid* => hostname:pid
     |
     |- local
     |     |
     |     |- vt
     |        |
     |        |--- ns
     |              | 
     |              |- <keyspace>
     |                      |
     |                      |- <shard id>
     |                           |
     |                           |- <db type> => json(VtnsAddrs)
     |            
     |            
     |- global
           |
           |- vt
              |
              |- keyspaces
                    |
                    |- <keyspace>
                            |
                            |- action
                            |    |
                            |    |- SEQUENCE => json(ActionNode)
                            |
                            |
                            |- shards
                                 |
                                 |- <shard id> => json(Shard)
                                        |
                                        |- action
                                             |
                                             |- SEQUENCE => json(ActionNode)



xx
--

cmd/vttablet/vttablet -port 6700 -tablet-path /zk/test_nj/vt/tablets/0000062344 -logfile /vt/vt_0000062344/vttablet.log

=============== =========== ==============================
cmd             rpc server  pkg
=============== =========== ==============================
vtocc           Y           tabletserver.queryctl.go
vtaction        Y           tabletmanager.actor.go
vttablet        Y           SqlQuery and TabletManager rpc server
vtctl           N           tabletmanager.initiator.go  wrangler.
zkctl           N           启动、关闭zk server
=============== =========== ==============================

Tablet
------

action

global uniq
cell in zk, json'ed data in zk

TabletType

- idle

- master

- [slave]

  - replica

  - rdonly

  - spare
    same as replica except that it does not serve query

  - backup



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
