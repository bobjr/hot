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

stateless

DataSource
----------

原则上只要写操作幂等(也就是写操作与顺序无关)则都可以支持

- mysql

- lucene

- redis

- memcache

Replication
-----------

采用 replication tree，由 **gizzard** 负责分发，可以跨越 datacenter

为memcache提供了副本的功能


Internals
=========

Write
-----

把写操作入队列，异步发送。如果某个 replica fail，写操作会放入 error queue，并 backoff retry

Migration
---------


