==============
OpenStack
==============

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


OpenStack是一个开源的Cloud操作系统， 它是由NASA和Racksapce主导的一个开源项目， 旨在提供一个开放的， 可大规模部署的云计算平台。 
通过这段时间对OpenStack的学习和研究， 发现它是一个建立在各种hypervisor基础上的管理服务总线， 提供了大量的基于http/https的REST api, 所以， 通过这些API， 应用就可以很方便的去管理这些计算资源。

Openstack里提供了一系列的应用平台， 这些平台基本上都是由Paste(一个Python的web框架)来驱动， 然后通过Greenlet(一个轻量级事件驱动框架)来提供性能上的提升。

greenlet
eventlet
paste
kombu

dnsmasq

mysql blocking: from eventlet import db_pool, which go through a thread pool

nova --debug list

======================= ========
Server Node             Service
======================= ========
Cloud Controller        nova-network nova-volume nova-api nova-scheduler
Compute                 hypervisor libvirt nova-compute
======================= ========

Nova
====

Features
--------

- IaaS

   offering essentially a virtual, bare machine as a service

- rebuild

- resize

- reboot

- snapshot

  OS Image

cloud controller => global state

::


            dashboard
                |
                | REST
                |
            nova-api
                |
                | amqp/produce
                |                       - direct
            RabbitMQ ------------------|- fanout
                |                       - topic
                | consume
                |
       ------------------------
      |       |       |        |
    worker  worker  worker  worker
                               |
                               | libvirt
                               |
                            hypervisor
                               |
                        -------------
                       |      |      |
                      vm1    vm2    vmN




                            |
       component ---------- | manager -------- component
                            |   


generate_request_id
