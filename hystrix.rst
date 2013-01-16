==============
Hystrix
==============

:Author: Gao Peng <funky.gao@gmail.com>

.. contents:: Table Of Contents
.. section-numbering::


Numbers
=======

- 10+ billion thread isolated command execution/day

- 200+ billion semephore isolated command execution/day

- kx

  ::

        cd /kx/dlog/130115
        ls -l|wc -l
        2985  => 3k

        lzop -dcf lz.130115-152432|grep '/SAMPLE:1000/A'|wc -l
        11745 => 20k

        假如完全采样
        1000=1k 

        完全采样则该时段总的中间层调用次数
        20k * 1k = 20M 

            |
            V

        20M * 2k = 40 billion/day

- 40+ thread pools
  
  each 5-20 threads

- 100+ HystrixCommand types

Features
========

- Latency and Fault Tolerance

  Resilience engineering

- Realtime Operations

- Request Collapsing

- Monitor and alert

  Dashboard



CircuitBreaker Pattern
======================

::

          --------- Closed <-----------
         |                             |
         | exceed                      | conn
         | failure                     | succeed
         | threshold                   |
         |                             |
         V    retry timeout pass       |
        Open ---------------------> HalfOpen
             <---------------------
              conn fail

KeyImplementation
=================

ExecutionIsolationStrategy
--------------------------

- THREAD

- SEMAPHORE


HystrixEventType
----------------

- SUCCESS

- FAILURE

- TIMEOUT
  
- SHORT_CIRCUITED
  
- THREAD_POOL_REJECTED
  
- SEMAPHORE_REJECTED
  
- FALLBACK_SUCCESS
  
- FALLBACK_FAILURE
  
- FALLBACK_REJECTION
  
- EXCEPTION_THROWN
  
- RESPONSE_FROM_CACHE
  
- COLLAPSED

Failure
=======

- A request to remote service timed out

- The thread pool and bounded task queue used to interact with a service dependency are at 100% capacity

- The client library used to interact with a service dependency throws an exception


Fallback
========

- Custom fallback

- Fail silent

- Fail fast

Method
======

fallback options


fail fast and rapidly recover


decorator for each service

- track result of each call

  fail

- latency

::

                            UserRequest(1billion/d)
                                |
                                | fan out to 6billion/d upstream calls
                                |
            ---------------------------------------------------
           |            |           |           |              |
        Service     Service     Service      Service        Service
                                                               |
                         --------------------------------------------------
                        |           |           |              |           |
                    Service     Service      Service        Service     Service


::



                    HystrixThreadPool
                          |
                          | by key
                          |
                    HystrixCommand
                          |
                          | produce
                          |
                          |             consume
                HystrixCommandMetrics ---------- HystrixCircuitBreaker

