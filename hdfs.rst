==============

==============

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


HDFS write

::

    client              nameNode        dataNode1       dataNode2       dataNodeN
      |                     |               |               |               |
      | create              |               |               |               |
      |-------------------->|               |               |               |
      |                     |               |               |               |
      | inode, lease        |               |               |               |
      |<--------------------|               |               |               |
      |                     |               |               |               |
      | apply for block     |               |               |               |
      |-------------------->|               |               |               |
      |                     |               |               |               |
      | [{block: [node]}]   |               |               |               |
      |<--------------------|               |               |               |
      |                                     |               |               |
      | req{write block data}               |               |               |
      |------------------------------------>|               |               |
      | [node1, node2, ..., nodeN]          | block         |               |
      |                                     |-------------->|               |
      |                                     |[node2, nodeN] |               |
      |                                     |               | block         |
      |                                     |               |-------------->|
      |                                     |               | [nodeN]       |
      |                                     |               |               |
      |                                     |               | ack           |
      |                                     | ack           |<--------------|
      | ack                                 |<--------------|               |
      |<------------------------------------|               |               |
      |                                     |               |               |

