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


