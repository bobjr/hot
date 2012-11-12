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

Architecture
============

通过配置文件xml知道某个ip在哪个ds/rack

Node.freeSpace = maxVolCnt - activeVolCnt

每个volume的最大空间的固定的：32G，因此cnt就觉得了可以容纳多大的空间

Write
-----

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
                    ------------------------
                   |            |           |
                DataNode    DataNode     DataNode
                                |
                              Store
                                |
                        ---------------
                       |       |       |
                    Volume  Volume  Volume(haystack)
                                       |
                                   ------
                                  |      |
                                 idex   data



Abstractions
------------

::


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
      |- Sequencer
      |     |
      |      - fileId
      |
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
    | file cookie     | 4B ---                  | items ...     |
    |-----------------|       |                 |---------------|
    | file key        | 8B    |                 |               |
    |-----------------|       |
    | data size       | 4B    |
    |-----------------|       | needle
    | []data          | xB    |
    |-----------------|       |
    | CRC checksum    | 4B    |
    |-----------------|       |
    | []padding       | xB ---
    |-----------------|
    | needle ....     |
    |-----------------|
    |                 |


- Needle

- Store


fid
---


#. VolumnId uint32

# File Key uint64(variable length)

#. File Cookie uint32(fixed length)

::

            3
            --------
        3,01637037d6
        - --
        1 2

      FileKey = (2+3)[0:len-4]

MasterNode
----------

::

    {VolumeId: <url, free size>}

VolumeNode
----------

::

    {key: <offset, size>}


