#include "SjywAnaVPDN.h"

/*************************************************
* ����    �����캯����ʼ�������ṹ��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_SjywAnaVPDN::C_SjywAnaVPDN()
{
        memset(&m_InData, 0, sizeof(m_InData));
        memset(&m_OutData, 0, sizeof(m_OutData));
        table = NULL;
}


/*************************************************
* ����    ����������
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
C_SjywAnaVPDN::~C_SjywAnaVPDN()
{
}


/*************************************************
* ����    ����ʼ����������ʼ����Ҫ��ȡ���õĲ���
* ��ڲ�������
* ���ڲ�������
* ����    ��0
**************************************************/         
void C_SjywAnaVPDN::init(char *szSourceGroupID, char *szServiceID, int index) 
{
        theJSLog<<"==========C_SjywAnaVPDN class begin=========="<<endd;
        table = getMemPoint();
        if(table == NULL)
                throw(ERR_SHM_ERROR, "pAccessMem is null", __FILE__, __LINE__);
        strcpy(m_szTableName, "I_SJYW_VPDN_AREAINFO");
        m_iTableOffset = table->getTableOffset(m_szTableName);
        m_iIndex = 1;
}

/*************************************************
 *description:
 *       ƥ��VPDNҵ��Ҫ������һ����ʽ
         �û���@�ͻ�����.133vpdn.��������	
         �û���@�ͻ�����.vpdn.��������
         1��NAIֵ�����ʽ�����û���@�ͻ�����.133vpdn.�������������û���@�ͻ�����.vpdn.������������					
         2�����û�����������20 ���ַ���ֻ��Ϊ��ĸ�����ֺ��»��ߡ�_����				
         3�����ͻ�������������20 ���ַ���ֻ��Ϊ����ĸ�����ֺ��»��ߡ�_����					
         4�����������������������С������������д��ڡ�					
 *input:
 *      ������NAIID��
        ����Ҫ��ʱ�����ֵ��6��
        ������Ҫ��ʱ�����ֵ��0��
 *output:
 *      ������Ҫ�����NAITYPEֵ6���������0
**************************************************/  
void C_SjywAnaVPDN::execute(PacketParser& pps,ResParser& retValue)
{
	 char ErrorMsg[ERROR_MSG_LEN+1];
	 if ( pps.getItem_num() != 5 )
		{
			sprintf( ErrorMsg,"���SjywAnaVPDN�������������ȷ!");
			throw jsexcp::CException(ERR_LACK_PARAM,(char *)ErrorMsg,(char *)__FILE__,__LINE__);
		}
		
		char szSourceId[RECORD_LENGTH];
		char szValue[RECORD_LENGTH];
		char szStartTime[RECORD_LENGTH];
		char szOK[RECORD_LENGTH];
		char szNOK[RECORD_LENGTH];
		
	
	  memset( szSourceId, 0, sizeof( szSourceId ) );
		pps.getFieldValue( 1, szSourceId );
		delSpace( szSourceId, 0 );
		
		memset( szValue, 0, sizeof( szValue ) );
		pps.getFieldValue( 2, szValue );
		delSpace( szValue, 0 );


		memset( szStartTime, 0, sizeof( szStartTime ) );
		pps.getFieldValue( 3, szStartTime );
		delSpace( szStartTime, 0 );
		
    memset( szOK, 0, sizeof( szOK ) );
		pps.getFieldValue( 4, szOK );
		delSpace( szOK, 0 );
		
		memset( szNOK, 0, sizeof( szNOK ) );
		pps.getFieldValue( 5, szNOK );
		delSpace( szNOK, 0 );
		
		if(CheckVPDN(szSourceId, szValue, szStartTime)==0)
			retValue.setFieldValue( 1, szOK, strlen( szOK ) );	
		else
			retValue.setFieldValue( 1, szNOK, strlen( szNOK ) );	
			
}

int C_SjywAnaVPDN::CheckVPDN(char *sourceId, char *username,char *starttime){
	  if(username==NULL){
	  	//printf("username is null\n");
	  	return 1;
	  }
	  
	  char sepChar = '@';
	  int charLength = strlen(username);
	  
	  //printf("charLength is %d\n",charLength);
	  
	  int i=0;
	  for(;i<charLength;i++){
	  	 if(username[i]==sepChar){
	  	 	  //printf("ok\n");
	  	    break;
	  	  }
	  }
	  
	  char szValue[100];
	  
	  //printf("i=%d\n",i);
	  if(i<charLength){
	      memset( szValue, 0, sizeof( szValue ) );
				memcpy( szValue, username, i );
				szValue[i]='\0';
	//			printf("%s\n",szValue);
				//У���Ƿ�С��20���ַ�
				if(strlen(szValue)<=20){
					 for(int j=0;j<strlen(szValue);j++){
					 	//�ж��Ƿ���ȫ���ֻ���ĸ��'_'
					 		if(isalnum(szValue[j])!=0 || szValue[j]=='_')
					 			;
					 		else{
					 			//printf("not digit or char %c",szValue[j]);
					 			return 1;
					 		}
					 }
					 //----------------------------------------------------------
					 //У��ͻ�����
					 char guestName[100];
					 memset( guestName, 0, sizeof( guestName ) );
					 memcpy( guestName, username+i+1, strlen(username)-i-1 );
					 guestName[strlen(username)-i-1]='\0';
	//				 printf("guestName=%s  %d\n",guestName,strlen(username)-i-1);
					 
					 sepChar = '.';
					 charLength = strlen(guestName);
					 for(i=0;i<charLength;i++){
					  	 if(guestName[i]==sepChar){
					  	 	  //printf("ok\n");
					  	    break;
					  	 }
			  	 }
					 
					 if(i<charLength){
				      memset( szValue, 0, sizeof( szValue ) );
							memcpy( szValue, guestName, i );
							szValue[i]='\0';
	//						printf("%s\n",szValue);
							//У���Ƿ�С��20���ַ�
							if(strlen(szValue)<=20){
								 for(int j=0;j<strlen(szValue);j++){
								 	//�ж��Ƿ���ȫ���ֻ���ĸ��'_'
								 		if(isalnum(szValue[j])!=0 || szValue[j]=='_')
								 			;
								 		else{
								 			//printf("not digit or char %c",szValue[j]);
								 			return 1;
								 		}
								 }
								 //�ͻ�����У��ɹ�����ʼУ��vpdn����
								 //----------------------------------------------------------
								 //printf("guestName check success!\n");
								 
								 char vpdnValue[100];
								 memset( vpdnValue, 0, sizeof( vpdnValue ) );
								 memcpy( vpdnValue, guestName+i+1, strlen(guestName)-i-1 );
								 vpdnValue[strlen(guestName)-i-1]='\0';
	//							 printf("vpdnValue=%s  %d\n",vpdnValue,strlen(guestName)-i-1);
								 
								 sepChar = '.';
								 charLength = strlen(vpdnValue);
								 for(i=0;i<charLength;i++){
								  	 if(vpdnValue[i]==sepChar){
								  	 	  //printf("ok\n");
								  	    break;
								  	 }
						  	 }
								 
								 if(i<charLength){
							      memset( szValue, 0, sizeof( szValue ) );
										memcpy( szValue, vpdnValue, i );
										szValue[i]='\0';
										//printf("%s\n",szValue);
								 }else
								 		return 1;
								 
								 //У���Ƿ�Ϊ133vpdn��vpdn,�����ִ�Сд
								 char szValueTmp[100];
								 memset( szValueTmp, 0, sizeof( szValueTmp ) );
								 memcpy( szValueTmp, szValue, strlen(szValue) );
								 szValueTmp[i]='\0';
								 for(int i=0;i<strlen(szValueTmp);i++){
								    if(isupper(szValueTmp[i]))
								    		szValueTmp[i] = tolower(szValueTmp[i]);
								 }
								 
						 		 if(strcmp(szValueTmp,"133vpdn")==0||strcmp(szValueTmp,"vpdn")==0)
						 		 	 ;
						 		 else{
						 			 //printf("not compare %s",szValue);
						 			 return 1;
						 		 }
									 
								 //�ͻ�����У��ɹ�����ʼУ������������������Сд
								 //----------------------------------------------------------
								 char areaCode[100];
								 memset( areaCode, 0, sizeof( areaCode ) );
								 memcpy( areaCode, vpdnValue+i+1, strlen(vpdnValue)-i-1 );
								 areaCode[strlen(vpdnValue)-i-1]='\0';
								 //printf("areaCode=%s  %d\n",areaCode,strlen(vpdnValue)-i-1);
								 
								 char szValueTmp2[100];
								 memset( szValueTmp2, 0, sizeof( szValueTmp2 ) );
								 memcpy( szValueTmp2, areaCode, strlen(areaCode) );
								 szValueTmp2[i]='\0';
								 for(int i=0;i<strlen(szValueTmp2);i++){
								    if(isupper(szValueTmp2[i]))
								    		szValueTmp2[i] = tolower(szValueTmp2[i]);
								 }
								 
								 if(CheckAreaCode(sourceId,szValueTmp2,starttime)!=0)
								 	   return 1;
								 	   
								 return 0;
							}
					 }
				}
	  }
	  
	  //printf("not ok\n");
	  	 
	  return 1;
}

int C_SjywAnaVPDN::CheckAreaCode(char *sourceId, char *areacode,char *starttime){
	  if(areacode==NULL)
			return 1;
	
		char startTime[14];
		memset(startTime,0,14);
		memcpy(startTime,starttime,14);
		startTime[14]='\0';
		
		
		m_InData.clear();
    m_InData.set(areacode);
    m_InData.itemNum = 1;
    strcpy(m_InData.startTime, starttime);
    if( table->getData(m_iTableOffset, &m_InData, &m_OutData, m_iIndex) == 0 ){
    	return 0;
    }
	
	return 1;
}

/*************************************************
* ����    ��������Ϣ����
* ��ڲ�������Ϣ��
* ���ڲ�������
* ����    ����
**************************************************/
void C_SjywAnaVPDN::message(MessageParser&  pMessage)
{
}

/*************************************************
* ����    ����ӡ����汾��
* ��ڲ�������
* ���ڲ�������
* ����    ����
**************************************************/
void C_SjywAnaVPDN::printMe()
{
        printf("\t�������:SjywAnaVPDN,�汾�ţ�3.0.0 \n");
}

/*************************************************
* ����    ����ӡ�������
* ��ڲ�������
* ���ڲ�������
* ����    ���������
**************************************************/
std::string C_SjywAnaVPDN::getPluginName()
{
        return "SjywAnaVPDN";
}

std::string C_SjywAnaVPDN::getPluginVersion(){
        return "3.0.0";
}


