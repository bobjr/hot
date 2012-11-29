======================
youtube's vitess
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: vitess, the scalable mysql cluster framework explained.
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

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

    - /zk/global/vt/keyspaces/test_keyspace/shards/<shard id> => json(shard)

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/action

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/actionlog

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344

      this is the parent node for all its children
  
  - for slave

    - /zk/global/vt/keyspaces/test_keyspace/shards/0/test_nj-0000062344/test_nj-0000062345

    - /zk/test_nj/vt/tablets/0000062345 => json(tablet with parent info)
