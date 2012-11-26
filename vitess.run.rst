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

