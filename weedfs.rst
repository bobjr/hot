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
                                       MasterNode


Internals
=========

Abstractions
------------

- Needle

- Volume

- Store

- Node

  - Rack

  - DataCenter

- Sequence


::

            Store
             |       
             |- Volume       
             |- Volume       
             |- Volume       
             |       
                    

fid
---

#. VolumnId

#. File Key

#. File Cookie(4 Byte)

::

            3
            --------
        3,01637037d6
        - --
        1 2

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

    byte0 1
    byte1 replicationType
    byte2-7 0


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

topology
--------
