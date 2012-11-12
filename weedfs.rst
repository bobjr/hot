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
      |
      |- []VolumeLayout
      |         |- replicationType
      |         |- {VolumeId: VolumeLocationList}
      |         |               |
      |         |                - []DataNode
      |         |
      |         |- pulse
      |         |- volumeSizeLimit
      |          - writables []VolumeId
      |
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
                                     |- id
                                     |- size
                                     |- replicationType
                                     |- fileCount
                                      - deleteCount




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


data file
^^^^^^^^^

superblock, 8 byte

::

     -----------------
    | 1(magic)        | 1B ---
    |-----------------|       |
    | replicationType | 1B    | superblock
    |-----------------|       |
    | 0(reserved)     | 6B ---
    |-----------------|
    | cookie          | 4B ---
    |-----------------|       |
    | id              | 8B    |
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



index file
^^^^^^^^^^

::

    byte
    ====
    0-7   key       64b
    8-11  offset    32b
    12-15 size      32b

::

    type VolumeId uint32

file
----

::

    type FileId struct {
        VolumeId storage.VolumeId
        Key uint64
        Hashcode uint32
    }

