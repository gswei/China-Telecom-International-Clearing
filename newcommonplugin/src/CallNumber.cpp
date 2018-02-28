/****************************************************************
 filename: CallNumber.cpp
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
	������������Է�װ
 update:
20090309 by ouyh  ʡ����0��ͷ�ĺ��벻�ж�˫����
20090505 by ouyh  ���й������ź����С��1λ���𣬹���С��3λ���𲻱�
 *****************************************************************/

#include "CallNumber.h"

C_CallNumber::C_CallNumber()
{
	m_iIndex = 1;
}
 
C_CallNumber::~C_CallNumber()
{
	m_DataIn.clear();
	m_DataOut.clear();
	map_DefAttribute.clear();
}

/*************************************************
* ����    ����ʼ��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
//int C_CallNumber::Init(char* szServiceId, char* szSourceID, BaseAccessMem *pAccessMem)
int C_CallNumber::Init(BaseAccessMem *pAccessMem)
{
	char sztmp[FIELD_LEN+1];
	memset( sztmp, 0, sizeof( sztmp ) );
	getFromGlobalEnv( sztmp, "GSM_LENGTH" );
	m_iGsmLength = atoi(sztmp);

//	strcpy(m_InputParam.m_szServiceId, szServiceId);
//	strcpy(m_InputParam.m_szSourceId, szSourceID);

	strcpy(table_DelTelHead, "I_TELHEADER_DELETE");
	strcpy(table_BefTolSer, "I_BEFTOLLCODE_SERVICE");
	strcpy(table_Gsm2Tollcode, "I_GSM2TOLLCODE");
	strcpy(table_TelenoPro, "I_TELENO_PROPERTY");
	strcpy(table_TelenoSer, "I_TELENO_SERVICE");
	strcpy(table_TollCode, "I_TOLLCODE");
	strcpy(table_Localnet, "I_LOCALNET");
	strcpy(table_FreeTel, "I_FREE_TELHEADER");
	strcpy(table_TelenoLen, "I_TELENO_LENGTH");
	strcpy(table_TelDefPro, "I_TELENO_DEF_PROPERTY");
	strcpy(table_FreeLack, "I_FREE_LACKHEADER");
	//���ӹ����ڴ�
	table = pAccessMem;
	if( table == NULL )
	{
		theJSLog<<"��ʼ��AccessMem��ʧ�ܣ�"<<ende;
		throw jsexcp::CException(0,"��ʼ��AccessMem��ʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableDelTelHeadOffset = table->getTableOffset(table_DelTelHead)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TELHEADER_DELETEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableBefTolSerOffset = table->getTableOffset(table_BefTolSer)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_BEFTOLLCODE_SERVICEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableGsm2TollcodeOffset = table->getTableOffset(table_Gsm2Tollcode)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_GSM2TOLLCODEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableTelenoProOffset = table->getTableOffset(table_TelenoPro)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TELENO_PROPERTYʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableTelenoSerOffset = table->getTableOffset(table_TelenoSer)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TELENO_SERVICEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableTollCodeOffset = table->getTableOffset(table_TollCode)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TOLLCODEʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableLocalnetOffset = table->getTableOffset(table_Localnet)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_LOCALNETʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableFreeTelOffset = table->getTableOffset(table_FreeTel)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_FREE_TELHEADERʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableTelenoLenOffset = table->getTableOffset(table_TelenoLen)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TELENO_LENGTHʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableTelDefProOffset = table->getTableOffset(table_TelDefPro)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_TELENO_DEF_PROPERTYʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	if( (m_iTableFreeLackOffset = table->getTableOffset(table_FreeLack)) < 0)
	{
		throw jsexcp::CException(0,"��ʼ����I_FREE_LACKHEADERʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	
	if( Gsm2Toll.Init(table_Gsm2Tollcode) != 0 )
	{
		theJSLog<<"��ʼ���ֻ�����ͷʧ�ܣ�"<<ende;
		throw jsexcp::CException(0,"��ʼ���ֻ�����ͷʧ�ܣ�",(char *)__FILE__,__LINE__);
	}
	return 0;
}

int C_CallNumber::Reset(char* szServiceId, char* szSourceID)
{
	strcpy(m_InputParam.m_szServiceId, szServiceId);
	strcpy(m_InputParam.m_szSourceId, szSourceID);

	//Ĭ�����ԵĻ�ȡ
	//�Ȳ���map
	mapIt_DefAttribute=map_DefAttribute.find((string)szSourceID);
	if(mapIt_DefAttribute!=map_DefAttribute.end())
	{
		//ֱ��ʹ��map�еĲ���
		strcpy(m_DefServBefore,(mapIt_DefAttribute->second).m_DefServBefore);
		strcpy(m_DefServAfter,(mapIt_DefAttribute->second).m_DefServAfter);
		strcpy(m_DefBusiness,(mapIt_DefAttribute->second).m_DefBusiness);
		strcpy(m_DefMobile,(mapIt_DefAttribute->second).m_DefMobile);
	}
	else
	{
		loadDefTelenoAttr();
		strcpy(ms_Attribute.m_DefServBefore,m_DefServBefore);
		strcpy(ms_Attribute.m_DefServAfter,m_DefServAfter);
		strcpy(ms_Attribute.m_DefBusiness,m_DefBusiness);
		strcpy(ms_Attribute.m_DefMobile,m_DefMobile);
		map_DefAttribute.insert(make_pair((string)szSourceID,ms_Attribute));
	}
	
	return 0;
}

/**************************************************************/

int C_CallNumber::Set_Value()
{
	memset(&m_CurTelAttr,0,sizeof(m_CurTelAttr));
	return 0;
}
int C_CallNumber::Set_ServCatId(char * m_szServCatId)
{
	strcpy(m_InputParam.m_szServCatId,m_szServCatId);	
	return 0;	
}

int C_CallNumber::Set_SourceId(char * m_szSourceId)
{
	strcpy(m_InputParam.m_szSourceId,m_szSourceId);	
	return 0;
}
int C_CallNumber::Set_CallNbr(char * m_szCallNbr)
{
	strcpy(m_InputParam.m_szTeleno,m_szCallNbr);	
	strcpy(m_CurTelAttr.m_szTeleno,m_szCallNbr);	
	return 0;
}

int C_CallNumber::Set_NbrType(char *m_szNbrType)
{
	strcpy(m_InputParam.m_szTelenoFlag,m_szNbrType);	
	strcpy(m_CurTelAttr.m_szTelenoFlag,m_szNbrType);	
	return 0;

}
int C_CallNumber::Set_CdrBegin(char * m_szCdrBegin)
{
	strcpy(m_InputParam.m_szCdrBegin,m_szCdrBegin);	
	return 0;
}
int C_CallNumber::Set_DefTollcode(char * m_szDefTollcode)
{
	strcpy(m_InputParam.m_szLocalToll,m_szDefTollcode);	
	return 0;
}


int C_CallNumber::Set_BefTollCode(char * m_szBefTollcode)
{
	strcpy(m_InputParam.m_szLocalToll,m_szBefTollcode);	
	return 0;
}

int C_CallNumber::Set_ChargeFlag(char * m_szChargeFlag)
{
	strcpy(m_InputParam.m_szChargeFlag,m_szChargeFlag);	
	strcpy(m_CurTelAttr.m_szChargeFlag,m_szChargeFlag);	
	return 0;
}


//add by weixy 20080219
int C_CallNumber::Set_Tollcode(char * m_szTollcode)
{
	strcpy(m_CurTelAttr.m_szTollcode,m_szTollcode);	
	return 0;
}
//end add by weixy 20080219

/**************************************************************/

int C_CallNumber::Get_BefTollTel(char * m_szBefTollTel)
{
	strcpy(m_szBefTollTel,m_CurTelAttr.m_szBefTollTel);	
	return 0;
}
int C_CallNumber::Get_Tollcode(char * m_szTollcode)
{
	strcpy(m_szTollcode,m_CurTelAttr.m_szTollcode);	
	return 0;
}
int C_CallNumber::Get_AfterTollTel(char * m_szAfterTollTel)
{
	strcpy(m_szAfterTollTel,m_CurTelAttr.m_szAfterTollTel);	
	return 0;
}

int C_CallNumber::Get_Teleno(char * m_szTeleno)
{
	strcpy(m_szTeleno,m_CurTelAttr.m_szTeleno);	
	return 0;
}

int C_CallNumber::Get_District(char * m_szDistrict)
{
	strcpy(m_szDistrict,m_CurTelAttr.m_szDistrict);	
	return 0;
}
int C_CallNumber::Get_BusiAfter(char * m_szBusiAfter)
{	
	if (! strcmp(m_CurTelAttr.m_szBusiAfter,""))
		strcpy(m_szBusiAfter,m_DefBusiness);			
	else
		strcpy(m_szBusiAfter,m_CurTelAttr.m_szBusiAfter);	
	return 0;
}
int C_CallNumber::Get_BusiBefore(char * m_szBusiBefore)
{
	strcpy(m_szBusiBefore,m_CurTelAttr.m_szBusiBefore);	
	return 0;
}
int C_CallNumber::Get_BusiPriority(char * m_szBusiPriority)
{
	strcpy(m_szBusiPriority,m_CurTelAttr.m_szBusiPriority);	
	return 0;
}
int C_CallNumber::Get_Mobile(char * m_szMobile)
{
	if (! strcmp(m_CurTelAttr.m_szMobile,""))
		strcpy(m_szMobile,m_DefMobile);			
	else
		strcpy(m_szMobile,m_CurTelAttr.m_szMobile);	
	return 0;
}
int C_CallNumber::Get_SvrBefore(char * m_szSvrBefore)
{
	if (! strcmp(m_CurTelAttr.m_szSvrBefore,""))
		strcpy(m_szSvrBefore,m_DefServBefore);	
	else
		strcpy(m_szSvrBefore,m_CurTelAttr.m_szSvrBefore);	
	return 0;
}
int C_CallNumber::Get_SvrAfter(char * m_szSvrAfter)
{
	if (! strcmp(m_CurTelAttr.m_szSvrAfter,""))
		strcpy(m_szSvrAfter,m_DefServAfter);			
	else
		strcpy(m_szSvrAfter,m_CurTelAttr.m_szSvrAfter);	
	return 0;
}

int C_CallNumber::Get_ChargeFlag(char *m_szChargeFlag)
{
	strcpy(m_szChargeFlag,m_CurTelAttr.m_szChargeFlag);	
	return 0;
}

/**************************************************************/
int C_CallNumber::queryRubTelHead(char * m_szServCatId,char * m_szSourceId,char *m_szTelenoFlag,char*m_szTeleno,char*m_szCdrBegin,int Len,int &ODel,int &OPos )
{
	m_DataIn.clear();
	m_DataIn.set(m_szServCatId, 0);
	m_DataIn.set(m_szSourceId, 0);
	m_DataIn.set(m_szTelenoFlag, 0);
	m_DataIn.set(m_szTeleno, 0);
	strcpy(m_DataIn.startTime, m_szCdrBegin);
	m_DataIn.itemNum = 1;
	
	int MinLen,MaxLen,BeginPos,DelLen;

	m_DataOut.clear();
	if(table->getData(m_iTableDelTelHeadOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		MinLen=atoi(m_DataOut.values[6]);
		MaxLen=atoi(m_DataOut.values[7]);
		BeginPos=atoi(m_DataOut.values[8]);
		DelLen=atoi(m_DataOut.values[9]);

		if (Len>=MinLen && Len<=MaxLen)
		{
			//���ȺϷ�����ȡ�������
			ODel=DelLen;
			OPos=BeginPos;
			return 0;
		}
	}
	
	return -1;
}



int C_CallNumber::DealRubTelHead()
{	
	char tmp_str[FIELD_LEN+1];
	int ret,Len,DelLen,BeginPos;
	Len=strlen(m_InputParam.m_szTeleno);
	strcpy(tmp_str,m_InputParam.m_szTeleno);
	int tmp_len=Len;
	
	ret=queryRubTelHead(m_InputParam.m_szServCatId,m_InputParam.m_szSourceId,m_InputParam.m_szTelenoFlag,tmp_str,m_InputParam.m_szCdrBegin,Len,DelLen,BeginPos);

	if (ret==0)
	{
		strncpy(m_CurTelAttr.m_szTeleno,m_InputParam.m_szTeleno,BeginPos-1);
		m_CurTelAttr.m_szTeleno[BeginPos-1]=0;
		strcat(m_CurTelAttr.m_szTeleno,m_InputParam.m_szTeleno+BeginPos-1+DelLen);
		strcpy(m_InputParam.m_szTeleno,m_CurTelAttr.m_szTeleno);
	}

	return 0;
}

int C_CallNumber::DealBefTelServ()
{
	m_DataIn.clear();
	m_DataIn.set(m_InputParam.m_szLocalToll, 0);
	m_DataIn.set(m_InputParam.m_szTelenoFlag, 0);
	m_DataIn.set(m_InputParam.m_szTeleno, 0);
	m_DataIn.itemNum = 1;
	strcpy(m_DataIn.startTime, m_InputParam.m_szCdrBegin);

	m_DataOut.clear();
	if(table->getData(m_iTableBefTolSerOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		//���������ҽ��
		strcpy(m_CurTelAttr.m_szTeleno,(m_InputParam.m_szTeleno+atoi(m_DataOut.values[5])));
		strncpy(m_CurTelAttr.m_szBefTollTel,m_InputParam.m_szTeleno,atoi(m_DataOut.values[5]));
		m_CurTelAttr.m_szBefTollTel[atoi(m_DataOut.values[5])]=0;
		strcpy(m_CurTelAttr.m_szBusiPriority,m_DataOut.values[6]);
		strcpy(m_CurTelAttr.m_szBusiBefore,m_DataOut.values[7]);
		strcpy(m_CurTelAttr.m_szSvrBefore,m_DataOut.values[8]);
		strcpy(m_CurTelAttr.m_szBTEmpty,m_DataOut.values[9]);
	}
	return 0;
}

int C_CallNumber::DealTollcode()
{
	strcpy(m_CurTelAttr.m_szAfterTollTel,m_CurTelAttr.m_szTeleno+strlen(m_CurTelAttr.m_szTollcode));
	return 0;
}

int C_CallNumber::DealGsm()
{
	char tmp[FIELD_LEN+1];
	strcpy(tmp,m_CurTelAttr.m_szTeleno);
	if (IsZeroHead())
	{
		strcpy(tmp,m_CurTelAttr.m_szTeleno+1);
	}
	strcpy(m_CurTelAttr.m_szTeleno,tmp);	
	return 0;
}

int C_CallNumber::AddZero()
{
	char tmpnbr[FIELD_LEN+1];
	strcpy(tmpnbr,"0");
	strcat(tmpnbr,m_CurTelAttr.m_szTeleno);
	strcpy(m_CurTelAttr.m_szTeleno,tmpnbr);
	return 0;
}

int C_CallNumber::AddDefTollcode()
{
	char tmpnbr[FIELD_LEN+1];
	strcpy(tmpnbr,m_InputParam.m_szLocalToll);
	if (m_CurTelAttr.m_szTeleno[0]=='0')
		strcat(tmpnbr,m_CurTelAttr.m_szTeleno+1);
	else
		strcat(tmpnbr,m_CurTelAttr.m_szTeleno);
	strcpy(m_CurTelAttr.m_szTeleno,tmpnbr);
	return 0;
}

int C_CallNumber::setDefTollcode()
{
	strcpy(m_CurTelAttr.m_szTollcode,m_InputParam.m_szLocalToll);
	return 0;
}


int C_CallNumber::ConnectTollGsm()
{
	char tmp[FIELD_LEN+1];
	strcpy(tmp,m_CurTelAttr.m_szTeleno);
	strcpy(m_CurTelAttr.m_szTeleno,m_CurTelAttr.m_szTollcode);
	strcat(m_CurTelAttr.m_szTeleno,tmp);
	return 0;
}

/**************************************************************/
int C_CallNumber::queryTollcode(char * Lack_Code)
{
	m_DataIn.clear();
	m_DataIn.set(m_CurTelAttr.m_szTeleno);
	m_DataIn.itemNum = 1;
	strcpy(m_DataIn.startTime,m_InputParam.m_szCdrBegin);

	m_DataOut.clear();
	if(table->getData(m_iTableTollCodeOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		strcpy(m_CurTelAttr.m_szTollcode,m_DataOut.values[0]);
	}
	else
	{
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLING_TOLLCODE;    		
		}
  		else if( strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
  		{
  			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLED_TOLLCODE; 
  		}
		else 
		{
		  	sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CHARGE_TOLLCODE;   		
		}
	}
	return 0;
}

int C_CallNumber::queryGsm(char * Lack_Code)
{
	m_DataIn.clear();
	m_DataIn.set(m_CurTelAttr.m_szTeleno);
	m_DataIn.itemNum = 1;
	strcpy(m_DataIn.startTime,m_InputParam.m_szCdrBegin);

	m_DataOut.clear();
	if(table->getData(m_iTableGsm2TollcodeOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		strcpy(m_CurTelAttr.m_szTollcode,m_DataOut.values[3]);
		strcpy(m_CurTelAttr.m_szBusiAfter,m_DataOut.values[4]);
		strcpy(m_CurTelAttr.m_szMobile,m_DataOut.values[5]);
		return 0;
	}
	else
	{
	
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLING_GSM2TOLLCODE;    		
		}
  		else  if( strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)   		
  		{
  			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLED_GSM2TOLLCODE;   		

  		}
		else
		{
  			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CHARGE_GSM2TOLLCODE;   	
		}
	}
}

 int C_CallNumber::queryLocalnet(char * Lack_Code)
{
	 m_DataIn.clear();
	 m_DataIn.set(m_CurTelAttr.m_szTollcode);
	 m_DataIn.itemNum = 1;
	 strcpy(m_DataIn.startTime,m_InputParam.m_szCdrBegin);

	 m_DataOut.clear();
	 if(table->getData(m_iTableLocalnetOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	 {
		 strcpy(m_CurTelAttr.m_szLocalNet,m_DataOut.values[1]);
		 strcpy(m_CurTelAttr.m_szIsGDProvince,m_DataOut.values[2]);
		 strcpy(m_CurTelAttr.m_szCodeLength,m_DataOut.values[3]);
		 return 0;
	 }
	 else
	 {
		 if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)
		 {
			 sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTollcode);
			 return LACK_CALLING_TOLLCODE;  
		 }
		 else  if( strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)   	
		 {
			 sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTollcode);
			 return LACK_CALLED_TOLLCODE;   	
		 }
		 else
		 {
			 sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTollcode);
			 return LACK_CHARGE_TOLLCODE; 
		 }
	 }
 }
	
int C_CallNumber::queryTeleProperty(char * Lack_Code)
{
	m_DataIn.clear();
	m_DataIn.set(m_CurTelAttr.m_szTollcode, 0);
	m_DataIn.set(m_CurTelAttr.m_szTeleno, 0);
	m_DataIn.itemNum = 1;
	strcpy(m_DataIn.startTime,m_InputParam.m_szCdrBegin);

	m_DataOut.clear();
	if(table->getData(m_iTableTelenoProOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		strcpy(m_CurTelAttr.m_szDistrict,m_DataOut.values[5]);
		strcpy(m_CurTelAttr.m_szBusiAfter,m_DataOut.values[6]);
		strcpy(m_CurTelAttr.m_szMobile,m_DataOut.values[7]);
		return 0;
	}
	else
	{
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLING_TELENO_PROPERTY;    		
		}
  		else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
  		{
  			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLED_TELENO_PROPERTY;   		
  		}
		else
		{
		  	sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CHARGE_TELENO_PROPERTY;   	
		}
	}	
}

int C_CallNumber::queryTelDefProperty(char * Lack_Code)
{
	m_DataIn.clear();
	m_DataIn.set(m_InputParam.m_szServCatId, 0); 
	m_DataIn.set(m_CurTelAttr.m_szTelenoFlag, 0);
	m_DataIn.set(m_CurTelAttr.m_szTollcode, 0);
	m_DataIn.set(m_CurTelAttr.m_szTeleno, 0);
	m_DataIn.itemNum = 1;
	strcpy(m_DataIn.startTime, m_InputParam.m_szCdrBegin);

	m_DataOut.clear();
	if(table->getData(m_iTableTelDefProOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		strcpy(m_CurTelAttr.m_szDistrict,m_DataOut.values[6]);
		strcpy(m_CurTelAttr.m_szBusiAfter,m_DataOut.values[7]);
		strcpy(m_CurTelAttr.m_szMobile,m_DataOut.values[8]);

		return 0;
	}
	else
	{
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLING_TELENO_DEFPRO;    		
		}
  		else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
  		{
  			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLED_TELENO_DEFPRO;   		
  		}
		else
		{
		  	sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CHARGE_TELENO_DEFPRO;   	
		}
	}
}

int C_CallNumber::queryTeleServ(char * Lack_Code)
{
	m_DataIn.clear();
	m_DataIn.set(m_InputParam.m_szServCatId, 0);
	m_DataIn.set(m_CurTelAttr.m_szTelenoFlag, 0);
	m_DataIn.set(m_CurTelAttr.m_szTollcode, 0);
	m_DataIn.set(m_CurTelAttr.m_szTeleno, 0);
	m_DataIn.itemNum = 1;
	strcpy(m_DataIn.startTime,m_InputParam.m_szCdrBegin);

	m_DataOut.clear();
	if(table->getData(m_iTableTelenoSerOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		strcpy(m_CurTelAttr.m_szSvrAfter,m_DataOut.values[6]);
	}
	else
	{
		strcpy(m_CurTelAttr.m_szSvrAfter, m_DefServAfter);
	}	
	return 0;
}

int C_CallNumber::queryFreeTele()
{
	m_DataIn.clear();
	m_DataIn.set(m_InputParam.m_szServCatId, 0);
	m_DataIn.set(m_InputParam.m_szLocalToll, 0);
	m_DataIn.set(m_CurTelAttr.m_szTelenoFlag, 0);
	m_DataIn.set(m_CurTelAttr.m_szChargeFlag, 0);
	m_DataIn.set(m_CurTelAttr.m_szTeleno, 0);
	m_DataIn.itemNum = 1;

	m_DataOut.clear();
	if(table->getData(m_iTableFreeTelOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		int iHeader = atoi(m_DataOut.values[4]);

		//2-8��ͷ�ļƷѺ�������ڱ�V_FREE_LACKHEADER���м�¼���ж�Ϊ�����ϻ���
		if(iHeader > 1 && iHeader < 9)
		{
			m_DataIn.clear();
			m_DataIn.set(m_InputParam.m_szServCatId, 0);
			m_DataIn.set(m_InputParam.m_szLocalToll, 0);
			m_DataIn.set(m_CurTelAttr.m_szTeleno, 0);
			m_DataIn.itemNum = 1;
			strcpy(m_DataIn.startTime,m_InputParam.m_szCdrBegin);

			memset(&m_DataOut,0,sizeof(m_DataOut));
			if(table->getData(m_iTableFreeLackOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
			{
				return 1;
			}
		}
		return 0;
	}
	return 1;
}

int C_CallNumber::queryTelenoLen(int &Len)
{
	m_DataIn.clear();
	m_DataIn.set(m_CurTelAttr.m_szTollcode, 0);
	m_DataIn.set(m_CurTelAttr.m_szTeleno, 0);
	m_DataIn.itemNum = 1;

	m_DataOut.clear();
	if(table->getData(m_iTableTelenoLenOffset, &m_DataIn, &m_DataOut, m_iIndex)==0)
	{
		Len=atoi(m_DataOut.values[2]);
		return 0;
	}
	Len=atoi(m_CurTelAttr.m_szCodeLength);
	return 1;
}

/**************************************************************/

//�ֻ��ŷ���1�����ֻ��ŷ���0
int C_CallNumber::IsGsm()
{
	if(1==Gsm2Toll.isGsm(m_CurTelAttr.m_szTeleno))
		return 1;
	return 0;
}

int C_CallNumber::IsZeroHead()
{
	if (m_CurTelAttr.m_szTeleno[0]=='0')
		return 1;
	return 0;
}

int C_CallNumber::IsEmpty()
{
	if (strlen(m_CurTelAttr.m_szTeleno)==0)
		return 1;
	return 0;
}
/*
int C_CallNumber::IsTollcodeOmit()
{
	CBindSQL ds( DBConn );
	if( strcmp( m_InputParam.m_szSourceId, m_szLastSourceId ) )
	{
		strcpy( m_szLastSourceId, m_InputParam.m_szSourceId );
	
		// get tollcode omit_flag from source 
		char szOmitTollcode[FIELD_LEN+1];
		
		ds.Open("select tollcode_omit from SOURCE where source_id=:a and pipe_id=:b", SELECT_QUERY );
		ds<<m_InputParam.m_szSourceId<<m_InputParam.m_szPipeId;
		if ( !(ds>>szOmitTollcode) ) 
		{
			sprintf(ErrorMsg,"select tollcode_omit from SOURCE  error");
			printf("%s\n",ErrorMsg);
			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,ErrorMsg,__FILE__,__LINE__);
		}
		ds.Close();
		DeleteSpace( szOmitTollcode );
		m_iOmitTollcode = atoi( szOmitTollcode );
	}
	
	if ( m_iOmitTollcode)
		return 1;
	return 0;
}
*/
int C_CallNumber::IsDefTollProvince(char * Lack_Code)
{
	strcpy(m_CurTelAttr.m_szTollcode,m_InputParam.m_szLocalToll);
	
	if (queryLocalnet(Lack_Code))
	{
		return LACK_DEF_TOLLCODE;
	}
	if (strcmp(m_CurTelAttr.m_szIsGDProvince,"N"))
		return 1;
	
	return 0;
}

int C_CallNumber::IsTeleLengthRight()
{
	if (strlen(m_CurTelAttr.m_szTeleno)==atoi(m_CurTelAttr.m_szCodeLength))
		return 1;
	return 0;	
}

int C_CallNumber::IsDefTollHead()
{
	char tmp[FIELD_LEN+1];
	//ȥ��0��ͷ��Ĭ������
	strcpy(tmp,m_InputParam.m_szLocalToll+1);
	
	if (strncmp(m_CurTelAttr.m_szTeleno,tmp,strlen(tmp))==0)
		return 0;
	else
		return 1;
}

bool C_CallNumber::IsForeign()
{
	if (strncmp(m_CurTelAttr.m_szTollcode,"00",2)==0)
		return true;
	return false;
}

/**************************************************************/
//У��˫���ţ���ʱ���ʲ�У�飬ȥ��������0��ͷֱ����Ϊ˫����
int C_CallNumber::Check2Tollcode(char * Lack_Code)
{
	char tmpTeleno[FIELD_LEN+1];
	//�����ж�
	if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
	{
		//���ʱ������Ų���˫���� 
		if (strncmp(m_CurTelAttr.m_szTollcode,"00",2)==0)
			return 0;
		
		if (m_CurTelAttr.m_szTeleno[0]=='0' )
		{
			//˫���Ź���
			if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
			{
				//ʡ����0��ͷ�����в��ж�
				if(queryLocalnet(Lack_Code))
					return 0;
				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLING_OCCOR_2TOLL1;    		
			}
  			else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
  			{
  				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLED_OCCOR_2TOLL1;   		
  			}
			else
			{
			  	sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CHARGE_OCCOR_2TOLL1;   	
			}
		}
		return 0;
	}
	else
	{
		//�����жϣ����ʻ�������˫����	
		if (strncmp(m_CurTelAttr.m_szTollcode,"00",2)==0)
			return 0;
		//�˴��Ѿ���ȥ�����ź�ĺ���
		if(m_CurTelAttr.m_szTeleno[0]=='0' )
		{
			//˫���Ź���
			if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
			{
				//ʡ����0��ͷ�����в��ж�
				if(queryLocalnet(Lack_Code))
					return 0;
				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLING_OCCOR_2TOLL1;  
			}
  			else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
  			{
  				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLED_OCCOR_2TOLL1;   		
  			}
			else
			{
			  	sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CHARGE_OCCOR_2TOLL1;   	
			}
		}
		else
		{
			strcpy(tmpTeleno,m_CurTelAttr.m_szTeleno);
			AddZero();
			char tmpTollcodeA[FIELD_LEN+1];
			strcpy(tmpTollcodeA,m_CurTelAttr.m_szTollcode);
			if (0==queryTollcode(Lack_Code))
			{
				if(strcmp(tmpTollcodeA,m_CurTelAttr.m_szTollcode)!=0)
				{
					//ע��˴���Ҫ�������ݻ�ԭ
					strcpy(m_CurTelAttr.m_szTollcode,tmpTollcodeA);
					strcpy(m_CurTelAttr.m_szTeleno,tmpTeleno);
					return 0;
				}
				else 
				{
					//���Ҿֺ����ϱ�
					strcpy (m_CurTelAttr.m_szTollcode,tmpTollcodeA);
					strcpy (m_CurTelAttr.m_szTeleno,tmpTeleno);
					if(0==queryTeleProperty(Lack_Code))
					{
						//�оֺ�����
						return 0;
					}
					else
					{
						//˫���Ź���
						if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)
						{
							sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
							return ABN_CALLING_OCCOR_2TOLL1;  
						}
						else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
						{
							sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
							return ABN_CALLED_OCCOR_2TOLL1;  
						}
						else
						{
							sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
							return ABN_CHARGE_OCCOR_2TOLL1;  
						}
					}
				}
			}
			else
			{
				//ע��˴���Ҫ�������ݻ�ԭ
				strcpy(m_CurTelAttr.m_szTollcode,tmpTollcodeA);
				strcpy(m_CurTelAttr.m_szTeleno,tmpTeleno);
				return 0;
			}
		}
	}//�����ж�
}

/**************************************************************/
int C_CallNumber::CheckValid(int CheckLen,char * Lack_Code)
{
	/*�жϺ���β���Ƿ�����ַ�,�������,���ȥ�ַ���������*/
	int iLen;
	
	if ((CheckLen==0) ||(strlen(m_CurTelAttr.m_szTeleno)<=CheckLen ))
		iLen = strlen(m_CurTelAttr.m_szTeleno) - 1;
	else 
			iLen=CheckLen-1;
	
	for(int i=0;i<=iLen;i++)
	{
		if ( !isdigit(m_CurTelAttr.m_szTeleno[i]) )
		{
			strcpy(Lack_Code,m_CurTelAttr.m_szTeleno);			
			if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    		
			{
				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLING_DIGIT;    		
			}
  			else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
  			{
  				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLED_DIGIT;   		
  			}
			else
			{
			  	sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CHARGE_DIGIT;   	
			}
		}
	}
	return 0;
}


int C_CallNumber::CheckGsm(char * Lack_Code)
{
	char BefGsmToll[FIELD_LEN+1];
	strcpy(BefGsmToll,m_CurTelAttr.m_szTollcode);
	int ret=0;
	ret=queryGsm(Lack_Code);
	if (!ret)
	{	
		if (!strcmp(m_CurTelAttr.m_szTollcode,BefGsmToll))
			return 0;
		else
		{
			//�ֻ������ź�H���һ��
			//���У��ƷѺ���ȡ����ǰ����
			if( strcmp(m_CurTelAttr.m_szTelenoFlag, "10")!=0)   
			{
				strcpy(m_CurTelAttr.m_szTollcode,BefGsmToll);
				if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)    
				{
					sprintf(Lack_Code,"%s%s",BefGsmToll,m_CurTelAttr.m_szTeleno);
					return ABN_CALLING_GSMTOLLCODE; 
				}
				else
				{
					sprintf(Lack_Code,"%s%s",BefGsmToll,m_CurTelAttr.m_szTeleno);
					return ABN_CHARGE_GSMTOLLCODE;  
				}
			}
			else 
			{
				//13800138000�������⴦������ͨ����һ��
				sprintf(Lack_Code,"%s%s",m_CurTelAttr.m_szTollcode,m_CurTelAttr.m_szTeleno);
				return ABN_CALLED_GSMTOLLCODE;   
			}
		}
	}
	return ret;
}

int C_CallNumber::CheckGsmLen(char * Lack_Code)
{
	if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
	{
		//���к���
		if (m_CurTelAttr.m_szTeleno[0]!='0')
		{
			//���У���������>=
			if (strlen(m_CurTelAttr.m_szTeleno)>=m_iGsmLength)
				return 0;
			else
			{
				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLED_GSMLEN;  
			}
		}
		else
		{
			if ((strlen(m_CurTelAttr.m_szTeleno)-1)>=m_iGsmLength)
				return 0;	
			else
			{
				sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
				return ABN_CALLED_GSMLEN;   
			}
		}
	}
	else
	{
		//���У��ƷѺ���
		if (m_CurTelAttr.m_szTeleno[0]!='0')
		{
			//���У���������>=
			if (strlen(m_CurTelAttr.m_szTeleno)==m_iGsmLength)
				return 0;
			else
			{
				if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)  
				{
					sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
					return ABN_CALLING_GSMLEN;  
				}
				else
				{
					sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
					return ABN_CHARGE_GSMLEN; 
				}
			}
		}
		else
		{
			if ((strlen(m_CurTelAttr.m_szTeleno)-1)==m_iGsmLength)
				return 0;
			else
			{
				if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)  
				{
					sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
					return ABN_CALLING_GSMLEN;   
				}
				else
				{
					sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
					return ABN_CHARGE_GSMLEN;  
				}
			}
		}
	}
}

int C_CallNumber::CheckLen(char * Lack_Code )
{
	if (strlen(m_CurTelAttr.m_szTeleno)<MINTELEN)
	{
		strcpy(Lack_Code,m_CurTelAttr.m_szTeleno);
		
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)  
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return ABN_CALLING_SHORTLEN; 
		}
		else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
		{
			//�������ź����С��1λ����
			if (strncmp(m_CurTelAttr.m_szTollcode,"00",2)==0 && strlen(m_CurTelAttr.m_szTeleno)>0)
				return 0;
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return ABN_CALLED_SHORTLEN;  
		}
		else
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return ABN_CHARGE_SHORTLEN;
		}
	}
	return 0;
}

//���������ϱ�У����볤�ȣ��޸�ԭ�е�У�鷽ʽ
int C_CallNumber::CheckGDLen(char * Lack_Code)
{
	//ֻ�����У��ƷѺ�����г���У�飬����ֱ�ӷŹ�
	if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
		return 0;

	//ֻ�Թ㶫ʡ�ڵĺ������У��
	if (queryLocalnet( Lack_Code))
		return 0;

	//ֻ��2~8����У��
	if (m_CurTelAttr.m_szTeleno[0]=='1' || m_CurTelAttr.m_szTeleno[0]=='9')
		return 0;

	int checkLen;

	queryTelenoLen(checkLen);
	//���ã�1��ʾ������У��
	if (checkLen==-1) 
		return 0;
	
	if (strlen(m_CurTelAttr.m_szTeleno)==checkLen)
		return 0;
	
	if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0) 
	{
		sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
		return ABN_CALLING_GDLEN;
	}
	else
	{
		sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
		return ABN_CHARGE_GDLEN;  
	}
}

//����н���ţ����ҽ���ź���벻��0��ͷ�����쳣����
//0��ʾ��������1��ʾ�������Ϊ�ղ��ҿ���Ϊ�գ�>0 ��ʾ�쳣����
int C_CallNumber::CheckBefTel(char * Lack_Code)
{
	if (strcmp(m_CurTelAttr.m_szBefTollTel,"")!=0)
	{
		//�н����룬У�����������
		if (IsZeroHead())	
			return 0;

		if ( IsEmpty())    
		{
			//���������Ϊ��
			if (m_CurTelAttr.m_szBTEmpty[0]=='1')
				return -1;
		}
		
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0)
		{
			sprintf(Lack_Code,"%s%s",m_CurTelAttr.m_szBefTollTel,m_CurTelAttr.m_szTeleno);
			return ABN_CALLING_BEFTEL;  
		}
		else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
		{
			sprintf(Lack_Code,"%s%s",m_CurTelAttr.m_szBefTollTel,m_CurTelAttr.m_szTeleno);
			return ABN_CALLED_BEFTEL;  
		}
		else
		{
			sprintf(Lack_Code,"%s%s",m_CurTelAttr.m_szBefTollTel,m_CurTelAttr.m_szTeleno);
			return ABN_CHARGE_BEFTEL;  
		}
	}
	else 
		return 0;
}


int C_CallNumber::Check10Len(char * Lack_Code )
{
	if (strlen(m_CurTelAttr.m_szTeleno)<10)
	{
		strcpy(Lack_Code,m_CurTelAttr.m_szTeleno);
		
		if( strcmp(m_CurTelAttr.m_szTelenoFlag, "01")==0) 
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLING_TOLLCODE; 
		}
		else if (strcmp(m_CurTelAttr.m_szTelenoFlag, "10")==0)
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CALLED_TOLLCODE;
		}
		else
		{
			sprintf(Lack_Code,"%s",m_CurTelAttr.m_szTeleno);
			return LACK_CHARGE_TOLLCODE; 
		}
	}
	return 0;
}


int C_CallNumber::loadDefTelenoAttr()
{
	getFromSrcEnv( m_DefMobile, 
					"DEFAULT_MOBILE",
					m_InputParam.m_szSourceId,
					m_InputParam.m_szServiceId );
							
	getFromSrcEnv( m_DefBusiness, 
					"DEFAULT_BUSINESS",
					m_InputParam.m_szSourceId,
					m_InputParam.m_szServiceId );
	getFromSrcEnv( m_DefServBefore, 
					"DEFAULT_SVRBEFORE",
					m_InputParam.m_szSourceId,
					m_InputParam.m_szServiceId );
	getFromSrcEnv( m_DefServAfter, 
					"DEFAULT_SERVICE",
					m_InputParam.m_szSourceId,
					m_InputParam.m_szServiceId );
		
	return 0;
}

/*
int C_CallNumber::UpdateTable()
{
	if(table_DelTelHead != NULL)
		table_DelTelHead->reBind();
	if(table_BefTolSer != NULL)
		table_BefTolSer->reBind();
	if(table_Gsm2Tollcode != NULL)
		table_Gsm2Tollcode->reBind();
	if(table_TelenoPro != NULL)
		table_TelenoPro->reBind();
	if(table_TelenoSer != NULL)
		table_TelenoSer->reBind();
	if(table_TollCode != NULL)
		table_TollCode->reBind();
	if(table_Localnet != NULL)
		table_Localnet->reBind();
	if(table_FreeTel != NULL)
		table_FreeTel->reBind();
	if(table_TelenoLen != NULL)
		table_TelenoLen->reBind();
	if(table_TelDefPro != NULL)
		table_TelDefPro->reBind();
	if(table_FreeLack != NULL)
		table_FreeLack->reBind();

	return 0;
}
*/
