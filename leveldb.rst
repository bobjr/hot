=================
leveldb explained
=================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: leveldb, a kv db with LSM-tree
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

Components
==========

::

    Get     Put     Delete
     |       |        |
      ----------------
           |
           | SkipList
           |
           |------------
           |            |
        MemTable    ImutableMemTable
                        |
                        | BackgroundCompaction
                        |
                    SSTable 
                        |
                        |- log
                        |- manifest
                         - current


Arch
====

Read
----

::

        Read                        ^
          |                         |
        MemTable                    | young
          |                         |
        ImmutableMemTable           |
          |                         |
        Level0 SSTable + Manifest   | data
          |                         | refreshness
        Level1 SSTable              |
          |                         |
        LevelN-1 STable             |
          |                         | old
        LevelN SSTable              |
