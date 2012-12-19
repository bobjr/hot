=========================
nsq - a realtime mq
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: NA
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

TODO
====

- nsqd

  maxBytesPerFile
  memQueueSize
  syncEvery


  tcp window

Features
============

- performance

  billions of msg per day

- ha

  - no SPOF

  - strong msg delivery guarantee

  - persistent

  - nsq_lookup multiple instances

    eventually consistent


Arch
====

Each channel receives all the messages from a topic. 

The channels buffer data independently of each other, preventing a slow consumer from causing a 
backlog for other channels. 

A channel can have multiple clients, a message (assuming successful delivery) will only be delivered 
to one of the connected clients, at random.

In practice, a channel maps to a downstream service consuming a topic.

For example:

::

    "clicks" (topic)
       |                             |- client
       |>---- "metrics" (channel) --<|- ...
       |                             |- client
       |
       |                                   |- client
       |>---- "spam_analysis" (channel) --<|- ...
       |                                   |- client
       |
       |                             |- client
       |>---- "archive" (channel) --<|- ...
                                     |- client


Arch

::

            nsqadmin
               |
        ------------------------------------------------------------
       |                                                            |
       |        /----------------+                                  |
       |        | RegistrationDB |                                  |
       |        +----------------/                                  |
       |              |                                             |
       |        nsqlookupd              nsqlookupd      nsqlookupd  |
       |              |                     |               |       |
        ------------------------------------------------------------
                     |  ^
           (OK, err) |  | IDENTIFY
                     |  | REGISTER $topic $channel
                     |  | UNREGISTER $topic $channel
                     |  | PING
                     V  |
        -------------------------------------
       |             |       |       |       |
     nsqd          nsqd    nsqd    nsqd    nsqd
       |
       |- topic
       |- topic
        - topic
            |
            |- channel
            |- channel
             - channel
                  |
              msg |
                  |- client(consumer)
                  |- client(consumer)
                   - client(consumer)


Protocol
========

consumer-nsqlookupd
-------------------

::

        foreach lookupdHTTPAddrs {
            producers = HTTP GET http://nsqlookupd/lookup?topic=$topic
            foreach producers {
                connectToNSQ(tcp_port, address)
            }
        }


nsqd-nsqlookupd
---------------

nsqlookupd will not housekeeping nsqd ping by timeout, it just identify EOF of the conn

on each PING, update LastUpdate to now(). 

http://lookupd/lookup?topic=xx will only return producers that has pinged within 5 minutes

RegistrationDB
^^^^^^^^^^^^^^

::

        Registration{"client", "", ""}
        Registration{"topic", $topic, ""}
        Registration{"channel", $topic, $channel}


Protocol
^^^^^^^^

::

        IDENTIFY
            {version":"0.2.16-alpha","tcp_port":4150,"http_port":4151,"address":"mac.local"}

        REGISTER $topic $channel
        UNREGISTER $topic $channel

        PING


nsqd
====

syncEvery       = flag.Int64("sync-every", 2500, "number of messages between diskqueue syncs")
msgTimeoutMs    = flag.Int64("msg-timeout", 60000, "time (ms) to wait before auto-requeing a message")
dataPath        = flag.String("data-path", "", "path to store disk-backed messages")
workerId        = flag.Int64("worker-id", 0, "unique identifier (int) for this worker (will default to a hash of hostname)")
memQueueSize    = flag.Int64("mem-queue-size", 10000, "number of messages to keep in memory (per topic)")
maxBytesPerFile = flag.Int64("max-bytes-per-file", 104857600, "number of bytes per diskqueue file before rolling")


::

        incomingMsg
        memoryMsg
        backend


        ${topic}.diskqueue.meta.dat
        ${topic}.diskqueue.${fileNum}.dat



        echo topic has a disk queue


        nsqd                nsqlookupd
         |                      |
         | lookupLoop           |
         |----------------------|
         |                      |


housekeeping
------------

::

                    topic                       channel                 client
                    -----                       -------                 ------
         PUT          |                          |                          |
     msg ------> incomingMsg                     |                          |
                      |                          |                          |
                      | router                   |                          |
                      |                          |                          |
             ---------------------               |                          |
            |                     |              |                          |
           backend          memoryMsg            |                          |
            |                     |              |                          |
             ---------------------               |                          |
                      |                          |                          |
                      | messagePump              |                          |
                      |                          |                          |
                      | PUT                      |                          |
                       --------------------> incomingMsg                    |
                                                 |                          |
                                                 | router                   |
                                                 |                          |
                                      -------------------------             |
                                     |                         |            |
                                   backend                memoryMsg         |
                                     |                         |            |
                                      -------------------------             |
                                                 |                          |
                                                 | messagePump              |
                                                 |                          |
                                              clientMsg                    SUB
                                                 |                          |
                                                  ------------------> messagePump
