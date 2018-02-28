/****************************************************************
filename: dealfile.cpp
module:
created by: ouyh
create date: 
version: 3.0.0
description:
    话单文件处理函数
*****************************************************************/

#include "DealFile.h"
#include "main/pluginengine/performanceanalyzer.h"
#include <sys/time.h>

extern CDatabase DBConn;

using namespace filterchain;

CDealFile::CDealFile()
{
	vecFile.clear();
	//vecAlter.clear();
	vecRec.clear();
}

CDealFile::~CDealFile()
{
}

int CDealFile::dealfile(struct SParameter &Param, struct SFileStruct &FileStruct, PacketParser* pps, ResParser* res)
{
	/*
	srand((int)time(NULL));
	int iRadFlag=rand()%100;
	cout<<"iRadFlag="<<iRadFlag<<endl;
	if(iRadFlag>30)
		return -2;
	*/

	unsigned long timePart1 = 0, timePart2 = 0, timePart3 = 0, timePart4 = 0, timePart5 = 0;

	/*struct timeval filetime, start, finish;
	timerclear(&start);
	timerclear(&finish);
	timerclear(&filetime);*/
	/*	long timeMicros;
	gettimeofday(&start, NULL);
	*/

	/*定义变量*/
	char szLogStr[LOG_MSG_LEN+1];
	char szLongStr[MAX_LINE_LENGTH+1];

	/* 初始化无资料接口 */
	char CurrentTime[DATETIME_LEN+1];
	getCurTime(CurrentTime);
	Param.classify.initFile(Param.szServiceId, FileStruct.szSourceId, FileStruct.szSourcePath, 
		FileStruct.szRealFileName, Param.szOutputFiletypeId, CurrentTime);
	
	char szFmtTimeOutFile[FILE_NAME_LEN+1];
	char szFmtTimeOutTmp[FILE_NAME_LEN+1];
	FILE *fpFmtTimeOut = NULL;
	char szFmtOtherFile[FILE_NAME_LEN+1];
	char szFmtOtherTmp[FILE_NAME_LEN+1];
	FILE *fpFmtOther = NULL;
	char szFmtErrFile[FILE_NAME_LEN+1];
	char szFmtErrTmp[FILE_NAME_LEN+1];
	int iFmtTimeOutNum = 0;
	int iFmtOtherNum = 0;
	int iFmtErrorNum = 0;
	/* 初始化格式化错单 */
	if( !strcmp(Param.szIsFmtFirst, "Y") )
	{
		//sprintf(Param.szFmtErrDir, "%s%s", FileStruct.szSourcePath, Param.szError_Dir);
		sprintf(szFmtTimeOutFile, "%s%s%s", FileStruct.szSourcePath, Param.szFmtErrDir, Param.szFmtTimeOutDir);
		if( chkAllDir( szFmtTimeOutFile )!=0 )
		{
			sprintf(szLogStr, "无法创建路径： %s ！", szFmtTimeOutFile);
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		sprintf(szFmtTimeOutFile, "%s%s", szFmtTimeOutFile, FileStruct.szFileName);
		sprintf(szFmtTimeOutTmp, "%s.tmp", szFmtTimeOutFile);
		if((fpFmtTimeOut=fopen(szFmtTimeOutTmp,"w"))==NULL)
		{
			sprintf(szLogStr, "打开文件： %s 出错！", szFmtTimeOutTmp);
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		sprintf(szFmtOtherFile, "%s%s%s", FileStruct.szSourcePath, Param.szFmtErrDir, Param.szFmtOtherDir);
		if( chkAllDir( szFmtOtherFile )!=0 )
		{
			sprintf(szLogStr, "无法创建路径： %s ！", szFmtOtherFile);
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		sprintf(szFmtOtherFile, "%s%s", szFmtOtherFile, FileStruct.szFileName);
		sprintf(szFmtOtherTmp, "%s.tmp", szFmtOtherFile);
		if((fpFmtOther=fopen(szFmtOtherTmp,"w"))==NULL)
		{
			fclose(fpFmtTimeOut);
			sprintf(szLogStr, "打开文件： %s 出错！", szFmtOtherTmp);
			theLog<<szLogStr<<endd;
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		if( !strcmp(Param.szFmtErr2File, "Y") )
		{
			sprintf(szFmtErrFile, "%s%s%s", FileStruct.szSourcePath, Param.szFmtErrDir, FileStruct.szRealFileName);
			sprintf(szFmtErrTmp, "%s.tmp", szFmtErrFile);
			Param.fmt_err2file.Init(FileStruct.szSourceFiletype);
			Param.fmt_err2file.Open(szFmtErrTmp);
		}
	}

	/* 初始化和打开输入文件 */
	char InFilePath[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char InFileName[FILE_NAME_LEN+1];
	CF_MemFileI _infile;
	sprintf(InFilePath, "%s%s%s", FileStruct.szSourcePath, Param.szInPath, FileStruct.szFileName);
	sprintf(InFileName, "%s", FileStruct.szFileName);
	CFmt_Change _inrcd;
	//cout<<"初始化输入文件"<<Param.szOutputFiletypeId<<endl;
	if( strcmp(Param.szIsFmtFirst,"Y")==0 )
		_infile.Init(FileStruct.szSourceFiletype);
	else
		_infile.Init(Param.szOutputFiletypeId);
	//cout<<"初始化输入话单格式"<<Param.szOutputFiletypeId<<"\t"<<Param.szOutrcdType<<endl;
	_inrcd.Init(Param.szOutputFiletypeId, Param.szOutrcdType[0]);
	_infile.Open(InFilePath);
	
	long lFileNum = _infile.get_num();
	Param.ProcMonitor.UpdateMonitor(FileStruct.szFileName, lFileNum, 'Y', CProcInfo::SERV_BUSY);
	
	/*初始化和打开输出文件*/
	char OutTmpFilePath[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char OutRealFilePath[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char OutRealFileName[FILE_NAME_LEN+1];
	CF_MemFileO _outfile;
	if(Param.bOutputFile)
	{
		strcpy(OutRealFileName, FileStruct.szRealFileName); 
		//cout<<"OutRealFileName="<<OutRealFileName<<endl;
		sprintf(OutTmpFilePath, "%s%stmp_out/~%s.%d", FileStruct.szSourcePath, Param.szInPath, OutRealFileName, Param.iProcessId);
		sprintf(OutRealFilePath, "%s%s%s", FileStruct.szSourcePath, Param.szOutPath, OutRealFileName);
		//cout<<"初始化输出文件"<<Param.szOutputFiletypeId<<endl;
		_outfile.Init( Param.szOutputFiletypeId );
		_outfile.Open(OutTmpFilePath);
	}

	
	CDealedFileInfo fname;
	strcpy(fname.szOutputFiletypeId, Param.szOutputFiletypeId);
	fname.szOutrcdType = Param.szOutrcdType[0];
//	CFmt_Change _outrcd;
	//cout<<"初始化输出话单格式"<<Param.szOutputFiletypeId<<"\t"<<Param.szOutrcdType<<endl;
//	_outrcd.Init(Param.szOutputFiletypeId, Param.szOutrcdType[0]);
	vecRec.clear();
	//SRec tmpRec;
	string strRec;
	char tmpRec[RECLEN+1];
	memset(tmpRec, 0, sizeof(tmpRec));
  
	int iTotalNum = 0, iLackInfoType, iBcount, iOcount;
	pluginAnaResult Result;
	int classifyType;
	char RecordNo[FIELD_LEN];
	
	char szBuff[MAX_LINE_LENGTH+1];
	memset(szBuff, 0, sizeof(szBuff));
	char szServCat[SERVER_LEN+1];
	memset(szServCat, 0, sizeof(szServCat));
	
	FilterChain *chain = Param.pluginInitializer.getFilterChain();
	if(chain == NULL)
	{
		sprintf(szLogStr, "找不到插件！");
		theLog<<szLogStr<<ende;
		return -1;
	}
	ArgRecord rcd;
	Argument *pRcd = &rcd;
	ActionResult *ret;

	dealRcdNum.clear();

	/*
	gettimeofday(&finish, NULL);
	timeMicros = (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
	theLog<<"初始化文件使用时间："<<timeMicros<<endi;
	gettimeofday(&start, NULL);
	*/

	pps->clearAll();
	res->clearAll();
	/* 加默认的临时变量 */
	for(TMP_VAR::iterator it = Param.map_DefVar.begin(); it != Param.map_DefVar.end(); it++)
	{
		pps->addVariable(it->szVarName, it->szVarValue);
		res->addVariable(it->szVarName, it->szVarValue);
	}
	
	//gettimeofday(&filetime, NULL);
	
	/* 重复读取所有的话单并对其进行处理 */ 
	theLog<<"File in processing..."<<endd;

	try
	{
		while(1)
		{
			//gettimeofday(&start, NULL);

			// Init record
			try {
				pps->clear();
				res->clear();

				Result = eNormal;
				classifyType = 0;

				/* 写文件偏移量 */
				int iPos = _infile.GetPos();
				pps->setOffset(iPos);

				if( strcmp(Param.szIsFmtFirst,"Y")==0 )
				{
					memset(szBuff, 0, sizeof(szBuff));
					if( _infile.readRec(szBuff, MAX_LINE_LENGTH) != READ_AT_END )
					{
						cout<<"it is format first"<<endl;
						/* 第一个插件为格式化时 */
						if(strlen(szBuff)==0)
							break;
						pps->setRecord(szBuff);
						//theLog<<"输入第"<<iTotalNum+1<<"行："<<pps->getString()<<endd;
					}
					else
					{
						if(iTotalNum == 0)
							dealRcdNum.set(FileStruct.szServCat, 0, 0, 0, 0, 0, 0);
						break;
					}
				}
				else if( _infile.readRec(_inrcd) == READ_AT_END )
				{
					if(iTotalNum == 0)
						dealRcdNum.set(FileStruct.szServCat, 0, 0, 0, 0, 0, 0);
					break;
				}
				iTotalNum++;
				strcpy(RecordNo,_infile.getRecordNo());
				pps->setRcd(_inrcd);
				//res->setRcd(_outrcd);
				res->setRcd(_inrcd);

				pRcd->set(*pps, *res);
			} catch (CException &e) {
				e.PushStack(e.GetAppError(), "initialize record error", __FILE__,
						__LINE__);
				throw e;
			} catch (const std::exception &e) {
				throw CException(0, e.what(), __FILE__, __LINE__);
			} catch (...) {
				throw CException(0, "unknown error", __FILE__, __LINE__);
			}
	
			//gettimeofday(&finish, NULL);
//			theLog<<"初始化话单信息用时："<<timeMicros<<endi;
			//timePart1 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);;
			//gettimeofday(&start, NULL);

			try{
			/* 插件分析 */
			while( chain->hasNextAction() )
			{
				//gettimeofday(&finish, NULL);
				//timePart3 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);

				//gettimeofday(&start, NULL);
				ret = chain->runNextAction(pRcd);
				//gettimeofday(&finish, NULL);
				//timePart2 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);;
				//gettimeofday(&start, NULL);

//				if(ret->getUsedTimeMicros() > MAX_TIME_IN_PLUGIN)
//				{
//					theLog<<"LINE："<<iTotalNum<<"<<当前执行的插件/表达式ID："<<ret->getRelatedActionId()<<"   类型：" <<ret->getRelatedActionType()<<"   用时： "<<ret->getUsedTimeMicros()<<endd;
//				}
				theLog<<"LINE："<<iTotalNum<<"<<当前执行的插件/表达式ID："<<ret->getRelatedActionId()<<"   类型：" <<ret->getRelatedActionType()<<endi;
				if( ret->getRelatedActionType() != "PLUGIN-EXECUTOR" )
					continue;
				Result = pRcd->getAnaType();
				cout<<"Result = "<<Result<<endl;;
				theLog<<res->m_outRcd.Get_record()<<endi;
				if( Result == eNormal)
				{
					pps->copyRes(*res);
					cout<<"Normal record"<<endl;
					//cout<<res->m_outRcd.Get_record()<<"\n"<<pps->getRecord()<<endl;
					//cout<<res->m_outRcd.Get_record()<<endl;
					continue;
				}
				else if( Result == eClassifiable)
				{
					classifyType = Param.classify.dealRecord(res->getRuleType(), pps->m_inRcd);
					if(classifyType == 1)
					{
						//互斥分拣话单
						break;
					}
					else if(classifyType == 3)
					{
						//备份分拣话单
						res->resetAnaResult();
						Result = eNormal;
						/* 记录处理数 */
						memset(szServCat, 0, sizeof(szServCat));
						(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
						if(strlen(szServCat)==0)
							strcpy(szServCat, FileStruct.szServCat);
						dealRcdNum.set(szServCat, 0, 0, 0, 0, 0, 1);
						dealRcdNum.setFee(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Fee());
						dealRcdNum.setTime(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Time());
						//continue;
					}
					else
					{
						// 分拣出错
						//theLog<<"分拣处理出错！"<<endw;
						if( !strcmp(Param.szIsFmtFirst, "Y") )
						{
							fclose(fpFmtTimeOut);
							fclose(fpFmtOther);
							if( !strcmp(Param.szFmtErr2File, "Y") )
								Param.fmt_err2file.Close();
						}
						//return -1;
						throw CException(errno, "分拣处理出错！", __FILE__, __LINE__);
					}
				}
				else if( Result == eAbnormal )  /* 异常告警 */
				{
					cout<<"abnormal: "<<res->getRuleType()<<"\t"<<res->getReason()<<endl;
					Param.abnormal.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 0, 0, 0, 0, 0, 1);
					dealRcdNum.setFee( 0, 0, 0, 0, 0, (pps->m_inRcd).Get_Fee());
					dealRcdNum.setTime(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Time());
					res->resetAnaResult();
					Result = eNormal;
				}
				else
				{
					break;
				}
			}//end of while
			} catch (CException &e) {
				e.PushStack(e.GetAppError(), "process record error", __FILE__, __LINE__);
				throw e;
			} catch (const std::exception &e) {
				throw CException(0, e.what(), __FILE__, __LINE__);
			} catch (...) {
				throw CException(0, "unknown error", __FILE__, __LINE__);
			}

			//gettimeofday(&finish, NULL);
			//timePart3 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
/*			gettimeofday(&finish, NULL);
			timeMicros = (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
			theLog<<"用插件分析第"<<iTotalNum<<"条话单用时："<<timeMicros<<endi;
			*/
			//gettimeofday(&start, NULL);

			try {
				switch (Result)
				{
				case eNormal:  /* 正常  */
					cout<<"normal record"<<endl;
					if(Param.bOutputFile)
					{
						(res->m_outRcd).Get_record(tmpRec, RECLEN+1);//(res->m_outRcd).Get_record(tmpRec.record, RECLEN+1);
						//cout<<"write :"<<_outrcd.Get_record()<<endl;
						//strRec = tmpRec;
						//vecRec.push_back(strRec);
						_outfile.writeRec(res->m_outRcd);
					}
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					//_outrcd.Get_Field(Param.iServCatConfig, szServCat);
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 0);
					dealRcdNum.setFee((res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee(), 0, 0, 0, 0);
					dealRcdNum.setTime((res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time(), 0, 0, 0, 0);
					break;
				case eClassifiable:  /*互斥分拣话单*/
					cout<<"classify record"<<endl;
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					//_outrcd.Get_Field(Param.iServCatConfig, szServCat);
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 0, 1, 1);
					dealRcdNum.setFee((res->m_outRcd).Get_Fee(), 0, 0, 0, (res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee());
					dealRcdNum.setTime((res->m_outRcd).Get_Time(), 0, 0, 0, (res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time());
					break;
				case eRepeat:  /* 重单 */
					cout<<"repeat record"<<endl;
					Param.classify.dealRecord(res->getRuleType(), pps->m_inRcd);
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtErr:  /* 格式化错单 */
					cout<<"fmt error record"<<endl;
					iFmtErrorNum++;
					/* 入库或入文件 */
					if( !strcmp(Param.szFmtErr2Table, "Y") )
						Param.fmt_err2table.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					if( !strcmp(Param.szFmtErr2File, "Y") )
					{
						sprintf(szLongStr, "%d%c%s%c%s%c%s", pps->getOffset(), pps->m_inRcd.Get_FieldSep(1), res->getRuleType(), pps->m_inRcd.Get_FieldSep(1), res->getReason(), pps->m_inRcd.Get_FieldSep(1), pps->m_inRcd.Get_record());
						Param.fmt_err2file.writeRec(szLongStr);
					}
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					cout<<"ServCat in record("<<szServCat<<")"<<endl;
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					//cout<<(pRcd->getRes())->getRuleType()<<"\n"<<(pRcd->getRes())->getReason()<<endl;
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtTimeOut:  /* 格式化超时单  */
					cout<<"fmt timeout record"<<endl;
					iFmtTimeOutNum++;
					/* 入文件 */
					fprintf(fpFmtTimeOut, "%s\n", (pps->m_inRcd).Get_record());
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtOther:  /* 格式化未定义格式话单 */
					cout<<"fmt other record"<<endl;
					iFmtOtherNum++;
					/* 入文件 */
					fprintf(fpFmtOther, "%s\n", pps->getString());
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					dealRcdNum.setFee(0, 0, 0, 0, 0, 0);
					dealRcdNum.setTime(0, 0, 0, 0, 0, 0);
					break;
				case eAbnormal:  /* 异常告警 */
					cout<<"abnormal record"<<endl;
					Param.abnormal.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 1);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), (pps->m_inRcd).Get_Fee(), 0, 0, 0, (pps->m_inRcd).Get_Fee());
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), (pps->m_inRcd).Get_Time(), 0, 0, 0, (pps->m_inRcd).Get_Time());
					break;
				default:  /* 无资料或异常 */
					cout<<"lackinfo record:"<<res->getRuleType()<<"\t"<<res->getReason()<<endl;
					Param.lack_info.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* 记录处理数 */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 1, 0, 0, 0);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, (pps->m_inRcd).Get_Fee(), 0, 0, 0);
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, (pps->m_inRcd).Get_Time(), 0, 0, 0);
					break;
				}
				Param.ProcMonitor++;
				chain->reset();

			} catch (CException &e) {
				e.PushStack(e.GetAppError(), "analyze record result error",
						__FILE__, __LINE__);
				throw e;
			} catch (const std::exception &e) {
				throw CException(1331609, e.what(), __FILE__, __LINE__);
			} catch (...) {
				throw CException(1331609, "unknown error", __FILE__, __LINE__);
			}


			//gettimeofday(&finish, NULL);
			//timePart4 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);

		}/* end of while */

		/*
		gettimeofday(&finish, NULL);
		unsigned long timeAll = (finish.tv_sec - filetime.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - filetime.tv_usec);
		theLog << "##################### file process in " << timeAll << "(micros)" << endi;
		theLog << "######################## timePart1:" << timePart1 << endi;
		theLog << "######################## timePart2:" << timePart2 << endi;
		theLog << "######################## timePart3:" << timePart3 << endi;
		theLog << "######################## timePart4:" << timePart4 << endi;
		*/
		
		/* 关闭输入输出文件 */
		_outfile.Close();
		_infile.Close();
		Param.classify.endFile();

		/* 处理格式化错单 */
		if( !strcmp(Param.szIsFmtFirst, "Y") )
		{
			fclose(fpFmtTimeOut);
			fclose(fpFmtOther);

			if(iFmtTimeOutNum > 0)
			{
				if(rename(szFmtTimeOutTmp,szFmtTimeOutFile) != 0 )
				{
					// 信息点日志
					Param.info.SetFile(FileStruct.szFileName, FileStruct.szSourceId, FileStruct.szDealStartTime, FileStruct.szDealEndTime, 'E');
					Param.info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
						dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
					Param.info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
						dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
					Param.info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
						dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
					Param.info.InsertData();
					sprintf(szLogStr,"将文件 %s 改成 %s 失败！",szFmtTimeOutTmp, szFmtTimeOutFile);
					//throw CException(PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
					errLog(4, FileStruct.szFileName, PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
					return -2;
				}
			}
			else
				unlink(szFmtTimeOutTmp);
			if(iFmtOtherNum > 0)
			{
				if(rename(szFmtOtherTmp,szFmtOtherFile) != 0 )
				{
					// 信息点日志
					Param.info.SetFile(FileStruct.szFileName, FileStruct.szSourceId, FileStruct.szDealStartTime, FileStruct.szDealEndTime, 'E');
					Param.info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
						dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
					Param.info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
						dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
					Param.info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
						dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
					Param.info.InsertData();
					sprintf(szLogStr,"将文件 %s 改成 %s 失败！",szFmtOtherTmp, szFmtOtherFile);
					//throw CException(PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
					errLog(4, FileStruct.szFileName, PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
					return -2;
				}
			}
			else
				unlink(szFmtOtherTmp);
			if( !strcmp(Param.szFmtErr2File, "Y") )
			{
				Param.fmt_err2file.Close(1);
				if(iFmtErrorNum > 0)
				{
					if(rename(szFmtErrTmp,szFmtErrFile) != 0 )
					{
						// 信息点日志
						Param.info.SetFile(FileStruct.szFileName, FileStruct.szSourceId, FileStruct.szDealStartTime, FileStruct.szDealEndTime, 'E');
						Param.info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
							dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
						Param.info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
							dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
						Param.info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
							dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
						Param.info.InsertData();
						sprintf(szLogStr,"将文件 %s 改成 %s 失败！",szFmtErrTmp, szFmtErrFile);
						//throw CException(PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
						errLog(4, FileStruct.szFileName, PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
						return -2;
					}
				}
			}
		}
		/* 存储要改名的信息 */
		strcpy(fname.m_szOutTmpFileName, OutTmpFilePath);
		strcpy(fname.m_szOutRealFileName, OutRealFilePath);
		strcpy(fname.szFileName, FileStruct.szFileName);
		strcpy(fname.szSourceId, FileStruct.szSourceId);
		strcpy(fname.szDealStartTime, FileStruct.szDealStartTime);
		strcpy(fname.szDealEndTime, FileStruct.szDealEndTime);
		vecFile.push_back(fname);
		
		/* 纪录不平衡话单数等 */
		dealRcdNum.check();

		/* 登记处理日志 */
		dealRcdNum.updateTable(Param.szLogTabname, FileStruct, Param.iProcessId);
		
		// Print performance
		/*
		filterchain::util::PerformanceAnalyzer* analyzer =
				Param.pluginInitializer.getPerformanceAnalyzer();
		if (analyzer) {
			std::map<std::string,
					filterchain::util::PerformanceAnalyzer::ActionPerformance*>
					actionPerformance = analyzer->getActionPerformances();
			for (std::map<std::string,
					filterchain::util::PerformanceAnalyzer::ActionPerformance*>::iterator
					it = actionPerformance.begin(); it
					!= actionPerformance.end(); it++) {
				filterchain::util::PerformanceAnalyzer::ActionPerformance* ap =
						it->second;
				theLog << "action:" << ap->getActionId() << " time:"
						<< ap->getTimeMillisCounter() << " run:"
						<< ap->getRunningCounter() << " reset:"
						<< ap->getResetCounter() << " signal:"
						<< ap->getSignalCounter() << endi;
			}
		}
		*/

//		pps->printTime();
//		res->printTime();

	}
	catch(CException &e)
	{
		sprintf(szLogStr, "处理文件(%s)第%d行失败", FileStruct.szFileName, iTotalNum);
		e.PushStack(e.GetAppError(), szLogStr, __FILE__, __LINE__);
		throw e;
	}
	catch(const std::exception &e){
		sprintf(szLogStr, "%s\n处理文件(%s)第%d行失败", e.what(), FileStruct.szFileName, iTotalNum);
		throw CException(1331609, szLogStr, __FILE__, __LINE__);
	}

	return 0;
}

//int CDealFile::endfile(int iFileIndex, vector<SAlterRecordAfterDeal> &vecMod)
int CDealFile::endfile(vector<SAlterRecordAfterDeal> &vecMod, InfoLog &info, bool OutputFile)
{
	char szTemp[FIELD_LEN+1];
	memset(szTemp, 0, sizeof(szTemp));
	int iRet;
	int iFileIndex = vecFile.size() - 1;
//	if(OutputFile)
//	{
//		if(vecMod.size() > 0)
//		{
//			//按OFFSET排序
//		}
//		/* 插件发送文件结束消息后的操作 */
//		//int iFileIndex = vecFile.size() - 1;
//		if(iFileIndex < 0)
//			return -1;
//		CF_MemFileO _outfile;
//		_outfile.Init( vecFile[iFileIndex].szOutputFiletypeId );
//		_outfile.Open(vecFile[iFileIndex].m_szOutTmpFileName);
//		CFmt_Change _outrcd;
//		_outrcd.Init(vecFile[iFileIndex].szOutputFiletypeId, vecFile[iFileIndex].szOutrcdType);
//		int iModIndex = 0;
//		for(int i=0; i<vecRec.size(); i++)
//		{
//			if( _outrcd.Set_record((char *)vecRec[i].c_str(), vecRec[i].length()) != 0)
//			{
//				throw CException(0, "写出口文件出错", __FILE__, __LINE__);
//			}
//			//改内容
//			if(iModIndex < vecMod.size())
//			{
//				memset(szTemp, 0, sizeof(szTemp));
//				_outrcd.Get_Field(OFFSET_FIELD_NAME, szTemp);
//				iRet = strcmp(vecMod[iModIndex].szOffset, szTemp);
//				if(iRet == 0)
//				{
//					for(int j=iModIndex; j<vecMod.size(); j++)
//					{
//						if(!strcmp(vecMod[j].szOffset, szTemp))
//						{
//							_outrcd.Set_Field(vecMod[j].szColName, vecMod[j].szColValue);
//							iModIndex++;
//						}
//						else
//							break;
//					}
//				}
//			}
//			_outfile.writeRec(_outrcd);
//		}
//		_outfile.Close();
//	}
//	vecRec.clear();

	// 信息点日志
	info.SetFile(vecFile[iFileIndex].szFileName, vecFile[iFileIndex].szSourceId, vecFile[iFileIndex].szDealStartTime, \
		vecFile[iFileIndex].szDealEndTime, 'Y');
	info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
		dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
	info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
		dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
	info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
		dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
	info.InsertData();

	return 0;
}

int CDealFile::commit(bool OutputFile)
{
	if(OutputFile)
	{
		char szLogStr[LOG_MSG_LEN+1];
		for(int i=0; i<vecFile.size(); i++)
		{
			if(rename(vecFile[i].m_szOutTmpFileName, vecFile[i].m_szOutRealFileName) != 0 )
			{
				/* 文件改名失败 */
				sprintf(szLogStr,"将文件 %s 改成 %s 失败！",vecFile[i].m_szOutTmpFileName, vecFile[i].m_szOutRealFileName);
				throw CException(PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
			}
		}
	}
	vecFile.clear();
	return 0;
}

int CDealFile::rollback()
{
	vecFile.clear();
	return 0;
}

CDealedFileInfo::CDealedFileInfo()
{
	clear();
}

CDealedFileInfo::~CDealedFileInfo()
{
}

void CDealedFileInfo::clear()
{
	memset(m_szOutTmpFileName, 0, sizeof(m_szOutTmpFileName));
	memset(m_szOutRealFileName, 0, sizeof(m_szOutRealFileName));
	memset( szOutputFiletypeId, 0, sizeof(szOutputFiletypeId) );
	szOutrcdType = '0';
	memset(szFileName, 0, sizeof(szFileName));
	memset(szSourceId, 0, sizeof(szSourceId));
	memset(szDealStartTime, 0, sizeof(szDealStartTime));
	memset(szDealEndTime, 0, sizeof(szDealEndTime));
	//dealRcdNum.clear();
}
