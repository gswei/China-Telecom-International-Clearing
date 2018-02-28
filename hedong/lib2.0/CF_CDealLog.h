/***********************************************************************
*CF_CDealLog.h
*通用处理日志接口
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
  
  /*初始化函数
  * 所有按pipe 启动的程序使用本函数做初始化
  * const char *szPipeId  [in]  按流水线启动的程序此参数填PIPE_ID
  * int iProcessId  [in]  按流水线启动的程序此参数填PROCESS_ID
  * const char *szEnvFile [in]  公共环境变量文件
  */
  bool Init_Pipe(const char *szPipeId, int iProcessId, const char *szEnvFile);
  
  /*初始化函数
  * 所有按SOURCE启动的程序使用本函数做初始化
  * const char *szSourceId  [in]  数据源标识
  * const char *szEnvFile   [in]  公共环境变量文件
  */
  bool Init_Source(const char *szSourceId, const char *szEnvFile);
  
  /*初始化函数
  * 既不按PIPE启动，又不按SOURCE启动的程序使用本函数做初始化（目前只有传输程序）
  * const char *szEnvFile  [in] 公共环境变量文件
  */
  bool Init_None(const char *szEnvFile);
  
  /*数据采集程序使用的写处理日志函数
  * 参数表：标准文件名[in],原始文件名[in],采集开始时间[in],采集结束时间[in],
  *         该文件生成时间[in],该文件序列号[in]
  */
  bool DealLog_Collect(const char *szFileName, const char *szOrgFileName,
                       const char *szStartTime, const char *szEndTime,
                       const char *szSwitchTime, int iSN);
  bool DealLog_Recollect(const char *szOrgFileName, const char *szBFileName, const char *szAFileName,
                         const char *szReferTime, const char *szCompleteTime, const char cDealFlag);
                         
  /*数据传输程序使用的写处理日志函数
  * 参数表：标准文件名[in],传输开始时间[in],传输结束时间[in],文件大小[in],源机器[in],
            源路径[in],目标机器[in],目标路径[in]
  */
  bool DealLog_Trans(const char *szFileName, const char *szStartTime,
                     const char *szEndTime, int iFileSize, const char *szOrgMachine,
                     const char *szOrgPath, const char *szDestMachine, const char *szDestPath);
  
  /*数据接收程序使用的写处理日志函数
  * 参数表：标准文件名[in],接收时间[in],文件大小[in],原始文件名[in]
  */
  bool DealLog_Recv(const char *szFileName, const char *szRecvTime, int iFileSize, const char *szOrgFileName);
  
  /*预处理阶段程序（格式化，去重，超频话单处理，分拣）使用的写处理日志函数
  * 参数表：标准文件名[in],本地网[in],数据源标识[in],处理时间[in],文件总的话单数[in],
            正确话单数[in],输出话单数[in],文件中最早通话时间[in],文件中最晚通话时间[in]
  */
  bool DealLog_Prep(const char *szFileName, const char *szLocalNet,
                    const char *szSourceId, const char *szStartTime,
                    const char *szDealTime, int iTotalCdr,
                    int iNormalCdr,int iPickCdr, 
                    int iOutCdr, const char *szStartCdrTime,
                    const char *szEndCdrTime);
  
  /*计费阶段程序（路由分析，号码分析，批价，统计，清单生成）使用的写处理日志函数
  * 参数表：标准文件名[in],处理开始时间[in],处理结束时间[in],总话单数[in],无资料话单数[in],输出话单数[in]
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
  bool m_bLogFlag;           //直接写日志（true），还是建目录层次（false）
  ofstream OutStream;
  
  bool WriteLog(const char *szLogFileName, const char *szLog, const char *szDealTime);
  
  bool GetLogPath(const char *szEnvFileName, char *szLog, bool &bLogFlag);
  
  
  
};




#endif 


