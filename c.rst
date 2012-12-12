=========================
C Language
=========================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 21 century C
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


Environment
===========

valgrind, gprof, pkg-config

::

    pkg-config --cflags --libs sqlite3
    pkg-config --libs libxml-2.0 sqlite3


makefile
---------

::

    make -p # default rules and variables


EnvironmentVariables
^^^^^^^^^^^^^^^^^^^^

- CFLAGS

- LDLIBS

- CC

BuiltinVariables
^^^^^^^^^^^^^^^^

- $@

  The full target filename. 
  
  By target, I mean the file that needs to be built, such as a .o file being compiled from a .c file or a program made by linking .o files.

- $*

  The target file with the suffix cut off. 
  
  So if the target is prog.o, $* is prog, and $*.c would become prog.c.

- $<

  The name of the file that caused this target to get triggered and made. 
  
  If we are making prog.o, it is probably because prog.c has recently been modified, so $< is prog.c.


gdb
---

breakpoint
^^^^^^^^^^

- info break

- disable 3

  disable

  enable 3

- delete 3

backtrace
^^^^^^^^^

- f 2

  frame 2

  bt


gprof
-----

::

    gcc -g -pg a.c
    ./a.out # will generate gmon.out
    gprof a.out

    valgrind ./a.out # memory checker, it run on its own vm instead of os


autotools
---------

::

    create Makefile.am  # automake

    autoscan

    sed -e '10i\
    AM_INIT_AUTOMAKE' \
    < configure.scan > configure.ac # generate configure.ac

    touch NEWS README AUTHORS ChangeLog # GNU coding standard

    autoreconf -iv
