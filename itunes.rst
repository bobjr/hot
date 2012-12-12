===============================
iTunes network traffic analysis
===============================

:Author: Gao Peng <funky.gao@gmail.com>
:Description: 
:Revision: $Id$

.. contents:: Table Of Contents
.. section-numbering::


精选
===========

::

    GET  http://ax.init.itunes.apple.com/bag.xml?ix=5

    POST https://su.itunes.apple.com/WebObjects/MZSoftwareUpdate.woa/wa/availableSoftwareUpdates?l=zh&guid=C82A1430E527
        plist of local softwares

    POST http://EVIntl-ocsp.verisign.com/
        ocsp-request
        ocsp-response

    GET  https://itunes.apple.com/WebObjects/MZStore.woa/wa/storeFront?guid=C82A1430E527
        goto https://itunes.apple.com/WebObjects/MZStore.woa/wa/viewGrouping?cc=cn&guid=C82A1430E527&id=29562&mt=12

    GET  https://itunes.apple.com/WebObjects/MZStore.woa/wa/viewGrouping?cc=cn&id=29562&guid=C82A1430E527&mt=12

    GET  https://se.itunes.apple.com/WebObjects/MZStoreElements.woa/wa/personalizedAccountInfoFragment?guid=C82A1430E527&cc=cn
        {"welcomeMessage":"欢迎您，名", "creditBalance":"¥70.00"}

    POST https://se.itunes.apple.com/WebObjects/MZStoreElements.woa/wa/buyButtonMetaData?guid=C82A1430E527&cc=cn
        ids: 549294099:mac-software,580090604:mac-software,573317542:mac-software
        version: 2

    OPTIONS https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?eventType=ITSOmniture&version=2.1

    OPTIONS https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?eventType=ITSLoadTimes&version=2.1

    POST https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?version=2.1&eventType=ITSOmniture&guid=C82A1430E527

    POST https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?version=2.1&eventType=ITSLoadTimes&guid=C82A1430E52

    GET  https://help.apple.com/numbers/mac/2.3/help/zh_CN.lproj/search.helpindex

    GET  http://www.apple.com.cn/hotnews/rss/hotnews.rss
