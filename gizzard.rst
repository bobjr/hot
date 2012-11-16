==========================================
How twitter stores 250million tweets a day
==========================================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: Twitter's gizzard: datasource middleware
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


Intro
=====

解决分布式可以在client和middleware两个层次解决。

gizzard是后者。

Replication
-----------

采用 replication tree，由 **gizzard** 负责分发，可以跨越 datacenter


Internals
=========

Write
-----

把写操作入队列，异步发送。如果某个 replica fail，写操作会放入 error queue，并 backoff retry

Migration
---------


