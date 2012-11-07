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

meta data
---------

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



