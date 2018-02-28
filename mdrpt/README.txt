
¡ï ÏûÏ¢ÈİÔÖ»·¾³ËµÃ 2012.11.13

1) API bug-fix: ¾É°æ±¾±¸ÏµÍ³mdr_CmtResult()ÖĞÓĞbug£¬ÓèÒÔĞŞ¸´¡£

2) Ìá¹©IBM-AIX¡¢HP-UXµÄ¿â: ×öÁËHP-UX´úÂëµ½IBM-AIXµÄÒÆÖ²£¬Í³Ò»ÁË´úÂë¡£
   ¿â°æ±¾±¸ÍüÈçÏÂ£º
   $ what libmdrapi_node.ibm.so 
   libmdrapi_node.ibm.so:
       compiled at: [Nov 13 2012], [17:14:29] libmdrapi_node
   $ cksum libmdrapi_node.ibm.so 
   3421497478 5797275 libmdrapi_node.so
   $ what libmdrapi_node.hp.so 
   libmdrapi_node.hp.so:
       compiled at: [Nov 13 2012], [17:53:50] libmdrapi_node
   $ cksum libmdrapi_node.hp.so 
   3305416321 4437888 libmdrapi_node.hp.so

3) Ìá¹©ÁËÒ»¸öÀı×Ó³ÌĞò£ºut-MdrNodeApi.cpp£¬¹©¿ª·¢²Î¿¼¡£

4) Îª±ãÓÚĞ´ÈÕÖ¾¼°debug£¬MdrNodeApi.hÖĞĞÂÔöMdrAuditInfo::toStr(), MdrVarInfo::toStr()·½·¨¡£

5) ÆäËû£ºJava°æ±¾µÄmdrmgrÎŞ±ä»¯¡£


¡ï ÏûÏ¢ÈİÔÖ»·¾³ËµÃ 2012.11.07

0) È·ÈÏ
Ö÷»úÒÑ°²×°jdk 1.6

1) »·¾³±äÁ¿¾ÙÀı
export MDR_HOME=/BEA/tuxedo/DEV/MDRPT
export MDR_CONFIG=mdr_service_cfg.xml

export LIBPATH=$LIBPATH:$MDR_HOME/lib64		## IBM
LD_LIBRARY_PATH¡¢SHLIB_PATHÒ²ÊÇÕâ¸öÖµ		## HP-UX

* Ö÷ÏµÍ³
export MDR_SERVERID=TPSS1

* ±¸ÏµÍ³
export MDR_SERVERID=TPSS2

2) ÅäÖÃÎÄ¼ş
ÇëĞŞ¸Ä$MDR_HOME/$MDR_SERVERID/etc/mdrmgr_config.xml£¬ÆäÖĞµÄunixuserÅäÖÃ¸ÄÎªÊµ¼ÊÖµ¡£
ÆäËûÅäÖÃÏîÒ»°ã²»ÓÃ¸Ä¡£

3) Ö´ĞĞÆô¶¯½Å±¾
Ö´ĞĞ$MDR_HOME/$MDR_SERVERID/bin/runMdrMgr.sh start£¬¹Û²ì$MDR_HOME/$MDR_SERVERID/var/logÄ¿Â¼ÏÂµÄÈÕÖ¾ÄÚÈİ¡£

4) »ıÑ¹µÄÖÙ²ÃĞÅÏ¢ÇåÀí
Ä³Ğ©Çé¿öÏÂ£¬ĞèÒªÇåÀí»·¾³£¬¿ÉÒÔÉ¾³ı$MDR_HOME/$MDR_SERVERID/data/nodmqÄ¿Â¼ÏÂµÄ»º´æÎÄ¼ş£¬ÔÙÖØÆôÓ¦ÓÃ¡£

