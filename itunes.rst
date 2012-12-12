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
        signature, certs, bag

    POST https://su.itunes.apple.com/WebObjects/MZSoftwareUpdate.woa/wa/availableSoftwareUpdates?l=zh&guid=C82A1430E527
        plist of local softwares

    POST http://EVIntl-ocsp.verisign.com/
        ocsp-request, ocsp-response

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


安装某个app
================

点击按钮后
--------------

::

    按1下：

    OPTIONS https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?eventType=ITSOmniture&version=2.1

    GET  https://securemetrics.apple.com/b/ss/applesuperglobal/1/H.20.3/s91445143322926?pageName=Genre-CN-Desktop%20Applicati39&bw=1000&ce=UTF-8&ndh=1&g=https%3A%2F%2Fitunes.apple.com%2FWebObjects%2FMZStore.woa%2Fwa%2FviewGrouping%3Fcc%3Dcn%26id%3D29562%26guid%3DC82A1430E527%26mt%3D12&guid=C82A1430E527&pe=lnk_o&c=24&k=Y&cl=15778463&s=1280x800&t=12%2F11%2F2012%2011%3A30%3A32%203%20-480&AQB=1&pev2=Genre-CN-Desktop%20Applications-39%7CGrid_%E6%96%B0%E5%93%81%E6%8E%A8%E8%8D%90%7CLockup_8%7CBuy&bh=699&sfcustom=1&AQE=1&v=Y&h5=appleitmsxxap%2Cappleitmscnap

    POST https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?version=2.1&eventType=ITSOmniture&guid=C82A1430E527

    再按1下：

    GET  http://a780.phobos.apple.com/us/r1000/091/Purple/v4/13/fc/b0/13fcb031-12ae-ea12-4903-190bb582c807/signed.dcr.3632305418074835009.pfpkg
        os版本等合法性检查

    与1 一样，记录统计信息

    GET http://ax.init.itunes.apple.com/bag.xml?ix=5 

    弹出apple id登录框

    GET https://p50-buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/authenticate?password=XXXXXXXX&rmp=0&why=purchase&attempt=1&appleId=YYYYYYYY%40163.com&guid=C82A1430E527


下载
---------

::

    POST https://p50-buy.itunes.apple.com/WebObjects/MZBuy.woa/wa/buyProduct?guid=C82A1430E527
        req form: productType=C&pricingParameters=STDQ&salableAdamId=568494494&price=0&appExtVrsId=11750382&origPage=Genre-CN-Desktop%20Applications-39&origPageCh=Desktop%20Apps-main&origPageLocation=Grid_%E6%97%B6%E4%B8%8B%E7%83%AD%E9%97%A8%7CLockup_1%7CBuy&creditDisplay=%C2%A570.00&guid=C82A1430E527&macappinstalledconfirmed=1
        服务器端分配download-url, download-id, cacel_download_id, downloadKey(下周.pkg时用到，通过cookie传递)

    GET  http://a415.phobos.apple.com/us/r1000/067/Purple/v4/6b/27/41/6b27415a-540a-8683-bd19-dd4a0d1447d6/mzps2618483160685073283.pkg
        Cookie: downloadKey=expires=1355478385~access=/us/r1000/067/Purple/v4/6b/27/41/6b27415a-540a-8683-bd19-dd4a0d1447d6/mzps2618483160685073283.pkg*~md5=192798112c15601bbf461efa8f98bf0f
        这就是那个要下载的app包
    GET  https://se.itunes.apple.com/WebObjects/MZStoreElements.woa/wa/personalizedAccountInfoFragment?guid=C82A1430E527&cc=cn
        再取一遍账户余额，返回的JSON例如：{"welcomeMessage":"欢迎您，名", "creditBalance":"¥70.00"}

    GET https://p50-buy.itunes.apple.com/WebObjects/MZFastFinance.woa/wa/songDownloadDone?songId=568494494&download-id=500001662224635&Pod=50&guid=C82A1430E527
        下载完成后，向服务器汇报


搜索
==========

::

    GET http://ax.search.itunes.apple.com/WebObjects/MZSearchHints.woa/wa/hints?q=eve
    GET http://ax.search.itunes.apple.com/WebObjects/MZSearchHints.woa/wa/hints?q=evernote

    GET http://ax.search.itunes.apple.com/WebObjects/MZSearch.woa/wa/search?q=evernote


进入某个app明细页
========================

::


    GET https://itunes.apple.com/cn/app/yin-xiang-bi-ji/id406056744?mt=12&guid=C82A1430E527

    GET https://itunes.apple.com/customer-reviews/id406056744?displayable-kind=30&l=zh&guid=C82A1430E527

    GET http://myapp.itunes.apple.com/WebObjects/MZAppPersonalizer.woa/wa/customersAlsoBoughtFragment?l=zh&sd=406056744&guid=C82A1430E527

    POST https://se.itunes.apple.com/WebObjects/MZStoreElements.woa/wa/buyButtonMetaData?guid=C82A1430E527&cc=cn

    POST https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?version=2.1&eventType=ITSLoadTimes&guid=C82A1430E527

    POST https://metrics.mzstatic.com/WebObjects/MZUserXP.woa/wa/recordStats?version=2.1&eventType=ITSOmniture&guid=C82A1430E527

    GET https://securemetrics.apple.com/b/ss/applesuperglobal/1/H.20.3/s05786518459208?v12=MacAppStore%2F1.1.2%20(Macintosh%3B%20U%3B%20Intel%20Mac%20OS%20X%2010.7.5%3B%20zh-Hans)%20AppleWebKit%2F534.57.7&ch=Software%20Pages&c=24&AQB=1&guid=C82A1430E527&r=http%3A%2F%2Fax.search.itunes.apple.com%2FWebObjects%2FMZSearch.woa%2Fwa%2Fsearch%3Fq%3Devernote&s=1280x800&t=12%2F11%2F2012%2016%3A19%3A14%203%20-480&c12=MacAppStore%2F1.1.2%20(Macintosh%3B%20U%3B%20Intel%20Mac%20OS%20X%2010.7.5%3B%20zh-Hans)%20AppleWebKit%2F534.57.7&g=https%3A%2F%2Fitunes.apple.com%2Fcn%2Fapp%2Fyin-xiang-bi-ji%2Fid406056744%3Fmt%3D12%26guid%3DC82A1430E527&v22=HTML&products=Evernote-Evernote-406056744&v=Y&h5=appleitmsxxap%2Cappleitmscnap&ndh=1&pageName=Software-CN-Evernote-Evernote-406056744&bw=1000&cl=15778463&ce=UTF-8&k=Y&bh=699&AQE=1&c22=HTML&sfcustom=1



提交review
=================

::

    
    POST https://userpub.itunes.apple.com/WebObjects/MZUserPublishing.woa/wa/userRateContent?rating=4&id=406056744&displayable-kind=30&guid=C82A1430E527


    对别人的评价，觉得有帮助吗？
    POST https://userpub.itunes.apple.com/WebObjects/MZUserPublishing.woa/wa/rateUserReview?userReviewId=691686393&guid=C82A1430E527
        form: vote=1


