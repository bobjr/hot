=========================
weed-fs NoFS
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: a distributed file system in golang
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

Background
==========
Another facebook haystack implementation in golang.

Features
========

meta data management
--------------------

::

    volumn server1      volumn server2      volumn serverN
    --------------      --------------      --------------
            |                  |                   |
             --------------------------------------
                               |
                        central master

space
-----
Each file metadata is 40B and lookup with O(1).

extras
------

- replication

- auto gzip

- auto add/rm server without re-balancing


