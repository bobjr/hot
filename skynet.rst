==============
skynet
==============

:Author: Gao Peng <funky.gao@gmail.com>
:Description: Skynet is a framework for distributed services in Go.
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::

Arch
====

::



                Client                       Doozerd                         Server
                  |                            |                                |
                  | NewClient(config)          |                                |
                  |                            |                                |
                  | Dial                       |                                |
                  |--------------------------->|                                |
                  |                            |                                |
                  | NewInstanceMonitor         |                                |
                  |--------------------------->|                                |
                  |                            |                                |
                  | GetService                 |                                |
                  |                            |                                |


Client
======

Currently available clients:

- php

  https://github.com/mikespook/php_skynet

- ruby

  https://github.com/skynetservices/ruby_skynet

Clients contain a pool of connections to a given service, up to a specified size to load balance requests across.
