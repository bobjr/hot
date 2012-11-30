======================
youtube's vitess
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: vitess, the scalable mysql cluster framework explained.
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

Features
========

- Bind variables for queries, backed by a query cache
  
  To avoid repeated parsing and enable efficient reuse of query plans.

- Transaction management

- DML annotation
  
  Every DML is rewritten to include a comment field at the end of a query identifying the primary key of the rows it changed.

- Update stream

  - pk change notifications

  - derived from binlog

  - eventual consistency

    out of order

- roles

- DDL

  Many DDLs cannot be performed on high traffic live systems due to their locking requirements. 
  
  Vitess will allow you to coordinate such rollout with un-noticeable downtime by deploying the DDL to offline replicas and a reparenting process


Abstraction
===========

- keyspace

  default tablet db name = vtDbPrefix('vt_') + tablet.Keyspace

  All tables that are indexed by a set of keys are known as a keyspace, which basically represents the logical database that combines all the shards that store them.

- shard

  一个shard内只有1个 masterTablet

- cell

  属于某个tablet


::


        keyspace
          shard
            tablet

tabletReplicationPath = /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344

mysqlctl
========

Init
----

exec data/bootstrap/_vt_schema.sql

::

    CREATE DATABASE _vt
    CREATE TABLE _vt.replication_log
    CREATE TABLE _vt.reparent_log


ZK
==

zkctl
-----


::

    // server_id's must be 1-255, global id's are 1001-1255 mod 1000
    // <server_id>@<hostname>:<leader_port>:<election_port>:<client_port>
    config := "255@localhost:2889:3889:2182,1255@localhost:2890:3890:2183"

    create /vt/zk_*/ related files
    start up 'java org.apache.zookeeper.server.quorum.QuorumPeerMain'


ConnCache
---------

ZkPathToZkAddr /etc/zookeeper/zk_client.json

for each cachedConn, a dedicated goroutine loops to watch out for STATE_EXPIRED_SESSION & STATE_CLOSED

::

                    MetaConn
                       |
                    ConnCache - ConnForPath(zkPath string) - at most one zk conn per cell
                       |
                --------------
               |              |
             ZkConn         ZkoccConn
               |              |
               |              | rpc
               | c api        |
               |            zkocc
               |              |
                --------------
                       |
                    ZooKeeper

vtctl
=====

CreateKeyspace
--------------

/zk/global/vt/keyspaces/test_keyspace

- /zk/global/vt/keyspaces/<keyspace>

- /zk/global/vt/keyspaces/<keyspace>/action

- /zk/global/vt/keyspaces/<keyspace>/actionlog


InitTablet
----------

/zk/global/vt/keyspaces/test_keyspace/shards
/zk/global/vt/keyspaces/test_keyspace/shards/0
/zk/global/vt/keyspaces/test_keyspace/shards/0/action
/zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344/test_nj-0000062345
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344/test_nj-0000062346
/zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344/test_nj-0000062347
/zk/global/vt/keyspaces/test_keyspace/shards/1
/zk/global/vt/keyspaces/test_keyspace/shards/1/action
/zk/global/vt/keyspaces/test_keyspace/shards/1/actionlog
/zk/global/vt/keyspaces/test_keyspace/shards/1/test_nj-0000062349
/zk/global/vt/keyspaces/test_keyspace/shards/1/test_nj-0000062349/test_nj-0000062350


/zk/test_nj/vt/tablets/0000062344
/zk/test_nj/vt/tablets/0000062344/action
/zk/test_nj/vt/tablets/0000062344/actionlog
/zk/test_nj/vt/tablets/0000062345
/zk/test_nj/vt/tablets/0000062345/action
/zk/test_nj/vt/tablets/0000062345/actionlog
/zk/test_nj/vt/tablets/0000062346
/zk/test_nj/vt/tablets/0000062346/action
/zk/test_nj/vt/tablets/0000062346/actionlog
/zk/test_nj/vt/tablets/0000062347
/zk/test_nj/vt/tablets/0000062347/action
/zk/test_nj/vt/tablets/0000062347/actionlog
/zk/test_nj/vt/tablets/0000062348
/zk/test_nj/vt/tablets/0000062348/action
/zk/test_nj/vt/tablets/0000062348/actionlog
/zk/test_nj/vt/tablets/0000062349
/zk/test_nj/vt/tablets/0000062349/action
/zk/test_nj/vt/tablets/0000062349/actionlog
/zk/test_nj/vt/tablets/0000062350
/zk/test_nj/vt/tablets/0000062350/action
/zk/test_nj/vt/tablets/0000062350/actionlog

if not master, auto set parent and replication path

/zk/test_nj/vt/tablets/0000062344, uid = 0000062344

- /zk/test_nj/vt/tablets/0000062344 => json(tablet)

- /zk/test_nj/vt/tablets/0000062344/action

- /zk/test_nj/vt/tablets/0000062344/actionlog

- CreateTabletReplicationPaths 

  - for master

    - /zk/global/vt/keyspaces/test_keyspace/shards/<shard id> => json(Shard)

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/action

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344

      this is the parent node for all its children
  
  - for slave

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344/test_nj-0000062345

    - /zk/test_nj/vt/tablets/0000062345 => json(tablet with parent info)


RebuildShardGraph
-----------------

/zk/global/vt/keyspaces/test_keyspace/shards/0

- 通过action实现lock

- /zk/global/vt/keyspaces/test_keyspace/shards/0 => json(Shard)

  之前json里的空的

  递归变量FindAllTabletAliasesInShard，然后写入 /zk/global/vt/keyspaces/test_keyspace/shards/0


RebuildKeyspaceGraph
--------------------

/zk/global/vt/keyspaces/test_keyspace

对该keyspace下的所有shards，都执行 RebuildShardGraph

build /zk/test_nj/vt/ns/test_keyspace

::

    /zk/test_nj/vt/ns/test_keyspace/0               => json(SrvKeyspace)
    /zk/test_nj/vt/ns/test_keyspace/0/master        => json(VtnsAddrs)
    /zk/test_nj/vt/ns/test_keyspace/0/rdonly        => json(VtnsAddrs)
    /zk/test_nj/vt/ns/test_keyspace/0/replica       => json(VtnsAddrs)
    /zk/test_nj/vt/ns/test_keyspace/1               => json(SrvKeyspace)
    /zk/test_nj/vt/ns/test_keyspace/1/master        => json(VtnsAddrs)
    /zk/test_nj/vt/ns/test_keyspace/1/replica       => json(VtnsAddrs)


ReparentShard
-------------

vtctl ReparentShard /zk/global/vt/keyspaces/test_keyspace/shards/0 /zk/test_nj/vt/tablets/0000062344

::

    从 zk 读取 shardInfo
    从 shardInfo 获取 currentMasterTablet

    create SHARD_ACTION_REPARENT for lock

    从shardInfo里构造所有的 slaveTablets，形成 slaveTabletMap := make(map[uint32]*tm.TabletInfo)
    
    if currentMasterTablet != electMasterTablet {
        if currentMasterTablet is master {
            demoteMaster(currentMasterTablet)
        }

        构造需要restart的slave列表，其中lag类型被排除

        对每个restartable slave，检查与master position的数据一致性
    } else {
        // forcing reparent to same master
        foreach slave in slaveTabletMap {
            STOP SLAVE;
        }

        break currentMasterTablet slaves {
            INSERT INTO _vt.replication_log (time_created_ns, note) VALUES
            SET sql_log_bin = 0
            DELETE FROM _vt.replication_log WHERE time_created_ns = %v
            SET sql_log_bin = 1
            INSERT INTO _vt.replication_log (time_created_ns, note) VALUES
        }
    }

    promoteSlave(electMasterTablet) {
        if zk(action/restart_slave_data.json) exists {
            error
        }
        if master {
            show master status;
        } else {
            show slave status;
        }
        reset master;
        reset slave;
        show master status;
        INSERT INTO _vt.replication_log (time_created_ns, note) VALUES (1354179516856589000, 'reparent check')
        show master status;
        INSERT INTO _vt.reparent_log (time_created_ns, last_position, new_addr, new_position, wait_position) 


        delete old zk replication graph
        update zk tablet node
        create new zk replication graph
    }

    foreach slave {
        restart slave {
            stop slave;
            reset slave;
            change master to ...
            start slave;

            wait till Slave_SQL_Running and Slave_IO_Running

            SELECT MASTER_POS_WAIT

            SELECT * FROM _vt.replication_log WHERE time_created_ns = xx
        }
    }

    if most(slaves).restartSuccess {
        enable write for electMasterTablet
    }

    rebuild shard graph

    unlock


ApplySchemaShard
----------------

-sql='create table xxx' /zk/global/vt/keyspaces/test_keyspace/shards/0

::

    on master of this shard {
        PreflightSchema(ddl) {
            get current schemas of all tables in this db

            SET sql_log_bin = 0; { // session based
                DROP DATABASE IF EXISTS _vt_preflight;
                CREATE DATABASE _vt_preflight;
                USE _vt_preflight;

                replay current schemas on db(_vt_preflight);
                apply the ddl on db(_vt_preflight);

                get schemas of db(_vt_preflight)

                now, we have the (beforeSchema, afterSchema)

                DROP DATABASE _vt_preflight;  
            }
        }
    }

    lockAndApplySchemaShard {
        lock shard

        FindAllSlaveTabletAliasesInShard
        foreach slave {
            get current schemas

            compare current schemas with preflight schemas, return if all the same

            if tablet.IsServingType() {
                change type to TYPE_SCHEMA_UPGRADE
            }

            apply schema to this slave {
                ExecuteMysqlCommand(ddl)
            }

            if tablet.IsServingType() {
                change type back to beforeType
            }
        }


        unlock shard
    }


    

vttablet
========

start
-----

vttablet -port 6701 -tablet-path /zk/test_nj/vt/tablets/0000062344 -logfile /vt/vt_0000062344/vttablet.log 



mysql
=====

my.cnf
------

- log-bin=[basename]

  bin log files basename

- log-slave-updates


control cmd
-----------

- set sql_log_bin=0|1

  enable/disable binary logging

- binlog-do-db / binlog-ignore-db

  on master, control which db do/ignore replication

- replicate-do-db / replicate-ignore-db

  on slave
