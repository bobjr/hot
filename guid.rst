=========================
how to generate GUID
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: NA
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


noeqd
=====

Related
-------

https://github.com/twitter/snowflake

Implementation
--------------

- generate multiple guid at once

- clock skew protection

  ntp will let your fast clock repeat a few milliseconds

  just fail the client request

- HA implemented on client

  serverAddrs []string

  try next serverAddr when fail

- seq increment within a millisecond

  max 4096 seq in a millisecond

- alg

  if too many seq in a millisecond, wait till next millisecond

  ::

    millisecond datacenterId workerId seqInThisMillisecond
    ----------- ------------ -------- -------------------------
    41b         5b           5b       12
