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

  - nsq_lookup multiple instances

    eventually consistent


Arch
====

Usage
-----

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


Design
------

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
                     |  | PING(15s)
                     V  |       
        ------------------------------------------------
       |                        |       |       |       |
       | lookupLoop             |       |       |       |
       |                        |       |       |       |
     nsqd                      nsqd    nsqd    nsqd    nsqd
       |                                     
       |- idPump(workerId)                  
       |                                   
       |- httpServer
       |
       |             NOP
       |             PUB MPUB
       |             SUB RDY FIN REQ CLS
       |- tcpServer <----------------------------------------------> nsq reader
       |    |                                                            ^
       |  IOLoop()                                                       |
       |    |                                                            |
       |    | SUB topic/chan                                             |
       |     ----------------- messagePump                               |
       |                            |                                    |
       |                            |- heartbeat                      - Send
       |                             - client.Channel.clientMsgChan -|
       |                                            ^                 - StartInFlightTimeout
       |                                            |
        - topicS                                     ---------------------------------------
            |                                                                               |
            |- router()                                                                     |
            |    |                                                                          |
            |     - incomingMsgChan => [memoryMsgChan | backend]                            |
            |                                                                               |
            |- if any channel, messagePump()                                                |
            |    |                                                                          |
            |     - [memoryMsgChan | backend] => dup(msg) => channel.incomingMsgChan        |
            |                                                                               |
             - channelS                                                                     |
                  |                                                                         |
                  |- incomingMsg => memoryMsg => inflighMsg => deferredMsg                  |
                  |                                                                         |
                  |- messagePump()                                                          |
                  |     |                                                                   |
                  |      - [memoryMsgChan | backend] => clientMsg --------------------------
                  |
                  |- router()
                  |     |
                  |      - incomingMsgChan => [memoryMsgChan | backend]
                  |
                  |- deferredWorker()
                   - inFlightWorker()


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
                      V                          |                          |
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
                                                                            |
                                                         ----------------------------------
                                                        |               |                  |
                                                     clientMsg      heartbeat           ExitChan
                                                        |
                                               -------------------------
                                              |                         |
                                            tcp Send            channel.StartInFlightTimeout





::

    
    msgSize  timestamp        attempts  msgId                            msgBody
    -------- ---------------- ----      -------------------------------- -------------
    4B       8B               2B        16B

