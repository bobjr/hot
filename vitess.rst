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


Topology
--------

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
                           |      |       |        |      |
                           |      |       |        |      |
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


coordinator?

dbman groupd and only conn pooled with limited mysql instances?


Related Projects
================

- gizzard by twitter

  https://github.com/twitter/gizzard

  - datasource

    redis, mysql, lucene, etc

  -  all write operations must be idempotent

    does not guarantee that write operations are applied in order

  .. image:: https://github.com/twitter/gizzard/raw/master/doc/middleware.png?raw=true

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

- 36,015 line of code

- 173 go files

- golang plus python client

Features
--------

- logical vs physical database

- self management

  - external replication

  - range based sharding

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

Why
^^^

mysql:

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
           |  RPC Services              |
         v |----------------------------|
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


Usage
-----

Components
^^^^^^^^^^

=============== =========== ==============================
cmd             rpc server  desc
=============== =========== ==============================
vtctl           N           global mgmt & deployment tool
vttablet        Y           SqlQuery/TabletManager/UmgmtService rpc server, action agent watcher
vtaction        N           actions initiator
=============== =========== ==============================

ClientRpc
^^^^^^^^^

go and python currently, easy to migrate to php, ruby, etc.

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

    Execute("select uid, name from s_user_info where uid>:uid", 45)


ServerCluster
^^^^^^^^^^^^^

::

    vtctl CreateKeyspace /zk/global/vt/keyspaces/test_keyspace

    #init_tablet(tablet_type, keyspace, shard, zk_parent_alias, key_start, key_end)

    vtctl InitTablet /zk/test_nj/vt/tablets/0000062344 localhost 3700 6700 test_keyspace 0 master ""
    vtctl InitTablet /zk/test_nj/vt/tablets/0000062044 localhost 3701 6701 test_keyspace 0 replica /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-62344
    vtctl InitTablet /zk/test_nj/vt/tablets/0000041983 localhost 3702 6702 test_keyspace 0 replica /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-62344

    vttablet -port 6700 -tablet-path /zk/test_nj/vt/tablets/0000062344
    vttablet -port 6701 -tablet-path /zk/test_nj/vt/tablets/0000062044
    vttablet -port 6702 -tablet-path /zk/test_nj/vt/tablets/0000041983

    vtctl Ping /zk/test_nj/vt/tablets/0000062344
    vtctl RebuildShardGraph /zk/global/vt/keyspaces/test_keyspace/shards/0000000000000000-8000000000000000

    UpdateTablet
    SetReadOnly
    SetReadWrite
    DemoteMaster
    ChangeSlaveType
    Snapshot
    ReparentTablet

    ReparentShard
    ListShardTablets
    ListShardActions


    ApplySchema
    PreflightSchema



Architecture
------------

::

                vtctl   client
                  |        |
                   --------
                     |
                     |------ zkocc ------------ ZooKeeper
                     |                              |
            ------------------------------------------------
           |           |            |           |           |
           |           |            |           |           |
        vttablet    vttablet    vttablet    vttablet    vttablet
        --------    --------    --------    --------    --------
        mysqld      mysqld      mysqld      mysqld      mysqld


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

                               -------
                              | vtctl |
                               -------
                                 |
                         produce | put
                                 V
                                ----
             ----------------> | zk |            
            |                   ----
            |                    |
            | router              --------------
            |                                   |
            |                           consume | watch action
            |                                   |
            |             ----------------------|-----------------------------------
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
     |        |--- ns
     |        |     | 
     |        |     |- <keyspace> => json(SrvKeyspace{[]SrvShard{KeyRange, map[string]VtnsAddrs, readOnly}, TabletTypes []string})
     |        |             |
     |        |             |- <shard id>
     |        |                  |
     |        |                  |- <db type> => json(VtnsAddrs{uid, host, port})
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
                            |- actionlog
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
TABLET_ACTION_DEMOTE_MASTER DemoteMaster        SET GLOBAL read_only=ON; FLUSH TABLES WITH READ LOCK; UNLOCK TABLES; SHOW MASTER STATUS; set tablet readonly
TABLET_ACTION_PROMOTE_SLAVE PromoteSlave        STOP SLAVE; RESET MASTER; RESET SLAVE; SHOW MASTER STATUS
TABLET_ACTION_RESTART_SLAVE RestartSlave        STOP SLAVE; RESET SLAVE; CHANGE MASTER TO; wait till Slave_IO_Running & Slave_SQL_Running; SELECT MASTER_POS_WAIT()
TABLET_ACTION_BREAK_SLAVES  BreakSlaves
TABLET_ACTION_SCRAP         Scrap

SHARD_ACTION_REPARENT       ReparentShard
SHARD_ACTION_REBUILD        RebuildShard

KEYSPACE_ACTION_REBUILD     RebuildKeyspace
=========================== =================== =====

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


Implementation
==============

vttablet
--------

::

        read my.cnf
            |
        connect to zk
            |
        start action agent
            |   |
            |   |- what if tablet type changed?
            |   |- read tablet info from zk
            |   |- create pid znode(EPHEMERAL)
            |   |           |
            |   |           |- if exists, delete beforehead
            |   |           |- watch this znode: if delete, stop watch
            |   |
            |   |- actionEventLoop
            |
        start tablet manager rpc server
            |   |
            |   |- SlavePosition
            |   |- WaitSlavePosition
            |   |- MasterPosition
            |   |- StopSlave
            |   |- GetSlaves
            |
        start sql query rpc server
            |   |
            |   |- GetSessionId
            |   |- Begin
            |   |- Execute
            |   |- Commit/Rollback
            |
            |


vtctl
-----

::

        conn to zk
        create Wrangler

health check master db

SmartClient
-----------

sql driver
^^^^^^^^^^

- vttablet

  vttp://hostname:port/dbname

- vtdb

  vtzk://host:port/zkpath/dbType

  vtdb-zkocc

::

        in charge of a keyspace

        read zk /zk/local/vt/ns/<keyspace>

        get all shards info naming.SrvKeyspace



Routing
^^^^^^^

::

    client.Open('vtzk://host:port/zk/local/vt/ns/<keyspace>/<dbType>')
                                 --------------------------
    client.Begin()
    client.Execute('select * from s_user_info where uid>:uid', 123)

    read /zk/local/vt/ns/<keyspace>/<dbType>
        |
    get all related tablet server, each has a KeyRange
        |
    parse sql by bind vals
        |
    for each target tabletserver, connect and rpc call SqlQuery.Execute(sql)
        |
    final result


Client
------

Reshard
-------

ExecPlan
--------

explain

plan => query LRUCache

reloadSchema ticker

get index

ticker

    select table_name, table_type, unix_timestamp(create_time), table_comment from information_schema.tables where table_schema = database()

    show index from table_name

getScore Cardinality

DDL
^^^

::

    after exec ddl, schemaInfo.DropTable(ddlPlan.TableName)
    if ddlPlan.Action != sqlparser.DROP { // CREATE, ALTER, RENAME
        qe.schemaInfo.CreateTable(ddlPlan.NewName)
    }


Replication
-----------

LogicalDb
---------


Bottleneck
----------

zkocc
