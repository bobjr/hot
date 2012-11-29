======================
youtube's vitess
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


How to run
==========

Installation
------------

::

        export GOPATH=$HOME
        mkdir -p $HOME/src/code.google.com/p/vitess
        hg clone -u weekly https://code.google.com/p/vitess/ $HOME/src/code.google.com
        cd $HOME/src/code.google.com/p/vitess
        export MYSQL_CONFIG=/usr/local/mysql/bin/mysql_config
        export DYLD_LIBRARY_PATH
        ./bootstrap.sh
        source dev.env

        cd go
        make


Run
---

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


New
===

::

    /Users/gaopeng/bin/mysqlctl -tablet-uid 9460 -port 9461 -mysql-port 9460 init
    mysql5 -S /vt/vt_0000009460/mysql.sock -u vt_dba -e create database vt_test ; set global read_only = off




[gaopeng@localhost ~/bin]$zk ls -R /zk/global
/zk/global/vt
/zk/global/vt/keyspaces
/zk/global/vt/keyspaces/test_keyspace
/zk/global/vt/keyspaces/test_keyspace/action
/zk/global/vt/keyspaces/test_keyspace/actionlog
/zk/global/vt/keyspaces/test_keyspace/actionlog/0000000000
/zk/global/vt/keyspaces/test_keyspace/actionlog/0000000001
/zk/global/vt/keyspaces/test_keyspace/actionlog/0000000002
/zk/global/vt/keyspaces/test_keyspace/actionlog/0000000003
/zk/global/vt/keyspaces/test_keyspace/shards
/zk/global/vt/keyspaces/test_keyspace/shards/0
/zk/global/vt/keyspaces/test_keyspace/shards/0/action
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000000
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000001
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000002
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000003
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000004
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000005
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog/0000000006
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062345
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062345/test_nj-0000062346
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062345/test_nj-0000062347
/zk/global/vt/keyspaces/test_keyspace/shards/1
/zk/global/vt/keyspaces/test_keyspace/shards/1/action
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog/0000000000
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog/0000000001
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog/0000000002
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog/0000000003
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog/0000000004
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog/0000000005
/zk/global/vt/keyspaces/test_keyspace/shards/1/test_nj-0000062349
/zk/global/vt/keyspaces/test_keyspace/shards/1/test_nj-0000062349/test_nj-0000062350





/zk/test_nj/vt
/zk/test_nj/vt/ns
/zk/test_nj/vt/ns/test_keyspace
/zk/test_nj/vt/ns/test_keyspace/0
/zk/test_nj/vt/ns/test_keyspace/0/master
/zk/test_nj/vt/ns/test_keyspace/0/replica
/zk/test_nj/vt/ns/test_keyspace/1
/zk/test_nj/vt/ns/test_keyspace/1/master
/zk/test_nj/vt/tablets
/zk/test_nj/vt/tablets/0000062344
/zk/test_nj/vt/tablets/0000062344/action
/zk/test_nj/vt/tablets/0000062344/actionlog
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000000
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000001
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000002
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000003
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000004
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000005
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000006
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000007
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000008
/zk/test_nj/vt/tablets/0000062344/actionlog/0000000009
/zk/test_nj/vt/tablets/0000062344/pid
/zk/test_nj/vt/tablets/0000062345
/zk/test_nj/vt/tablets/0000062345/action
/zk/test_nj/vt/tablets/0000062345/actionlog
/zk/test_nj/vt/tablets/0000062345/actionlog/0000000012
/zk/test_nj/vt/tablets/0000062345/actionlog/0000000013
/zk/test_nj/vt/tablets/0000062345/actionlog/0000000014
/zk/test_nj/vt/tablets/0000062345/actionlog/0000000015
/zk/test_nj/vt/tablets/0000062345/actionlog/0000000016
/zk/test_nj/vt/tablets/0000062345/pid
/zk/test_nj/vt/tablets/0000062346
/zk/test_nj/vt/tablets/0000062346/action
/zk/test_nj/vt/tablets/0000062346/actionlog
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000000
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000001
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000002
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000003
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000004
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000005
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000006
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000007
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000008
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000009
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000010
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000011
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000012
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000013
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000014
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000015
/zk/test_nj/vt/tablets/0000062346/actionlog/0000000016
/zk/test_nj/vt/tablets/0000062346/pid
/zk/test_nj/vt/tablets/0000062347
/zk/test_nj/vt/tablets/0000062347/action
/zk/test_nj/vt/tablets/0000062347/actionlog
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000000
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000001
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000002
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000003
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000004
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000005
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000006
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000007
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000008
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000009
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000010
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000011
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000012
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000013
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000014
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000015
/zk/test_nj/vt/tablets/0000062347/actionlog/0000000016
/zk/test_nj/vt/tablets/0000062347/pid
/zk/test_nj/vt/tablets/0000062348
/zk/test_nj/vt/tablets/0000062348/action
/zk/test_nj/vt/tablets/0000062348/actionlog
/zk/test_nj/vt/tablets/0000062348/actionlog/0000000000
/zk/test_nj/vt/tablets/0000062348/actionlog/0000000001
/zk/test_nj/vt/tablets/0000062348/pid
/zk/test_nj/vt/tablets/0000062349
/zk/test_nj/vt/tablets/0000062349/action
/zk/test_nj/vt/tablets/0000062349/actionlog
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000000
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000001
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000002
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000003
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000004
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000005
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000006
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000007
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000008
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000009
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000010
/zk/test_nj/vt/tablets/0000062349/actionlog/0000000011
/zk/test_nj/vt/tablets/0000062349/pid
/zk/test_nj/vt/tablets/0000062350
/zk/test_nj/vt/tablets/0000062350/action
/zk/test_nj/vt/tablets/0000062350/actionlog
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000000
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000001
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000002
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000003
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000004
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000005
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000006
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000007
/zk/test_nj/vt/tablets/0000062350/actionlog/0000000008
/zk/test_nj/vt/tablets/0000062350/pid
