======================
youtube's vitess/vtocc
======================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

Goal
====

Assumption
----------

mysql
^^^^^

- good at storage

- bad at scaling

  not able to coordinate many instances of a single logical schema 

::

                    client
                      |
                      | RPC/bson
                      |
            ---------------------------- 
         v |  Connection handler        |
         t |----------------------------|
         o |  QueryCache | SqlParser    |
         c |----------------------------|
         c |  Optimizer                 |
            ---------------------------- 
                      |
                      |------------------------------------------
                      |                         |         |      |
            ----------------------------      -----     -----   -----
           |  Connection handler        |     mysql     mysql   mysql
         m |----------------------------|
         y |  QueryCache | SqlParser    |
         s |----------------------------|
         q |  Optimizer                 |
         l |----------------------------|
           |  StorageEngines            |
            ---------------------------- 


