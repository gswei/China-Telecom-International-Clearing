/***********************************************************************
*CF_CDealLog.h
*ͨ�ô�����־�ӿ�
*20050921 by tanj
**********************************************************************/

#ifndef _CF_CDEALLOG_H_
#define _CF_CDEALLOG_H_


#include "fstream.h"
#include "Common.h"
#include "CF_CError.h"
const int DEALLOG_COLLECT_CODE = 1;
const int DEALLOG_RECOLLECT_CODE = 2;
const int DEALLOG_TRANS_CODE = 3;
const int DEALLOG_RECV_CODE = 4;
const int DEALLOG_MAIN_CODE = 5;
const char DEALLOG_PATH_NAME[30] = "DEALLOG_DIR";
const char DEALLOG_MODE_NAME[30] = "DEALLOG_MODE";

const char DEALLOG_SUFFIX[10] = "dlog";
const char DEALLOG_SEPERATOR = ',';
class CF_CDealLog
{
public:
  CF_CDealLog();
  
  /*��ʼ������
  * ���а�pipe �����ĳ���ʹ�ñ���������ʼ��
  * const char *szPipeId  [in]  ����ˮ�������ĳ���˲�����PIPE_ID
  * int iProcessId  [in]  ����ˮ�������ĳ���˲�����PROCESS_ID
  * const char *szEnvFile [in]  �������������ļ�
  */
  bool Init_Pipe(const char *szPipeId, int iProcessId, const char *szEnvFile);
  
  /*��ʼ������
  * ���а�SOURCE�����ĳ���ʹ�ñ���������ʼ��
  * const char *szSourceId  [in]  ����Դ��ʶ
  * const char *szEnvFile   [in]  �������������ļ�
  */
  bool Init_Source(const char *szSourceId, const char *szEnvFile);
  
  /*��ʼ������
  * �Ȳ���PIPE�������ֲ���SOURCE�����ĳ���ʹ�ñ���������ʼ����Ŀǰֻ�д������
  * const char *szEnvFile  [in] �������������ļ�
  */
  bool Init_None(const char *szEnvFile);
  
  /*���ݲɼ�����ʹ�õ�д������־����
  * ��������׼�ļ���[in],ԭʼ�ļ���[in],�ɼ���ʼʱ��[in],�ɼ�����ʱ��[in],
  *         ���ļ�����ʱ��[in],���ļ����к�[in]
  */
  bool DealLog_Collect(const char *szFileName, const char *szOrgFileName,
                       const char *szStartTime, const char *szEndTime,
                       const char *szSwitchTime, int iSN);
  bool DealLog_Recollect(const char *szOrgFileName, const char *szBFileName, const char *szAFileName,
                         const char *szReferTime, const char *szCompleteTime, const char cDealFlag);
                         
  /*���ݴ������ʹ�õ�д������־����
  * ��������׼�ļ���[in],���俪ʼʱ��[in],�������ʱ��[in],�ļ���С[in],Դ����[in],
            Դ·��[in],Ŀ�����[in],Ŀ��·��[in]
  */
  bool DealLog_Trans(const char *szFileName, const char *szStartTime,
                     const char *szEndTime, int iFileSize, const char *szOrgMachine,
                     const char *szOrgPath, const char *szDestMachine, const char *szDestPath);
  
  /*���ݽ��ճ���ʹ�õ�д������־����
  * ��������׼�ļ���[in],����ʱ��[in],�ļ���С[in],ԭʼ�ļ���[in]
  */
  bool DealLog_Recv(const char *szFileName, const char *szRecvTime, int iFileSize, const char *szOrgFileName);
  
  /*Ԥ����׶γ��򣨸�ʽ����ȥ�أ���Ƶ���������ּ�ʹ�õ�д������־����
  * ��������׼�ļ���[in],������[in],����Դ��ʶ[in],����ʱ��[in],�ļ��ܵĻ�����[in],
            ��ȷ������[in],���������[in],�ļ�������ͨ��ʱ��[in],�ļ�������ͨ��ʱ��[in]
  */
  bool DealLog_Prep(const char *szFileName, const char *szLocalNet,
                    const char *szSourceId, const char *szStartTime,
                    const char *szDealTime, int iTotalCdr,
                    int iNormalCdr,int iPickCdr, 
                    int iOutCdr, const char *szStartCdrTime,
                    const char *szEndCdrTime);
  
  /*�Ʒѽ׶γ���·�ɷ�����������������ۣ�ͳ�ƣ��嵥���ɣ�ʹ�õ�д������־����
  * ��������׼�ļ���[in],����ʼʱ��[in],�������ʱ��[in],�ܻ�����[in],�����ϻ�����[in],���������[in]
  */
  bool DealLog_Billing(const char *szFileName, const char *szLocalNet,
                       const char *szSourceId, const char *szStartTime, 
                       const char *szEndTime,  int iTotalCdr,
                       int iNormalCdr, int iPickCdr,
                       int iLackInfoCdr,int iOutCdr);
  
private:
  char m_szPipeId[11];
  char m_szProcessId[6];
  char m_szLogPath[256];
  bool m_bLogFlag;           //ֱ��д��־��true�������ǽ�Ŀ¼��Σ�false��
  ofstream OutStream;
  
  bool WriteLog(const char *szLogFileName, const char *szLog, const char *szDealTime);
  
  bool GetLogPath(const char *szEnvFileName, char *szLog, bool &bLogFlag);
  
  
  
};




#endif 


