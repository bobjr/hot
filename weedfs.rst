=========================
weed-fs
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: a distributed file system in golang.
              Another facebook haystack implementation in golang.

              It is a key~blob mapping.
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

TODO
====

- what if we prevent from dup file contents?

- how replication done?

- count is for what?

  sequencer return fid

Architecture
============

通过配置文件xml知道某个ip在哪个ds/rack

FileId:

::

    Key = Uint64toBytes(SequencerVal)
    Cookie = Uint32toBytes(rand.Uint32())

    final = vid, hex.EncodeToString(Key + Cookie)


Node.freeSpace = maxVolCnt - activeVolCnt

每个volume的最大空间的固定的：32G，因此cnt就觉得了可以容纳多大的空间

Write
-----

procedure

::

        dir/assign
            |
        
            |
        dir/assign
            |
        dir/assign


::


            Client            MasterNode           VolumeNode(s)
              |                    |                    |
              |  dir/assign?       |                    |
              |------------------->|                    |
              |                    |                    |
              | (fid, pubUrl)      |                    |
              |<-------------------|                    |
              |                                         |
              | POST pubUrl/fid                         |
              |------->-------------------------------->|
              |                                         |
              |                              {size: x}  |
              |<--------------------------------<-------|


Read
----

::


            Client                      MasterNode           VolumeNode(s)
              |                             |                    |
              | dir/lookup?volumeId=3       |                    |
              |---------------------------->|                    |
              |                             |                    |
              |   {locations: pubUrl}       |                    |
              |<----------------------------|                    |
              |                                                  |
              | GET pubUrl/fid                                   |
              |------------------------------------------------->|
              |                                                  |
              |                                     file bytes   |
              |<-------------------------------------------------|


Heartbeat
---------


TestCase
--------

=============================== =============================== =============
Case                            MasterNode                      VolumnNode(s)
=============================== =============================== =============
bottleneck
容量                            
数据迁移
Down                            X                               X
Recover                         X                               X
Restart                         X                               X
DisFail                         X                               X
NetFail                         X                               X
AddServer                                                       X
RmServer                                                        X
=============================== =============================== =============

::


                        VolumeNode          VolumeNode      VolumeNode
                          |                     |               |
                          | join                |               |
                          |                     |               |
                           -------------------------------------
                                           |
                                           | StartRefreshWritableVolumes
                                           |
                                       MasterNode


Internals
=========

::


                        Topology
                            |
      ---------------------------
     |              |            |
    DataCenter  DataCenter  DataCenter
                    |
          ----------------------
         |           |          |
        Rack        Rack       Rack
                     |
                    ------------------------                        ^
                   |            |           |                       |
                DataNode    DataNode     DataNode                   |
                                |                               master node
    -----------------------------------------------------------------------------
                                |                               volume node
                              Store                                 |
                                |                                   |
                        ---------------                             V
                       |       |       |
                    Volume  Volume  Volume(haystack)
                                       |
                                   ------
                                  |      |
                                index   data
                                  |      |
                                   ------
                                   needle



Abstractions
------------

::


      Sequencer     Topology ----------- 
         |              |               |
         |              | replicaType   |
         |              |               |
         |---<------VolumeLayout        |
         V              |               |lookup(vid)
      PickNewVid        | vid           |
                        |               |
                    DataNode <----------
                        |
                        | vid
                        |
                    VolumeInfo




                                 - writables []vid
                                |- vid2location {vid: []DataNode}
              replicaType       |                           |
    topology -------------> VolumeLayout                    |- ip:port
                                                            |- volumes {vid: VolumeInfo}



    Topology DataCenter Rack DataNode
       |        |        |      |
        ------------------------
                    |
                    | inheritance
                    V
           ------->Node<----------
          |         |             |
          |         |             |
           - parent-|- children -
                    |
                    |
                    |- FreeSpace()
                    |- Id()
                    |-
                    |-
                    

    Topology
      |
      |- Sequencer(fileId generator)
      |- Lookup(volumeId) -> []DataNode
      |
      |- []VolumeLayout(每种replica type一个VolumeLayout item)
      |         |- replicationType
      |         |- pulse
      |         |- volumeSizeLimit
      |         |- writables []VolumeId
      |          - {VolumeId: VolumeLocationList}
      |                         |
      |                          - []DataNode
      |          
       - DataCenter
           |
            - Rack
               |
                - DataNode
                    |
                    |- ip:port
                    |- publicUrl
                    |- lastSeenTimestamp
                    |- isDead
                     - {VolumeId: VolumeInfo}
                                     |
                                     |- id ----------------- 
                                     |- size                |
                                     |- replicationType     |
                                     |- fileCount           |
                                      - deleteCount         |
                                                            |
                        Volume(volumeId=filename)           |
             -----------------------------------------------
            | data                                  | index
     -----------------                           ---------------
    | 1(magic)        | 1B ---                  | @file key     | 8B ---
    |-----------------|       |                 |---------------|       |
    | replicationType | 1B    | superblock      | offset        | 4B    | 1 item
    |-----------------|       |                 |---------------|       |
    | 0(reserved)     | 6B ---                  | @data size    | 4B ---
    |-----------------|                         |---------------|
    | file cookie     | 4B --- --               | items ...     |
    |-----------------|       |  |              |---------------|
    | file key        | 8B    |  | header       |               |
    |-----------------|       |  |
    | data size       | 4B ------ 
    |-----------------|       | 
    | []data          | xB    |
    |-----------------|       | needle
    | CRC checksum    | 4B    |
    |-----------------|       |
    | []padding       | xB ---
    |-----------------|
    | needle ....     |
    |-----------------|
    |                 |

    each file meta space: data(4+8+4+4) + index(8+4+4) = 36



fid
---

#. VolumnId uint32

# File Key uint64(variable length)

#. File Cookie uint32(fixed length)

#. delta(optional)

::

            3
            --------
        3,01637037d6_3
        - --
        1 2

      FileKey = (2+3)[0:len-4]
