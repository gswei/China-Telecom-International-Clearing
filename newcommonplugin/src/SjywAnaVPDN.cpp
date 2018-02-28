#include "SjywAnaVPDN.h"

/*************************************************
* 描述    ：构造函数初始化两个结构体
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
C_SjywAnaVPDN::C_SjywAnaVPDN()
{
        memset(&m_InData, 0, sizeof(m_InData));
        memset(&m_OutData, 0, sizeof(m_OutData));
        table = NULL;
}


/*************************************************
* 描述    ：析构函数
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
C_SjywAnaVPDN::~C_SjywAnaVPDN()
{
}


/*************************************************
* 描述    ：初始化函数，初始化需要读取表用的参数
* 入口参数：无
* 出口参数：无
* 返回    ：0
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
 *       匹配VPDN业务，要求满足一下形式
         用户名@客户域名.133vpdn.地区域名	
         用户名@客户域名.vpdn.地区域名
         1、NAI值满足格式：“用户名@客户域名.133vpdn.地区域名”或“用户名@客户域名.vpdn.地区域名”；					
         2、“用户名”不超过20 个字符，只能为字母、数字和下划线“_”；				
         3、“客户域名”不超过20 个字符，只能为用字母、数字和下划线“_”；					
         4、“地区域名”必须在下列“地区域名表”中存在。					
 *input:
 *      完整的NAIID、
        符合要求时的输出值（6）
        不符合要求时的输出值（0）
 *output:
 *      若符合要求，输出NAITYPE值6，否则输出0
**************************************************/  
void C_SjywAnaVPDN::execute(PacketParser& pps,ResParser& retValue)
{
	 char ErrorMsg[ERROR_MSG_LEN+1];
	 if ( pps.getItem_num() != 5 )
		{
			sprintf( ErrorMsg,"插件SjywAnaVPDN的输入参数不正确!");
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
				//校验是否小于20个字符
				if(strlen(szValue)<=20){
					 for(int j=0;j<strlen(szValue);j++){
					 	//判断是否是全数字或字母或'_'
					 		if(isalnum(szValue[j])!=0 || szValue[j]=='_')
					 			;
					 		else{
					 			//printf("not digit or char %c",szValue[j]);
					 			return 1;
					 		}
					 }
					 //----------------------------------------------------------
					 //校验客户域名
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
							//校验是否小于20个字符
							if(strlen(szValue)<=20){
								 for(int j=0;j<strlen(szValue);j++){
								 	//判断是否是全数字或字母或'_'
								 		if(isalnum(szValue[j])!=0 || szValue[j]=='_')
								 			;
								 		else{
								 			//printf("not digit or char %c",szValue[j]);
								 			return 1;
								 		}
								 }
								 //客户域名校验成功，开始校验vpdn域名
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
								 
								 //校验是否为133vpdn和vpdn,不区分大小写
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
									 
								 //客户域名校验成功，开始校验地区域名，不区别大小写
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
* 描述    ：处理消息函数
* 入口参数：消息类
* 出口参数：无
* 返回    ：无
**************************************************/
void C_SjywAnaVPDN::message(MessageParser&  pMessage)
{
}

/*************************************************
* 描述    ：打印插件版本号
* 入口参数：无
* 出口参数：无
* 返回    ：无
**************************************************/
void C_SjywAnaVPDN::printMe()
{
        printf("\t插件名称:SjywAnaVPDN,版本号：3.0.0 \n");
}

/*************************************************
* 描述    ：打印插件名称
* 入口参数：无
* 出口参数：无
* 返回    ：插件名称
**************************************************/
std::string C_SjywAnaVPDN::getPluginName()
{
        return "SjywAnaVPDN";
}

std::string C_SjywAnaVPDN::getPluginVersion(){
        return "3.0.0";
}


