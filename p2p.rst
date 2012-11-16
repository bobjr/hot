=========================
Peer to peer
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: P2P explained
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


BitTorrent
==========

intro
-----
Files is broken into chunks, typically 256KB. Upload chunk while downloading chunks.

file.torrent
------------

::

    length
    name
    hash
    url of tracker


Pastry
======

Each node knows its predecessor and successor, called its leaf set. Range is from its predecessor to successor.

To find owner(i), node n des the following:

从leafset开始找，没找到，则
