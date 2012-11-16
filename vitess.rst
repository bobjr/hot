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
                      |
            ---------------------------- 
           |  Connection handler        |
         m |----------------------------|
         y |  QueryCache | SqlParser    |
         s |----------------------------|
         q |  Optimizer                 |
         l |----------------------------|
           |  StorageEngines            |
            ---------------------------- 


