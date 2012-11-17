====================
海量存储与检索
====================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 海量存储与检索的实现
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


锁
====

与单机事务有关的锁
---------------------

- Mutex

- RWLock

  因为读可以并发，所以效率稍高，但写和读不能同时进行

  ACID里的Isolation `repeatable read` 就是通过RWLock实现的: 

  如果一个人在事务中，那么他所有写过的数据，所有读过的数据，都给他来个锁，让其他小样儿都只能等在外面，直到
  数据库能确定所有更改已经全部完成了，没有剩下什么半拉子状态的时候，就解开所有的锁，让其他人可以读取和写入。

- MVCC

  CopyOnWrite，读取和写入之间互不影响

  如果一个人在事务中，会先申请一个事务ID,这个ID是自增的，每个事务都有他自己的唯一的ID，那么他写过的数据，都
  会被转变为一次带有当前事务ID的新数据，在读取的时候，则只会读取小于等于自己事务ID的数据。
  
  这样实现的东东，语义上来说，与可重复读就一样了。而如果读小于等于全局ID的数据，那么这样的实现，就是读已提交了


::

                                        isolation
                                            |
         -----------------------------------------------------------
        |                   |                   |                   |
    ReadUncommitted     ReadCommitted       RepeatableRead      Serializable
        |                   |                   |                   |
        |                   |-------------------|                   |
        |                   |        |          |                   |
      NoLock            ReadLock    MVCC      RWLock              Mutex
                                     |
                                     |
                             write with versionId
                                CopyOnWrite
                                     |
                                     | only read
                                     |
                            --------------------
                           |                    |
                        < myTid              < globalTid





2PC
---

Alice 要给 Bob 100 块:

Prepare(Alic-100) at 机器A -> prepare (Bob+100) at 机器b -> commit(Alice) -> commit(Bob)

两段提交的核心，是在prepare的阶段，会对所有该操作所影响的数据加锁，这样就可以阻止其他人（或机器）对他的访问
