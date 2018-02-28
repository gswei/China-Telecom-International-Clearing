/****************************************************************
filename: dealfile.cpp
module:
created by: ouyh
create date: 
version: 3.0.0
description:
    �����ļ�������
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

	/*�������*/
	char szLogStr[LOG_MSG_LEN+1];
	char szLongStr[MAX_LINE_LENGTH+1];

	/* ��ʼ�������Ͻӿ� */
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
	/* ��ʼ����ʽ���� */
	if( !strcmp(Param.szIsFmtFirst, "Y") )
	{
		//sprintf(Param.szFmtErrDir, "%s%s", FileStruct.szSourcePath, Param.szError_Dir);
		sprintf(szFmtTimeOutFile, "%s%s%s", FileStruct.szSourcePath, Param.szFmtErrDir, Param.szFmtTimeOutDir);
		if( chkAllDir( szFmtTimeOutFile )!=0 )
		{
			sprintf(szLogStr, "�޷�����·���� %s ��", szFmtTimeOutFile);
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		sprintf(szFmtTimeOutFile, "%s%s", szFmtTimeOutFile, FileStruct.szFileName);
		sprintf(szFmtTimeOutTmp, "%s.tmp", szFmtTimeOutFile);
		if((fpFmtTimeOut=fopen(szFmtTimeOutTmp,"w"))==NULL)
		{
			sprintf(szLogStr, "���ļ��� %s ����", szFmtTimeOutTmp);
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		sprintf(szFmtOtherFile, "%s%s%s", FileStruct.szSourcePath, Param.szFmtErrDir, Param.szFmtOtherDir);
		if( chkAllDir( szFmtOtherFile )!=0 )
		{
			sprintf(szLogStr, "�޷�����·���� %s ��", szFmtOtherFile);
			throw CException(0, szLogStr, __FILE__, __LINE__);
		}
		sprintf(szFmtOtherFile, "%s%s", szFmtOtherFile, FileStruct.szFileName);
		sprintf(szFmtOtherTmp, "%s.tmp", szFmtOtherFile);
		if((fpFmtOther=fopen(szFmtOtherTmp,"w"))==NULL)
		{
			fclose(fpFmtTimeOut);
			sprintf(szLogStr, "���ļ��� %s ����", szFmtOtherTmp);
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

	/* ��ʼ���ʹ������ļ� */
	char InFilePath[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char InFileName[FILE_NAME_LEN+1];
	CF_MemFileI _infile;
	sprintf(InFilePath, "%s%s%s", FileStruct.szSourcePath, Param.szInPath, FileStruct.szFileName);
	sprintf(InFileName, "%s", FileStruct.szFileName);
	CFmt_Change _inrcd;
	//cout<<"��ʼ�������ļ�"<<Param.szOutputFiletypeId<<endl;
	if( strcmp(Param.szIsFmtFirst,"Y")==0 )
		_infile.Init(FileStruct.szSourceFiletype);
	else
		_infile.Init(Param.szOutputFiletypeId);
	//cout<<"��ʼ�����뻰����ʽ"<<Param.szOutputFiletypeId<<"\t"<<Param.szOutrcdType<<endl;
	_inrcd.Init(Param.szOutputFiletypeId, Param.szOutrcdType[0]);
	_infile.Open(InFilePath);
	
	long lFileNum = _infile.get_num();
	Param.ProcMonitor.UpdateMonitor(FileStruct.szFileName, lFileNum, 'Y', CProcInfo::SERV_BUSY);
	
	/*��ʼ���ʹ�����ļ�*/
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
		//cout<<"��ʼ������ļ�"<<Param.szOutputFiletypeId<<endl;
		_outfile.Init( Param.szOutputFiletypeId );
		_outfile.Open(OutTmpFilePath);
	}

	
	CDealedFileInfo fname;
	strcpy(fname.szOutputFiletypeId, Param.szOutputFiletypeId);
	fname.szOutrcdType = Param.szOutrcdType[0];
//	CFmt_Change _outrcd;
	//cout<<"��ʼ�����������ʽ"<<Param.szOutputFiletypeId<<"\t"<<Param.szOutrcdType<<endl;
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
		sprintf(szLogStr, "�Ҳ��������");
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
	theLog<<"��ʼ���ļ�ʹ��ʱ�䣺"<<timeMicros<<endi;
	gettimeofday(&start, NULL);
	*/

	pps->clearAll();
	res->clearAll();
	/* ��Ĭ�ϵ���ʱ���� */
	for(TMP_VAR::iterator it = Param.map_DefVar.begin(); it != Param.map_DefVar.end(); it++)
	{
		pps->addVariable(it->szVarName, it->szVarValue);
		res->addVariable(it->szVarName, it->szVarValue);
	}
	
	//gettimeofday(&filetime, NULL);
	
	/* �ظ���ȡ���еĻ�����������д��� */ 
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

				/* д�ļ�ƫ���� */
				int iPos = _infile.GetPos();
				pps->setOffset(iPos);

				if( strcmp(Param.szIsFmtFirst,"Y")==0 )
				{
					memset(szBuff, 0, sizeof(szBuff));
					if( _infile.readRec(szBuff, MAX_LINE_LENGTH) != READ_AT_END )
					{
						cout<<"it is format first"<<endl;
						/* ��һ�����Ϊ��ʽ��ʱ */
						if(strlen(szBuff)==0)
							break;
						pps->setRecord(szBuff);
						//theLog<<"�����"<<iTotalNum+1<<"�У�"<<pps->getString()<<endd;
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
//			theLog<<"��ʼ��������Ϣ��ʱ��"<<timeMicros<<endi;
			//timePart1 += (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);;
			//gettimeofday(&start, NULL);

			try{
			/* ������� */
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
//					theLog<<"LINE��"<<iTotalNum<<"<<��ǰִ�еĲ��/���ʽID��"<<ret->getRelatedActionId()<<"   ���ͣ�" <<ret->getRelatedActionType()<<"   ��ʱ�� "<<ret->getUsedTimeMicros()<<endd;
//				}
				theLog<<"LINE��"<<iTotalNum<<"<<��ǰִ�еĲ��/���ʽID��"<<ret->getRelatedActionId()<<"   ���ͣ�" <<ret->getRelatedActionType()<<endi;
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
						//����ּ𻰵�
						break;
					}
					else if(classifyType == 3)
					{
						//���ݷּ𻰵�
						res->resetAnaResult();
						Result = eNormal;
						/* ��¼������ */
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
						// �ּ����
						//theLog<<"�ּ������"<<endw;
						if( !strcmp(Param.szIsFmtFirst, "Y") )
						{
							fclose(fpFmtTimeOut);
							fclose(fpFmtOther);
							if( !strcmp(Param.szFmtErr2File, "Y") )
								Param.fmt_err2file.Close();
						}
						//return -1;
						throw CException(errno, "�ּ������", __FILE__, __LINE__);
					}
				}
				else if( Result == eAbnormal )  /* �쳣�澯 */
				{
					cout<<"abnormal: "<<res->getRuleType()<<"\t"<<res->getReason()<<endl;
					Param.abnormal.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
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
			theLog<<"�ò��������"<<iTotalNum<<"��������ʱ��"<<timeMicros<<endi;
			*/
			//gettimeofday(&start, NULL);

			try {
				switch (Result)
				{
				case eNormal:  /* ����  */
					cout<<"normal record"<<endl;
					if(Param.bOutputFile)
					{
						(res->m_outRcd).Get_record(tmpRec, RECLEN+1);//(res->m_outRcd).Get_record(tmpRec.record, RECLEN+1);
						//cout<<"write :"<<_outrcd.Get_record()<<endl;
						//strRec = tmpRec;
						//vecRec.push_back(strRec);
						_outfile.writeRec(res->m_outRcd);
					}
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					//_outrcd.Get_Field(Param.iServCatConfig, szServCat);
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 0);
					dealRcdNum.setFee((res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee(), 0, 0, 0, 0);
					dealRcdNum.setTime((res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time(), 0, 0, 0, 0);
					break;
				case eClassifiable:  /*����ּ𻰵�*/
					cout<<"classify record"<<endl;
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					//_outrcd.Get_Field(Param.iServCatConfig, szServCat);
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 0, 1, 1);
					dealRcdNum.setFee((res->m_outRcd).Get_Fee(), 0, 0, 0, (res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee());
					dealRcdNum.setTime((res->m_outRcd).Get_Time(), 0, 0, 0, (res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time());
					break;
				case eRepeat:  /* �ص� */
					cout<<"repeat record"<<endl;
					Param.classify.dealRecord(res->getRuleType(), pps->m_inRcd);
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtErr:  /* ��ʽ���� */
					cout<<"fmt error record"<<endl;
					iFmtErrorNum++;
					/* �������ļ� */
					if( !strcmp(Param.szFmtErr2Table, "Y") )
						Param.fmt_err2table.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					if( !strcmp(Param.szFmtErr2File, "Y") )
					{
						sprintf(szLongStr, "%d%c%s%c%s%c%s", pps->getOffset(), pps->m_inRcd.Get_FieldSep(1), res->getRuleType(), pps->m_inRcd.Get_FieldSep(1), res->getReason(), pps->m_inRcd.Get_FieldSep(1), pps->m_inRcd.Get_record());
						Param.fmt_err2file.writeRec(szLongStr);
					}
					/* ��¼������ */
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
				case eFmtTimeOut:  /* ��ʽ����ʱ��  */
					cout<<"fmt timeout record"<<endl;
					iFmtTimeOutNum++;
					/* ���ļ� */
					fprintf(fpFmtTimeOut, "%s\n", (pps->m_inRcd).Get_record());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtOther:  /* ��ʽ��δ�����ʽ���� */
					cout<<"fmt other record"<<endl;
					iFmtOtherNum++;
					/* ���ļ� */
					fprintf(fpFmtOther, "%s\n", pps->getString());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					dealRcdNum.setFee(0, 0, 0, 0, 0, 0);
					dealRcdNum.setTime(0, 0, 0, 0, 0, 0);
					break;
				case eAbnormal:  /* �쳣�澯 */
					cout<<"abnormal record"<<endl;
					Param.abnormal.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, FileStruct.szServCat);
					dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 1);
					dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), (pps->m_inRcd).Get_Fee(), 0, 0, 0, (pps->m_inRcd).Get_Fee());
					dealRcdNum.setTime((pps->m_inRcd).Get_Time(), (pps->m_inRcd).Get_Time(), 0, 0, 0, (pps->m_inRcd).Get_Time());
					break;
				default:  /* �����ϻ��쳣 */
					cout<<"lackinfo record:"<<res->getRuleType()<<"\t"<<res->getReason()<<endl;
					Param.lack_info.saveErrLackRec(pps->m_inRcd, FileStruct.szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
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
		
		/* �ر���������ļ� */
		_outfile.Close();
		_infile.Close();
		Param.classify.endFile();

		/* �����ʽ���� */
		if( !strcmp(Param.szIsFmtFirst, "Y") )
		{
			fclose(fpFmtTimeOut);
			fclose(fpFmtOther);

			if(iFmtTimeOutNum > 0)
			{
				if(rename(szFmtTimeOutTmp,szFmtTimeOutFile) != 0 )
				{
					// ��Ϣ����־
					Param.info.SetFile(FileStruct.szFileName, FileStruct.szSourceId, FileStruct.szDealStartTime, FileStruct.szDealEndTime, 'E');
					Param.info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
						dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
					Param.info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
						dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
					Param.info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
						dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
					Param.info.InsertData();
					sprintf(szLogStr,"���ļ� %s �ĳ� %s ʧ�ܣ�",szFmtTimeOutTmp, szFmtTimeOutFile);
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
					// ��Ϣ����־
					Param.info.SetFile(FileStruct.szFileName, FileStruct.szSourceId, FileStruct.szDealStartTime, FileStruct.szDealEndTime, 'E');
					Param.info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
						dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
					Param.info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
						dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
					Param.info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
						dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
					Param.info.InsertData();
					sprintf(szLogStr,"���ļ� %s �ĳ� %s ʧ�ܣ�",szFmtOtherTmp, szFmtOtherFile);
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
						// ��Ϣ����־
						Param.info.SetFile(FileStruct.szFileName, FileStruct.szSourceId, FileStruct.szDealStartTime, FileStruct.szDealEndTime, 'E');
						Param.info.SetCount(dealRcdNum.getTotal(), dealRcdNum.getRight(), dealRcdNum.getError(), \
							dealRcdNum.getLack(), dealRcdNum.getPick(), dealRcdNum.getOther());
						Param.info.SetFee(dealRcdNum.getTotalFee(), dealRcdNum.getRightFee(), dealRcdNum.getErrorFee(), \
							dealRcdNum.getLackFee(), dealRcdNum.getPickFee(), dealRcdNum.getOtherFee());
						Param.info.SetDuration(dealRcdNum.getTotalTime(), dealRcdNum.getRightTime(), dealRcdNum.getErrorTime(), \
							dealRcdNum.getLackTime(), dealRcdNum.getPickTime(), dealRcdNum.getOtherTime());
						Param.info.InsertData();
						sprintf(szLogStr,"���ļ� %s �ĳ� %s ʧ�ܣ�",szFmtErrTmp, szFmtErrFile);
						//throw CException(PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
						errLog(4, FileStruct.szFileName, PREDEAL_ERR_PK_DUPLICATE, szLogStr, __FILE__, __LINE__);
						return -2;
					}
				}
			}
		}
		/* �洢Ҫ��������Ϣ */
		strcpy(fname.m_szOutTmpFileName, OutTmpFilePath);
		strcpy(fname.m_szOutRealFileName, OutRealFilePath);
		strcpy(fname.szFileName, FileStruct.szFileName);
		strcpy(fname.szSourceId, FileStruct.szSourceId);
		strcpy(fname.szDealStartTime, FileStruct.szDealStartTime);
		strcpy(fname.szDealEndTime, FileStruct.szDealEndTime);
		vecFile.push_back(fname);
		
		/* ��¼��ƽ�⻰������ */
		dealRcdNum.check();

		/* �ǼǴ�����־ */
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
		sprintf(szLogStr, "�����ļ�(%s)��%d��ʧ��", FileStruct.szFileName, iTotalNum);
		e.PushStack(e.GetAppError(), szLogStr, __FILE__, __LINE__);
		throw e;
	}
	catch(const std::exception &e){
		sprintf(szLogStr, "%s\n�����ļ�(%s)��%d��ʧ��", e.what(), FileStruct.szFileName, iTotalNum);
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
//			//��OFFSET����
//		}
//		/* ��������ļ�������Ϣ��Ĳ��� */
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
//				throw CException(0, "д�����ļ�����", __FILE__, __LINE__);
//			}
//			//������
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

	// ��Ϣ����־
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
				/* �ļ�����ʧ�� */
				sprintf(szLogStr,"���ļ� %s �ĳ� %s ʧ�ܣ�",vecFile[i].m_szOutTmpFileName, vecFile[i].m_szOutRealFileName);
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
