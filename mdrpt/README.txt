
�� ��Ϣ���ֻ���˵� 2012.11.13

1) API bug-fix: �ɰ汾��ϵͳmdr_CmtResult()����bug�������޸���

2) �ṩIBM-AIX��HP-UX�Ŀ�: ����HP-UX���뵽IBM-AIX����ֲ��ͳһ�˴��롣
   ��汾�������£�
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

3) �ṩ��һ�����ӳ���ut-MdrNodeApi.cpp���������ο���

4) Ϊ����д��־��debug��MdrNodeApi.h������MdrAuditInfo::toStr(), MdrVarInfo::toStr()������

5) ������Java�汾��mdrmgr�ޱ仯��


�� ��Ϣ���ֻ���˵� 2012.11.07

0) ȷ��
�����Ѱ�װjdk 1.6

1) ������������
export MDR_HOME=/BEA/tuxedo/DEV/MDRPT
export MDR_CONFIG=mdr_service_cfg.xml

export LIBPATH=$LIBPATH:$MDR_HOME/lib64		## IBM
LD_LIBRARY_PATH��SHLIB_PATHҲ�����ֵ		## HP-UX

* ��ϵͳ
export MDR_SERVERID=TPSS1

* ��ϵͳ
export MDR_SERVERID=TPSS2

2) �����ļ�
���޸�$MDR_HOME/$MDR_SERVERID/etc/mdrmgr_config.xml�����е�unixuser���ø�Ϊʵ��ֵ��
����������һ�㲻�øġ�

3) ִ�������ű�
ִ��$MDR_HOME/$MDR_SERVERID/bin/runMdrMgr.sh start���۲�$MDR_HOME/$MDR_SERVERID/var/logĿ¼�µ���־���ݡ�

4) ��ѹ���ٲ���Ϣ����
ĳЩ����£���Ҫ������������ɾ��$MDR_HOME/$MDR_SERVERID/data/nodmqĿ¼�µĻ����ļ���������Ӧ�á�

