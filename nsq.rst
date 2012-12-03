=========================
nsq - a realtime mq
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: NA
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


Features
============

- performance

  billions of msg per day

- ha

  - no SPOF

  - strong msg delivery guarantee

  - persistent


Arch
====

::

        client

                    nsqlookupd(4160/1) ------------ nsadmin(4171)
                        |
                        | register
                        | heartbeat
                        |
                        |
        -------------------------------------
       |             |       |       |       |
    nsqd(4150/1)    nsqd    nsqd    nsqd    nsqd
       |
       |- topic
       |- topic
        - topic
            |
            |- channel
            |- channel
             - channel
                  |
                  |- client(consumer)
                  |- client(consumer)
                   - client(consumer)


Protocol
========

Client
------

::

    client                  nsqd
       |                      |
       | SUB                  |
       |--------------------->|
       |                      |
       | RDY n                |
       |--------------------->|
       |                      |
       |                      |
       |                      |
       |                      |
