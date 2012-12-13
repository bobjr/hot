=========================
weed-fs
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: A distributed file system in golang.
              
              Another facebook haystack implementation in golang.

              It is a key~blob mapping.

.. contents:: Table Of Contents
.. section-numbering::

TODO
====

- what if we prevent from dup file contents?

- what if repl 3 fail 1?

  what if it recovers?

- count is for what?

  sequencer return fid

Architecture
============

通过配置文件xml知道某个ip在哪个ds/rack

fid
---

#. VolumnId uint32

#. File Key uint64(variable length)

#. File Cookie uint32(fixed length)

#. delta(optional)

::

            3
            --------
        3,01637037d6_3
        - --
        1 2

      FileKey = (2+3)[0:len-4]

::

    Key = Uint64toBytes(SequencerVal)
    Cookie = Uint32toBytes(rand.Uint32())

    final = VolumeId, hex.EncodeToString(Key + Cookie)


Node.freeSpace = maxVolCnt - activeVolCnt

每个volume的最大空间的固定的：32G，因此cnt就觉得了可以容纳多大的空间

Heartbeat
---------

默认5秒钟

join
^^^^

::

        dataNode {
            on startup {
                load existing volumes
            }

            every 5s {
                post http://master/dir/join {
                    my{
                           ip, port, publicUrl, maxVolumeCount,
                           stats {
                               each volume{vid, size, repType}
                           }
                       }
                }
            }
        }

        master {
            RegisterVolumes {
                根据datanode ip，获取或创建其datacenter, rack
                根据lastSeen判断是新加入的还是recover的datanode {
                    如果在3次心跳，还没有某个datanode的join，则认为它Dead {
                        UnRegisterDataNode {
                            foreach this datanode's volumes {
                                its volumeLayout.setVolumeUnavailable(dn, vid) {
                                    从该 vid2location[vid] 里去除该dn
                                    if 剩下的dn数 < replicaCount {
                                        removeFromWritable(vid)
                                    }
                                }
                            }
                            adjust active volume acount
                            unlink this datanode
                        }
                    }
                }

                if recover {
                    tell topology recovery via channel
                    topology long run goroutine {
                        RegisterRecoveredDataNode {
                            foreach this datanode's volumes {
                                vl.SetVolumeAvailable(dn, vid) {
                                    vid2location[vid].Add(dn)
                                    如果该vid对应的datanode数 >- replicaCount {
                                        setVolumeWritable(vid) // this vid become writable
                                    }
                                }
                            }
                        }
                    }
                } else {
                    add child node to this rack
                }

                foreach volume {
                    根据 repType 找到其 volume layout, then {
                        register volume {
                            if 该vid对应的datanode数 == replicaCount {
                                writables = append(writables, vid)
                            }
                        }
                    }
                }
            }
        }


Write
-----

::


            Client                                  MasterNode           VolumeNode(s)
              |                                         |                    |
              | dir/assign?replication=x&count=y        |                    |
              |---------------------------------------->|                    |
              |                                         |                    |
              | (fid, pubUrl)                           |                    |
              |<----------------------------------------|                    |
              |                                                              |
              | POST pubUrl/fid                                              |
              |------->----------------------------------------------------->|
              |                                                              |
              |                                                   {size: x}  |
              |<--------------------------------<----------------------------|
              |                                                              |


assign
^^^^^^

::

    根据repType让topology预留volume layout空间 {
        根据repType余留1-3个volume => [{volServer, vid=t.NextVolumeId()}, ...].foreach {
            http://volServer/admin/assign_volume?volume=$vid&replicationType=$repType {
                open(volumeDataFile, O_CREAT|O_RDWR)
                readOrWriteSuperBlock()
                open(volumeIndexFile, O_CREAT|O_RDWR)
            }
        }

        这样，在master上就有了这样的topoloty map:
        {
            repType: VolumeLayout {
                writables []vid
                vid: {
                    []DataNode
                }
            }
        }
    }

    根据topology目前该repType的volume layout，选择出一个dataNode {
        从writables里随机取一个vid，从而得到该vid上服务的datanodes = []DataNode
        fid = NewFileId(vid, nextSeq, rand.Uint32())
        datanode = datanodes[0]
    }

    Q: 如果拿到assign了，不去datanode上传，会怎样?
    A:


postFile
^^^^^^^^

::

    new Needle from request
    store.Write(vid, needle)a
    replicaDataNodes = post('http://master/dir/lookup?volumeId=vid')
    foreach replicaDataNodes {
        upload('http://datanode/')
    }

    if any replica upload fails {
        delete all uploads
    }

Read
----

::


            Client                      MasterNode           VolumeNode(s)
              |                             |                    |
              | dir/lookup?volumeId=3       |                    |
              |---------------------------->|                    |
              |                             |                    |
              |   {locations: pubUrl}       |                    |
              |<----------------------------|                    |
              |                                                  |
              | GET pubUrl/fid                                   |
              |------------------------------------------------->|
              |                                                  |
              |                                     file bytes   |
              |<-------------------------------------------------|


datanode
^^^^^^^^

::

        if !store.HasVolume(vid) {
            lookup where is the vid
            redirect to that datanode

            Q: 什么时候会出现这种情况
        }

        get needle info from index
        dataFile.seek(needle.offset)
        dataFile.read(needle.size)


TestCase
--------

=============================== =============================== =============
Case                            MasterNode                      VolumnNode(s)
=============================== =============================== =============
bottleneck
容量                            
数据迁移
Down                            X                               X
Recover                         X                               X
Restart                         X                               X
DisFail                         X                               X
NetFail                         X                               X
AddServer                                                       X
RmServer                                                        X
=============================== =============================== =============

::


                        VolumeNode          VolumeNode      VolumeNode
                          |                     |               |
                          | join                |               |
                          |                     |               |
                           -------------------------------------
                                           |
                                           | StartRefreshWritableVolumes
                                           |
                                       MasterNode


Internals
=========

::


                        Topology
                            |
      ---------------------------
     |              |            |
    DataCenter  DataCenter  DataCenter
                    |
          ----------------------
         |           |          |
        Rack        Rack       Rack
                     |
                    ------------------------                        ^
                   |            |           |                       |
                DataNode    DataNode     DataNode                   |
                                |                               master node
    -----------------------------------------------------------------------------
                                |                               volume node
                              Store                                 |
                                |                                   |
                        ---------------                             V
                       |       |       |
                    Volume  Volume  Volume(haystack)
                                       |
                                   ------
                                  |      |
                                index   data
                                  |      |
                                   ------
                                   needle



Abstractions
------------

::


      Sequencer     Topology ----------- 
         |              |               |
         |              | replicaType   |
         |              |               |
         |---<------VolumeLayout        |
         V              |               |lookup(vid)
      PickNewVid        | vid           |
                        |               |
                    DataNode <----------
                        |
                        | vid
                        |
                    VolumeInfo




                                 - writables []vid
                                |- vid2location {vid: []DataNode}
              replicaType       |                           |
    topology -------------> VolumeLayout                    |- ip:port
                                                            |- volumes {vid: VolumeInfo}



    Topology DataCenter Rack DataNode
       |        |        |      |
        ------------------------
                    |
                    | inheritance
                    V
           ------->Node<----------
          |         |             |
          |         |             |
           - parent-|- children -
                    |
                    |
                    |- FreeSpace()
                    |- Id()
                    |-
                    |-
                    

    Topology
      |
      |- Sequencer(fileId generator)
      |- Lookup(volumeId) -> []DataNode
      |
      |- []VolumeLayout(每种replica type一个VolumeLayout item)
      |         |- replicationType
      |         |- pulse
      |         |- volumeSizeLimit
      |         |- writables []VolumeId
      |          - {VolumeId: VolumeLocationList}
      |                         |
      |                          - []DataNode
      |          
       - DataCenter
           |
            - Rack
               |
                - DataNode
                    |
                    |- ip:port
                    |- publicUrl
                    |- lastSeenTimestamp
                    |- isDead
                     - {VolumeId: VolumeInfo}
                                     |
                                     |- id ----------------- 
                                     |- size                |
                                     |- replicationType     |
                                     |- fileCount           |
                                      - deleteCount         |
                                                            |
                        Volume(volumeId=filename)           |
             -----------------------------------------------
            | data                                  | index
     -----------------                           ---------------
    | 1(magic)        | 1B ---                  | @file key     | 8B ---
    |-----------------|       |                 |---------------|       |
    | replicationType | 1B    | superblock      | offset        | 4B    | 1 item
    |-----------------|       |                 |---------------|       |
    | 0(reserved)     | 6B ---                  | @data size    | 4B ---
    |-----------------|                         |---------------|
    | file cookie     | 4B --- --               | items ...     |
    |-----------------|       |  |              |---------------|
    | file key        | 8B    |  | header       |               |
    |-----------------|       |  |
    | data size       | 4B ------ 
    |-----------------|       | 
    | []data          | xB    |
    |-----------------|       | needle
    | CRC checksum    | 4B    |
    |-----------------|       |
    | []padding       | xB ---
    |-----------------|
    | needle ....     |
    |-----------------|
    |                 |

    each file meta space: data(4+8+4+4) + index(8+4+4) = 36



