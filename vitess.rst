======================
youtube's vitess
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: vitess, the scalable mysql cluster framework explained.
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


Features

Implementation


what is missed?

query router



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
                          visitman  friendman  ...  userman     conf.kproxy |
                             |        |               |                     |
                              ------------------------                      |
                                          |                                 |
                                          V                                 |
                            ------------------------------                  |
                           |           dbman cluster(160) |<----------------
             shard routing |                              |
           rd load balance |    -----   -----    -----    |
                  exec sql |   |dbman| |dbman|  |dbman|   | stateless
                           |    -----   -----    -----    |
                           |                              |
                           |                 mysql client |
                            ------------------------------
                                        |
                                pthread | each dbman has 6 persistant conn with each mysql instance
                    libmysqlclient_r.so | totals 6 * 700 = 4300 tcp conn per dbman
                                        V
                               -----------------------------
                              |       |       |             |
                            mysql1  mysql2  mysql...     mysql700
                              |
                         conn pool size = 6 * #dbman = 1000 ?

Routing
-------

(kind, split_key) => (host, port, user, pass, kind_no)

::

    (kind, split_key)
        |
        | kind_setting.lookup(kind)
        V
    (table_num, no=split_key % table_num)
        |
        | table_setting.lookup(kind, no)
        V
       sid
        |
        | server_setting.lookup(sid, masterOrSlave)
        V
    [(host, port, user, pass), ...]


Related Projects
================

- gizzard by twitter

- spider storage engine

  http://spiderformysql.com/

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

Usage
-----

=> SqlQuery.GetSessionId(dbname)
<= sessionId (randInt64)

=> SqlQuery.Begin(sessionId)
<= transactionId (atomicInt64)

=> SqlQuery.Commit(sessionId, transactionId)
<= err

=> SqlQuery.Rollback(sessionId, transactionId)
<= err

=> SqlQuery.Execute(sql, bindVars, sessionId, transactionId)
<= result

Design
------

- logical vs physical database


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
           |  Connection pool           |
         v |----------------------------|
         t |  QueryCache | SqlParser    |
           |----------------------------|
           |  Optimizer  Replication    |
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



           -------    put       ----
          | vtctl | ---------> | zk |            
           -------    produce   ----
                                 |
                                  --------------
                                                |
                                        consume | watch action
                                                |
                          ----------------------|-----------------------------------
         --------        |                      |                                   |
        | smart  | query |  ----------          V                                   |
        | client |-------->| vttablet | o----- agent ------ vtaction ---- actor     |
         --------        |  ----------   start       invoke          call   |       |
                         |                                                 | ctl    |
                         |                                                 |        |
                         |                                              --------    |
                         |                                             | mysqld |   |
                         |                                              --------    |
                         |                                                          |
                         |                                       per mysql instance |
                          ----------------------------------------------------------
                          


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
cmd             rpc server  desc
=============== =========== ==============================
vtctl           N           global mgmt tool  tabletmanager.initiator.go  wrangler.
vttablet        Y           SqlQuery/TabletManager/UmgmtService rpc server, action agent watcher

mysqlctl        N           init/start/shutdown/teardown a mysql instance
zkctl           N           init/start/shutdown/teardown a zookeeper
vtaction        N           execute actions
=============== =========== ==============================


=========================== =====
action                      value
=========================== =====
TABLET_ACTION_PING          Ping
TABLET_ACTION_SLEEP         Sleep
TABLET_ACTION_SET_RDONLY    SetReadOnly
TABLET_ACTION_SET_RDWR      SetReadWrite
TABLET_ACTION_CHANGE_TYPE   ChangeType
TABLET_ACTION_DEMOTE_MASTER DemoteMaster
TABLET_ACTION_PROMOTE_SLAVE PromoteSlave
TABLET_ACTION_RESTART_SLAVE RestartSlave
TABLET_ACTION_BREAK_SLAVES  BreakSlaves
TABLET_ACTION_SCRAP         Scrap

SHARD_ACTION_REPARENT       ReparentShard
SHARD_ACTION_REBUILD        RebuildShard

KEYSPACE_ACTION_REBUILD     RebuildKeyspace
=========================== =====


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
