==============
OpenStack
==============

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


2010-06 open source
OpenStack是一个开源的Cloud操作系统， 它是由NASA和Racksapce主导的一个开源项目， 旨在提供一个开放的， 可大规模部署的云计算平台。 
通过这段时间对OpenStack的学习和研究， 发现它是一个建立在各种hypervisor基础上的管理服务总线， 提供了大量的基于http/https的REST api, 所以， 通过这些API， 应用就可以很方便的去管理这些计算资源。

Openstack里提供了一系列的应用平台， 这些平台基本上都是由Paste(一个Python的web框架)来驱动， 然后通过Greenlet(一个轻量级事件驱动框架)来提供性能上的提升。

Rackspace
=========

founded in 1988, IPO NYSE

- 120,000 customers

- 628M revenue in 2009

- 3,000+ employees

- 3 data centers

  US, UK, HK

  65,000+ physical servers

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

        

    def generate_request_id():
        return 'req-' + str(uuid.uuid4())



Service
RpcProxy A helper class for rpc clients
Manager



nova-compute
------------

::


        server = service.Service.create(binary='nova-compute', topic='compute')                
            manager = nova.compute.manager.ComputeManager # 找到manager的实现类

        service.serve(server)
            server.start()
                manager.init_host()
                conn = create_connect(amqp) # 连接到rabbit mq server
                dispatcher = manager.create_rpc_dispatcher() # 初始化dispatcher,它会根据msg里版本信息判断兼容性
                conn.create_consumer(topic, dispatcher, fanout=False) # 注册consumer
                    proxy = ProxyCallback(dispatcher) # 注册consumer获取消息后的callback proxy
                                                      # callback使用代理，是为了把coroutine封装在这一层
                                                      # 以及对manager执行时异常的处理
                conn.create_consumer(topic + '.' + host, dispatcher, fanout=False)
                conn.create_consumer(topic, dispatcher, fanout=True)

                # the main loop
                conn.consume_in_thread()
                    consume
                        def _callback(raw_message): # 从rabbitmq处获得一条raw msg的处理过程
                            message = self.channel.message_to_python(raw_message)
                            try:
                                msg = rpc_common.deserialize_msg(message.payload)
                                callback(msg) # 调用ProxyCallback的__call__(msg)方法
                                    ctxt = unpack_context(self.conf, msg)
                                    method = msg.get('method')
                                    args = msg.get('args', {})
                                    version = msg.get('version', None)
                                    # 对每条msg，创建一个绿色进程来处理
                                    eventlet.pool.spawn_n(self._process_data)
                                        dispatcher.dispatch(ctxt, version, method, **args)
                                            getattr(manager, method)(ctxt, **kwargs)
                                        
                                message.ack() # ack msg for rabbitmq
                            except Exception:
                                LOG.exception(_("Failed to process message... skipping it."))

paste.deploy
------------

loadapp

entry point: app_factory


swift
-----

use case
########

- dropbox

  Amazon S3

- glance image

- log file

- backup



multi-tenant

meta data

object size <= 5GB

- Proxy Server

- Ring

  zone: a swift object server process

  partition

  device
  
  replica

- Storage Server

  Account server

  Container server

  Object server: /mount/data_dir/partition/hash_suffix/hash/object.ts

- Consistency Server

  Replicator

  Updater

  Auditor


quorum writes

::

    cd /etc/swift
    sudo swift-ring-builder account.builder create 18 3 1 # <part_power> <replicas> <min_part_hours>
    sudo swift-ring-builder container.builder create 18 3 1
    sudo swift-ring-builder object.builder create 18 3 1

    sudo swift-ring-builder account.builder add <zone>-<ip>:<port>/<device_name> <weight>

Volume
######

Zettabyte = 1,000,000 PB

100% of the data on earth today

2% of the data on earth in 2020


PUT /<api version>/<account>/<container>/<object>
                   --------   ---------   ------
                     |           |          |
                      ----------------------
                                 |
                      ecb25d1facd7c6760f7663e394dbeddb

no central db

raid not required

Components
##########

- Ring

- Updater

  process failed or queued updates

- Auditor

  verify integrity of objects, containers and accounts

- Proxy

  request routing, expose the public API



::

    account
      |
      |- container
            |
            |- object


            client
              |
              | REST
              |
            proxy
              |
              |- handles failures and handoff
              |
              |- controller, path_parts = get_controller(req.path) # AccountController/ContainerController/ObjectController
              |- controller = controller(self, **path_parts)
              |- req.environ['swift.trans_id'] = uuid.uuid4().hex
              |- handler = getattr(controller, req.method)
              |- handler(req)
              |
        ----------------------------------------------------
       |                            |                       |
    AccountController       ContainerController     ObjectController
       |
       |- partition, nodes = self.app.account_ring.get_nodes(self.account_name)
       |- shuffle(nodes) # load balance
       |- http_connect(node['ip'], node['port'], node['device'], partition)
