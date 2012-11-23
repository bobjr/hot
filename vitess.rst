======================
youtube's vitess
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: vitess, the scalable mysql cluster framework explained.
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

DBMan
=======

Topology
--------

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
                                pthread | 6 persistant conn to each mysql instance per dbman
                    libmysqlclient_r.so | totals 6 * 700 = 4300
                                        |
                                        V internal replication
                               ---------------------------------
                              |       |       |      |          |
                            mysql1  mysql2  mysql3  ...     mysql700
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

  https://github.com/twitter/gizzard

- spider storage engine

  http://spiderformysql.com/

- vitess by youtube

  http://code.google.com/p/vitess/

- Amoeba

  http://sourceforge.net/projects/amoeba/


Vitess
======

Intro
-----

- Open source 2012-2

  still very active

- 25572 line of code

- 135 files

- golang plus python client


Usage
-----

RpcClient
^^^^^^^^^

go and python currently

::

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


Cluster
^^^^^^^

::

    vtctl CreateKeyspace /zk/global/vt/keyspaces/test_keyspace

    vtctl InitTablet /zk/test_nj/vt/tablets/0000062344 localhost 3700 6700 test_keyspace 0 master ""
    vtctl InitTablet /zk/test_nj/vt/tablets/0000062044 localhost 3701 6701 test_keyspace 0 replica /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-62344
    vtctl InitTablet /zk/test_nj/vt/tablets/0000041983 localhost 3702 6702 test_keyspace 0 replica /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-62344

    vttablet -port 6700 -tablet-path /zk/test_nj/vt/tablets/0000062344
    vttablet -port 6701 -tablet-path /zk/test_nj/vt/tablets/0000062044
    vttablet -port 6702 -tablet-path /zk/test_nj/vt/tablets/0000041983

    vtctl Ping /zk/test_nj/vt/tablets/0000062344
    vtctl RebuildShard /zk/global/vt/keyspaces/test_keyspace/shards/0

Features
--------

- logical vs physical database

- self management

  - external replication

  - range based sharding

    auto_increment will not work, split key should be distributed randomly

  - auto split a shard into 2 when it is hot

    auto merge shards into 1

  - online alter schema

    deploy DDL to offline replicas and reparenting because it can elect a new master

  - zero downtime restarts

- caching

- embedded sql parser
  
  auto anti-sql-inject/bind vars for query to reuse query plans

- tansaction

- fail-safe


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


Components
----------

=============== =========== ==============================
cmd             rpc server  desc
=============== =========== ==============================
vtctl           N           global mgmt tool  tabletmanager.initiator.go  wrangler.
vttablet        Y           SqlQuery/TabletManager/UmgmtService rpc server, action agent watcher

mysqlctl        N           init/start/shutdown/teardown a mysql instance
zkctl           N           init/start/shutdown/teardown a zookeeper
vtaction        N           execute actions
=============== =========== ==============================

vttablet startup

::

    select table_name, table_type, unix_timestamp(create_time), table_comment from information_schema.tables where table_schema = database()



Architecture
------------

TabletType
^^^^^^^^^^
 
- idle

  standby server without data

- master

- [slave]

  - replica

    a slaved copy of the data ready to be promoted to master

  - rdonly

    a slaved copy of the data for olap load patterns

  - spare

    same as replica except that it does not serve query

  - backup

    a slaved copy of the data, but offline to queries other than backup

    replication sql thread may be stopped

  - lag

    a slaved copy of the data intentionally lagged for pseudo backup

  - scrap

    a machine with data that needs to be wiped


DataFlow
^^^^^^^^

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
                         |      |                                          | ctl    |
                         |      | unix sock                                |        |
                         |      |                                       --------    |
                         |    umgmt                                    | mysqld |   |
                         |                                              --------    |
                         |                                                          |
                         |                                       per mysql instance |
                          ----------------------------------------------------------
                          
Diagrams
--------

.. image:: http://wiki.vitess.googlecode.com/hg/tabletserver.png
.. image:: http://wiki.vitess.googlecode.com/hg/vtpools.png
.. image:: http://zookeeper.apache.org/doc/r3.1.2/images/zkperfRW.jpg

zk
--

Roles
^^^^^

- queue for action

- directory lookup

- lock


Znodes
^^^^^^

`*` is EPHEMERAL

::

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



action
------


=========================== =================== =====
action                      value               exec
=========================== =================== =====
TABLET_ACTION_PING          Ping                
TABLET_ACTION_SLEEP         Sleep
TABLET_ACTION_SET_RDONLY    SetReadOnly
TABLET_ACTION_SET_RDWR      SetReadWrite
TABLET_ACTION_CHANGE_TYPE   ChangeType
TABLET_ACTION_DEMOTE_MASTER DemoteMaster        SET GLOBAL read_only=ON; FLUSH TABLES WITH READ LOCK; UNLOCK TABLES
TABLET_ACTION_PROMOTE_SLAVE PromoteSlave        STOP SLAVE; RESET MASTER; RESET SLAVE; SHOW MASTER STATUS
TABLET_ACTION_RESTART_SLAVE RestartSlave        STOP SLAVE; RESET SLAVE; CHANGE MASTER TO; wait till Slave_IO_Running & Slave_SQL_Running; SELECT MASTER_POS_WAIT()
TABLET_ACTION_BREAK_SLAVES  BreakSlaves
TABLET_ACTION_SCRAP         Scrap

SHARD_ACTION_REPARENT       ReparentShard
SHARD_ACTION_REBUILD        RebuildShard

KEYSPACE_ACTION_REBUILD     RebuildKeyspace
=========================== =================== =====

action state

- queued

- running

- failed

KeyRange
--------

::

    SET GLOBAL vt_enable_binlog_splitter_rbr = 1;
    SET GLOBAL vt_shard_key_range_start = xx;
    SET GLOBAL vt_shard_key_range_end = yy;


WhatIsMissed
------------

- query router

- SHARD_ACTION_REPARENT

- binlog reader applier

  CreateReplicaTarget

  CreateReplicaSource

  ConfigureKeyRange
