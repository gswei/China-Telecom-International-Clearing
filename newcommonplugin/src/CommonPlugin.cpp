#include "CommonPlugin.h"

//static char ErrorMsg[ERROR_MSG_LEN + 1];

//判断字符串是否是数字型
//是返回true
//否返回false;
bool isNo(const char * array) {
	int length;
	if (array == NULL || (length = strlen(array)) == 0) {
		return false;
	}

	if (array[0] != '-' && (array[0] > '9' || array[0] < '0')) {
		return false;
	}

	bool isDotHadBefore = false; //逗号是否出现过

	for (int i = 1; i < length; i++) {
		if (array[i] > '9' || array[i] < '0') {
			if (array[i] == '.' && !isDotHadBefore) {
				isDotHadBefore = true;
			} else {
				return false;
			}
		}
	}
	return true;
}

my_time::my_time() {
	year = 0;
	month = 0;
	day = 0;
	hour = 0;
	minute = 0;
	sec = 0;
}

bool my_time::init(string t_time) {
	year = atoi(t_time.substr(0, 4).c_str());
	month = atoi(t_time.substr(4, 2).c_str());
	day = atoi(t_time.substr(6, 2).c_str());
	hour = atoi(t_time.substr(8, 2).c_str());
	minute = atoi(t_time.substr(10, 2).c_str());
	sec = atoi(t_time.substr(12, 2).c_str());
	//赋值之后还是测试一下传进来的时间对不对，如果不对就抛出异常
	for (int i = 0; i < t_time.size(); i++) {
		if (t_time[i] > '9' || t_time[i] < '0') {
			sprintf(
					ErrorMsg,
					"the time =%s= sent to TimeOperator have char like 20a11230145630",
					t_time.c_str());
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}
	}
	if (sec >= 60 || sec < 0 || minute >= 60 || minute < 0 || hour >= 24
			|| hour < 0) {
		sprintf(
				ErrorMsg,
				"the time =%s= sent to TimeOperator is error like 20011231246060",
				t_time.c_str());
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8
			|| month == 10 || month == 12) {
		if (day > 31 || day < 0) {
			sprintf(
					ErrorMsg,
					"the time =%s= sent to TimeOperator is error like 20011033000000",
					t_time.c_str());
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}
	} else if (month == 4 || month == 6 || month == 9 || month == 11) {
		if (day > 30 || day < 0) {
			sprintf(
					ErrorMsg,
					"the time =%s= sent to TimeOperator is error like 20011131000000",
					t_time.c_str());
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}
	} else if (month == 2) {
		if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) {
			if (day > 29 || day < 0) {
				sprintf(
						ErrorMsg,
						"the time =%s= sent to TimeOperator is error like 200402301000000",
						t_time.c_str());
				throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
						(char *) __FILE__, __LINE__);
			}
		} else {
			if (day > 28 || day < 0) {
				sprintf(
						ErrorMsg,
						"the time =%s= sent to TimeOperator is error like 200202291000000",
						t_time.c_str());
				throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
						(char *) __FILE__, __LINE__);
			}
		}
	} else {
		sprintf(
				ErrorMsg,
				"the time =%s= sent to TimeOperator is error like 200415301000000",
				t_time.c_str());
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	return true;
}

void BaseCommonPlugin::init(char *szSourceGroupID, char *szServiceID, int index) {
}

void BaseCommonPlugin::message(MessageParser& pMessage) {
}

/* Length插件  */
CLength::CLength() {
}

CLength::~CLength() {
	zhjs::CommonPluginFactory::dispose(this);
}

void CLength::printMe() {
	printf("\t插件名称:Length,版本号：3.0.0 \n");
}

std::string CLength::getPluginName() {
	return "Length";
}

std::string CLength::getPluginVersion(){
	return "3.0.0";
}

void CLength::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 1) {
		sprintf(ErrorMsg, "the params sent to Length is Error");
		throw jsexcp::CException(0, ErrorMsg, __FILE__, __LINE__);
	}

	char buff[RECORD_LENGTH + 1];
	memset(buff, 0, RECORD_LENGTH + 1);
	pps.getFieldValue(1, buff);
	int length = strlen(buff);

	sprintf(buff, "%d", length);
	retValue.setFieldValue(1, buff, strlen(buff));
	return;
}

/* SubString插件 */

SubString::SubString() {
}

SubString::~SubString() {
	zhjs::CommonPluginFactory::dispose(this);
}

void SubString::printMe() {
	printf("\t插件名称:SubString,版本号：3.0.0 \n");
}

std::string SubString::getPluginName() {
	return "SubString";
}

std::string SubString::getPluginVersion(){
	return "3.0.0";
}

//20120918 增加sub的截取类型(value，pos1，char1，pos2，char2)
//表示从value值中从第pos1个满足char1字符开始，到第pos2个满足char2字符为止
//截取中间部分的字符
//如A:123:456|B:222:333|C:444:555|D:666:777|E:0:0，pos1=2，char1=":",pos2=1,char2="|"
//截取的字符为456
//当pos1=0时，从value起始位置开始截取，当pos2=0时，截取到value末尾
void SubString::execute(PacketParser& pps, ResParser& retValue) {
	char szValue[RECORD_LENGTH + 1]={0};
	char szStrCmp[RECORD_LENGTH + 1]={0};
	int iPos=0;
        int iPos2=0;
	int iStrLen=0;
	int iLen=0;
	char sztmp[RECORD_LENGTH + 1]={0};
        char sztmp2[RECORD_LENGTH + 1]={0};
	int iBegin=0;
        int iBegin2=0;

        char szChar[RECORD_LENGTH + 1]={0};
        char szChar2[RECORD_LENGTH + 1]={0};

	if ((pps.getItem_num() < 2) || (pps.getItem_num() > 5)) {
		sprintf(ErrorMsg, "the params sent to SubString Error!");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	memset(szValue, 0, sizeof(szValue));
	pps.getFieldValue(1, szValue);
	delSpace(szValue, 0);
	iStrLen = strlen(szValue);

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(2, sztmp);
	delSpace(sztmp, 0);
	iPos = atoi(sztmp);

	if (iPos >= 0) {
		if (iPos > iStrLen)
			iPos = iStrLen;

		if (iPos > 0)
			iPos--;

		iBegin = iPos;
	} else {
		iBegin = iStrLen + iPos;
		if (iBegin < 0)
			iBegin = 0;
	}

	if (pps.getItem_num() == 2) {
		strcpy(sztmp, szValue + iBegin);
		retValue.setFieldValue(1, sztmp, strlen(sztmp));
	} else if (pps.getItem_num() == 3) {
		memset(sztmp, 0, sizeof(sztmp));
		pps.getFieldValue(3, sztmp);
		delSpace(sztmp, 0);
		iLen = atoi(sztmp);

		if ((iLen + iBegin) > iStrLen)
			iLen = iStrLen - iBegin;

		memcpy(sztmp, szValue + iBegin, iLen);
		sztmp[iLen] = '\0';
		retValue.setFieldValue(1, sztmp, strlen(sztmp));
	} else if (pps.getItem_num() == 4) {
		memset(szStrCmp, 0, sizeof(szStrCmp));
		pps.getFieldValue(3, szStrCmp);
		delSpace(szStrCmp, 0);
		char *p1 = NULL;
		char *p2 = NULL;
		p1 = szValue + iBegin;
		p2 = strstr(p1, szStrCmp);
		if (p2 == NULL) {
			sprintf(ErrorMsg, "No find %s in %s!", szStrCmp, p1);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}
		*p2 = '\0';
		strcpy(sztmp, p1);
		retValue.setFieldValue(1, sztmp, strlen(sztmp));
	}else if (pps.getItem_num() == 5) {
        //20120918 vivi add for cutting word between diff charactor in diff special position
        	memset(sztmp, 0, sizeof(sztmp));
	        pps.getFieldValue(2, sztmp);
	        delSpace(sztmp, 0);
	        iPos = atoi(sztmp);
				        
	        memset(szChar, 0, sizeof(szChar));
                pps.getFieldValue(3, szChar);
                delSpace(szChar, 0);
                
	        memset(sztmp, 0, sizeof(sztmp));
	        pps.getFieldValue(4, sztmp);
	        delSpace(sztmp, 0);
	        iPos2 = atoi(sztmp);
				        
	        memset(szChar2, 0, sizeof(szChar2));
                pps.getFieldValue(5, szChar2);
                delSpace(szChar2, 0);
                
                char *p1 = NULL;
	        char *p2 = NULL;
	        char *p3 = NULL;
	        char *p = NULL;
				    
		p = szValue;
		p1 = p;
		//cout<<"for first pos:"<<szChar<<endl;
		memset(sztmp, 0, sizeof(sztmp));
		if(iPos==0)
		    strcpy(sztmp, p1);
		else
		    for(int i=0;i<iPos;i++){
		        p2 = strstr(p1, szChar);
		        if(p2==NULL){
		        	sprintf(ErrorMsg, "No find first %s in %s pos %d!", szChar, p1, iPos);
                                throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
                                        (char *) __FILE__, __LINE__);
			 }
		        p2++;
		        strcpy(sztmp, p2);
		        p1 = sztmp;
		        p2 = NULL;
		        //cout<<sztmp<<endl;
		    }
						
		    //cout<<"for second pos:"<<szChar2<<endl;
		 
                 p1 = p;
		 memset(sztmp2, 0, sizeof(sztmp2));
                 if(iPos2 !=0){
		 for(int i=0;i<iPos2;i++){
		        p3 = strstr(p1, szChar2);
		        if(p3==NULL){
		        	sprintf(ErrorMsg, "No find %s in %s pos %d!", szChar2, p1, iPos2);
                                throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
                                        (char *) __FILE__, __LINE__);
			 }
			 p3++; 
			 strcpy(sztmp2, p3);
			 p1 = sztmp2;
			 p3 = NULL;
			 //cout<<sztmp2<<endl;
		}
                }
						
	        iBegin = iStrLen - strlen(sztmp);
		if(iPos2==0)
		      iBegin2 = iStrLen - iBegin;
		else
		      iBegin2 = iStrLen - strlen(sztmp2) - iBegin - 1 ;
		
		    //cout<<iStrLen<<endl;
		    //cout<<sztmp<<endl;
		    //cout<<sztmp2<<endl;
		    //cout<<iBegin<<endl;
		    //cout<<iBegin2<<endl;

		if(iBegin2<=0){
		     	  sprintf(ErrorMsg, "pos %d for char %s is wrong", iPos2,szChar2);
                          throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
                                        (char *) __FILE__, __LINE__);
	        }

         	memset(sztmp,0,strlen(sztmp));
		memcpy(sztmp, szValue+iBegin, iBegin2);
						    
		sztmp[iBegin2]='\0';
                retValue.setFieldValue(1, sztmp, strlen(sztmp));
				        
        }
}

/* Case插件 */
CCase::CCase() {
}

CCase::~CCase() {
	zhjs::CommonPluginFactory::dispose(this);
}

void CCase::printMe() {
	printf("\t插件名称:Case,版本号：3.0.0 \n");
}

std::string CCase::getPluginName() {
	return "Case";
}

std::string CCase::getPluginVersion(){
	return "3.0.0";
}

void CCase::execute(PacketParser& pps, ResParser& retValue) {
	m_iParamNum = pps.getItem_num();
	if (m_iParamNum % 2 == 1) {
		sprintf(ErrorMsg, "the params sent to CCase is lack");
		throw jsexcp::CException(0, (char *) ErrorMsg, (char *) __FILE__, __LINE__);
	}

	char szFieldValue[RECORD_LENGTH + 1];
	memset(szFieldValue, 0, sizeof(szFieldValue));
	pps.getFieldValue(1, szFieldValue);
	delSpace(szFieldValue, 0);

	char szCondition[RECORD_LENGTH + 1];
	char szResult[RECORD_LENGTH + 1];
	int i;

	for (i = 1; i < m_iParamNum - 2; i = i + 2) {
		memset(szCondition, 0, sizeof(szCondition));
		pps.getFieldValue(i + 1, szCondition);
		delSpace(szCondition, 0);

		if (!strcmp(szFieldValue, szCondition)) {
			memset(szResult, 0, sizeof(szResult));
			pps.getFieldValue(i + 2, szResult);
			delSpace(szResult, 0);
			retValue.setFieldValue(1, szResult, strlen(szResult));
			break;
		}
	}

	if (i > m_iParamNum - 2) {
		pps.getFieldValue(m_iParamNum, szResult);
		delSpace(szResult, 0);
		retValue.setFieldValue(1, szResult, strlen(szResult));
	}

}

/* Connect */
CConnect::CConnect() {
}

CConnect::~CConnect() {
	zhjs::CommonPluginFactory::dispose(this);
}

void CConnect::printMe() {
	printf("\t插件名称:Connect,版本号：3.0.0 \n");
}

std::string CConnect::getPluginName() {
	return "Connect";
}

std::string CConnect::getPluginVersion(){
	return "3.0.0";
}

void CConnect::execute(PacketParser& pps, ResParser& retValue) {
	m_iParamNum = pps.getItem_num();
	if (m_iParamNum < 1) {
		sprintf(ErrorMsg, "the params sent to Connect is lack");
		throw jsexcp::CException(0, (char *) ErrorMsg, (char *) __FILE__, __LINE__);
	}

	char szAfterConnect[RECORD_LENGTH + 1];
	memset(szAfterConnect, 0, sizeof(szAfterConnect));
	char szFieldValue[RECORD_LENGTH + 1];
	for (int i = 0; i < m_iParamNum; i++) {
		memset(szFieldValue, 0, sizeof(szFieldValue));
		pps.getFieldValue(i + 1, szFieldValue);
		delSpace(szFieldValue, 0);
		strcat(szAfterConnect, szFieldValue);
	}

	retValue.setFieldValue(1, szAfterConnect, strlen(szAfterConnect));
}

/* Cut插件 */
Cut::Cut() {
}

Cut::~Cut() {
	zhjs::CommonPluginFactory::dispose(this);
}

void Cut::printMe() {
	printf("\t插件名称:Cut,版本号：3.0.0 \n");
}

std::string Cut::getPluginName() {
	return "Cut";
}

std::string Cut::getPluginVersion(){
	return "3.0.0";
}

void Cut::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() == 2) {
		memset(m_szFieldValue, 0, sizeof(m_szFieldValue));
		pps.getFieldValue(1, m_szFieldValue);
		delSpace(m_szFieldValue, 0);

		memset(m_szToBeCut, 0, sizeof(m_szToBeCut));
		pps.getFieldValue(2, m_szToBeCut);
		delSpace(m_szToBeCut, 0);

		m_iCutLen = strlen(m_szToBeCut);
		if (0 == memcmp(m_szFieldValue, m_szToBeCut, m_iCutLen)) {
			strcpy(m_szValueAfterCut, (m_szFieldValue + m_iCutLen));
			retValue.setFieldValue(1, m_szValueAfterCut, strlen(
					m_szValueAfterCut));
		} else {
			retValue.setFieldValue(1, m_szFieldValue, strlen(m_szFieldValue));
		}
	} else if (pps.getItem_num() == 3) {
		memset(m_szFieldValue, 0, sizeof(m_szFieldValue));
		pps.getFieldValue(1, m_szFieldValue);
		delSpace(m_szFieldValue, 0);

		memset(m_szLocation, 0, sizeof(m_szLocation));
		pps.getFieldValue(2, m_szLocation);
		delSpace(m_szLocation, 0);

		char szLen[RECORD_LENGTH];
		memset(szLen, 0, sizeof(szLen));
		pps.getFieldValue(3, szLen);
		delSpace(szLen, 0);
		m_iCutLen = atoi(szLen);

		if (m_szLocation[0] == 'L') {
			strcpy(m_szValueAfterCut, (m_szFieldValue + m_iCutLen));
			retValue.setFieldValue(1, m_szValueAfterCut, strlen(
					m_szValueAfterCut));
		} else if (m_szLocation[0] == 'R') {
			memset(m_szValueAfterCut, 0, sizeof(m_szValueAfterCut));
			strncpy(m_szValueAfterCut, m_szFieldValue, (strlen(m_szFieldValue)
					- m_iCutLen));
			retValue.setFieldValue(1, m_szValueAfterCut, strlen(
					m_szValueAfterCut));

		} else {
			sprintf(ErrorMsg, "the second param sent to Cut Error!");
			throw jsexcp::CException(0, (char *) ErrorMsg, (char *) __FILE__, __LINE__);
		}
	} else if (pps.getItem_num() == 4) {
		char szOutValue[RECORD_LENGTH + 1];
		memset(m_szFieldValue, 0, sizeof(m_szFieldValue));
		pps.getFieldValue(1, m_szFieldValue);
		delSpace(m_szFieldValue, 0);

		char szStrToCut[RECORD_LENGTH + 1];
		memset(szStrToCut, 0, sizeof(szStrToCut));
		pps.getFieldValue(2, szStrToCut);
		delSpace(szStrToCut, 0);

		char szOccurNum[RECORD_LENGTH + 1];
		int iOccurNum;
		memset(szOccurNum, 0, sizeof(szOccurNum));
		pps.getFieldValue(3, szOccurNum);
		delSpace(szOccurNum, 0);
		iOccurNum = atoi(szOccurNum);

		memset(m_szLocation, 0, sizeof(m_szLocation));
		pps.getFieldValue(4, m_szLocation);
		delSpace(m_szLocation, 0);

		strcpy(szOutValue, m_szFieldValue);

		char* pFirstPos = NULL;
		char* pCutPos = NULL;
		pFirstPos = strstr(m_szFieldValue, szStrToCut);
		if (pFirstPos != NULL) {
			if (iOccurNum == 1) {
				memset(szOutValue, 0, sizeof(szOutValue));
				strncpy(szOutValue, m_szFieldValue,
						(pFirstPos - m_szFieldValue));
				strcat(szOutValue, pFirstPos + 1);
			} else {
				pCutPos = pFirstPos + 1;
				int i;
				for (i = 1; i < iOccurNum; i++) {
					pCutPos = strstr(pCutPos, szStrToCut);
					if (pCutPos != NULL)
						continue;
				}
				if (i == iOccurNum) {
					memset(szOutValue, 0, sizeof(szOutValue));
					strncpy(szOutValue, m_szFieldValue, (pFirstPos
							- m_szFieldValue));
					strcat(szOutValue, pCutPos + 1);
				}
			}
		}
		retValue.setFieldValue(1, szOutValue, strlen(szOutValue));
	} else {
		sprintf(ErrorMsg, "the params sent to Cut Error!");
		throw jsexcp::CException(0, (char *) ErrorMsg, (char *) __FILE__, __LINE__);
	}
}

/* Output插件 */
Output::Output() {
}

Output::~Output() {
	zhjs::CommonPluginFactory::dispose(this);
}

void Output::printMe() {
	printf("\t插件名称:Output,版本号：3.0.0 \n");
}

std::string Output::getPluginName() {
	return "Output";
}

std::string Output::getPluginVersion(){
	return "3.0.0";
}

void Output::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 1) {
		sprintf(ErrorMsg, "the params sent to Output is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	char szTmp[RECORD_LENGTH + 1];
	memset(szTmp, 0, sizeof(szTmp));
	pps.getFieldValue(1, szTmp);
	//cout << "get field 1 = " << szTmp << endl;
	delSpace(szTmp, 0);
	//cout << "set 1 = " << szTmp << endl;
	retValue.setFieldValue(1, szTmp, strlen(szTmp));
}

/* Fill插件 */
Fill::Fill() {
}

Fill::~Fill() {
	zhjs::CommonPluginFactory::dispose(this);
}

void Fill::printMe() {
	printf("\t插件名称:Fill,版本号：3.0.0 \n");
}

std::string Fill::getPluginName() {
	return "Fill";
}

std::string Fill::getPluginVersion(){
	return "3.0.0";
}

void Fill::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 4) {
		sprintf(ErrorMsg, "the params sent to Fill is lack");
		throw jsexcp::CException(0, (char *) ErrorMsg, (char *) __FILE__, __LINE__);
	}

	memset(m_szFieldValue, 0, sizeof(m_szFieldValue));
	pps.getFieldValue(1, m_szFieldValue);
	delSpace(m_szFieldValue, 0);

	char szTmp[RECORD_LENGTH + 1];
	memset(szTmp, 0, sizeof(szTmp));
	pps.getFieldValue(2, szTmp);
	delSpace(szTmp, 0);
	m_iLength = atoi(szTmp);

	memset(m_szWhere, 0, sizeof(m_szWhere));
	pps.getFieldValue(3, m_szWhere);
	delSpace(m_szWhere, 0);

	memset(m_szFilling, 0, sizeof(m_szFilling));
	pps.getFieldValue(4, m_szFilling);
	delSpace(m_szFilling, 0);

	int iLen = strlen(m_szFieldValue);

	if (iLen < m_iLength) {
		memset(szTmp, m_szFilling[0], sizeof(szTmp));
		szTmp[m_iLength - iLen] = '\0';

		if (m_szWhere[0] == 'L') {
			strcat(szTmp, m_szFieldValue);
			strcpy(m_szFieldValue, szTmp);

		} else if (m_szWhere[0] == 'R') {
			strcat(m_szFieldValue, szTmp);
		} else {
			sprintf(ErrorMsg,
					"The third param sent to Fill should be 'R' or 'L' !");
			throw jsexcp::CException(0, (char *) ErrorMsg, (char *) __FILE__, __LINE__);
		}
	}
	retValue.setFieldValue(1, m_szFieldValue, strlen(m_szFieldValue));
}

Change::Change() {
}

Change::~Change() {
	zhjs::CommonPluginFactory::dispose(this);
}

void Change::printMe() {
	printf("\t插件名称:Change,版本号：3.0.0 \n");
}

std::string Change::getPluginName() {
	return "Change";
}

std::string Change::getPluginVersion(){
	return "3.0.0";
}

void Change::execute(PacketParser& pps, ResParser& retValue) {
	char m_szBefChange[RECORD_LENGTH];
	char m_szAfterChange[RECORD_LENGTH];
	char m_szValue[RECORD_LENGTH];
	int m_iPos;

	if (pps.getItem_num() == 3) {
		memset(m_szValue, 0, sizeof(m_szValue));
		pps.getFieldValue(1, m_szValue);
		delSpace(m_szValue, 0);

		char sztmp[RECORD_LENGTH];
		memset(sztmp, 0, sizeof(sztmp));
		pps.getFieldValue(2, sztmp);
		delSpace(sztmp, 0);
		m_iPos = atoi(sztmp);

		memset(m_szFillValue, 0, sizeof(m_szFillValue));
		pps.getFieldValue(3, m_szFillValue);
		delSpace(m_szFillValue, 0);

		if ((m_iPos > strlen(m_szValue)) || (strlen(m_szFillValue) > 1)) {
			sprintf(ErrorMsg, "the params sent to Change Error!");
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		} else {
			m_szValue[m_iPos - 1] = m_szFillValue[0];
			retValue.setFieldValue(1, m_szValue, strlen(m_szValue));
			return;
		}

	} else {
		sprintf(ErrorMsg, "the params sent to Change Error!");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
}

//initial::~initial()
//{
//}
//
//initial::initial()
//{
//}
//
//void initial::printMe()
//{
//	printf("\t插件名称:initial,版本号：3.0.0 \n");
//}
//
//std::string initial::getPluginName()
//{
//	return "initial";
//}
//
//void initial::execute(PacketParser& pps, ResParser& retValue)
//{
//	if (pps.getItem_num() != 1)
//	{
//		sprintf(ErrorMsg, "the params sent to initial is lack");
//		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
//				__LINE__);
//	}
//
//	memset(col_num, 0, sizeof(col_num));
//	pps.getFieldValue(0, col_num);
//	delSpace(col_num, 0);
//
//	int icol_num = atoi(col_num);
//	char nvalue[2] = "";
//
//	for (int i = 0; i < icol_num; i++)
//		retValue.setFieldValue(i, nvalue, strlen(nvalue));
//}

Sum::Sum() {
}

Sum::~Sum() {
	zhjs::CommonPluginFactory::dispose(this);
}

void Sum::printMe() {
	printf("\t插件名称:Sum,版本号：3.0.0 \n");
}

std::string Sum::getPluginName() {
	return "Sum";
}

std::string Sum::getPluginVersion(){
	return "3.0.0";
}

void Sum::execute(PacketParser& pps, ResParser& retValue) {
	m_iParamNum = pps.getItem_num();
	if (m_iParamNum < 2) {
		sprintf(ErrorMsg, "the params sent to Sum is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	m_lResult = 0;
	char szTmp[RECORD_LENGTH + 1];
	int lAfterConvert;
	char szAfterConvert[RECORD_LENGTH];
	int iLen;

	//edit by linyb 20070802
	for (int i = 1; i <= m_iParamNum; i++) {
		memset(szTmp, 0, sizeof(szTmp));
		pps.getFieldValue(i, szTmp);
		delSpace(szTmp, 0);

		if (!isNo(szTmp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Sum, %s is not number", szTmp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_lResult = m_lResult + atol(szTmp);
	}
	sprintf(szTmp, "%ld", m_lResult);
	retValue.setFieldValue(1, szTmp, strlen(szTmp));
}

C_HhMmSsTt2S::C_HhMmSsTt2S() {
}

C_HhMmSsTt2S::~C_HhMmSsTt2S() {
	zhjs::CommonPluginFactory::dispose(this);
}

void C_HhMmSsTt2S::printMe() {
	printf("\t插件名称:HhMmSsTt2S,版本号：3.0.0 \n");
}

std::string C_HhMmSsTt2S::getPluginName() {
	return "HhMmSsTt2S";
}

std::string C_HhMmSsTt2S::getPluginVersion(){
	return "3.0.0";
}

void C_HhMmSsTt2S::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 10) {
		sprintf(ErrorMsg, "the params sent to C_HhMmSsTt2S is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	char sztmp[1024];
	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(2, sztmp);
	delSpace(sztmp, 0);
	m_iHBeginPos = atoi(sztmp);
	m_iHBeginPos--;

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(3, sztmp);
	delSpace(sztmp, 0);
	m_iHLength = atoi(sztmp);

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(4, sztmp);
	delSpace(sztmp, 0);
	m_iMBeginPos = atoi(sztmp);
	m_iMBeginPos--;

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(5, sztmp);
	delSpace(sztmp, 0);
	m_iMLength = atoi(sztmp);

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(6, sztmp);
	delSpace(sztmp, 0);
	m_iSBeginPos = atoi(sztmp);
	m_iSBeginPos--;

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(7, sztmp);
	delSpace(sztmp, 0);
	m_iSLength = atoi(sztmp);

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(8, sztmp);
	delSpace(sztmp, 0);
	m_iTBeginPos = atoi(sztmp);
	m_iTBeginPos--;

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(9, sztmp);
	delSpace(sztmp, 0);
	m_iTLength = atoi(sztmp);

	memset(sztmp, 0, sizeof(sztmp));
	pps.getFieldValue(10, sztmp);
	delSpace(sztmp, 0);
	m_iOutLength = atoi(sztmp);

	char szFieldValue[1024];
	memset(szFieldValue, 0, sizeof(szFieldValue));
	pps.getFieldValue(1, szFieldValue);
	delSpace(szFieldValue, 0);

	long iSumResult = 0;

	memset(sztmp, 0, sizeof(sztmp));
	iSumResult += atoi((const char*) (memcpy(sztmp,
			szFieldValue + m_iHBeginPos, m_iHLength))) * 3600;
	memset(sztmp, 0, sizeof(sztmp));
	iSumResult += atoi((const char*) (memcpy(sztmp,
			szFieldValue + m_iMBeginPos, m_iMLength))) * 60;
	memset(sztmp, 0, sizeof(sztmp));
	iSumResult += atoi((const char*) (memcpy(sztmp,
			szFieldValue + m_iSBeginPos, m_iSLength)));
	memset(sztmp, 0, sizeof(sztmp));
	if (atoi((const char*) (memcpy(sztmp, szFieldValue + m_iTBeginPos,
			m_iTLength))) > 0)
		iSumResult++;

	char szTmpResult[1024];
	char szResult[1024];
	int iZeroCount = 0;
	sprintf(szTmpResult, "%d", iSumResult);
	iZeroCount = m_iOutLength - strlen(szTmpResult);
	memset(szResult, 0, sizeof(szResult));
	while (iZeroCount > 0) {
		strcat(szResult, "0");
		iZeroCount--;
	}
	strcat(szResult, szTmpResult);
	retValue.setFieldValue(1, szResult, strlen(szResult));
}

//减法
C_Minus::C_Minus() {
}

C_Minus::~C_Minus() {
	zhjs::CommonPluginFactory::dispose(this);
}

void C_Minus::printMe() {
	printf("\t插件名称:Minus,版本号：3.0.0 \n");
}

std::string C_Minus::getPluginName() {
	return "Minus";
}

std::string C_Minus::getPluginVersion(){
	return "3.0.0";
}

void C_Minus::execute(PacketParser& pps, ResParser& retValue) {
	m_iParamNum = pps.getItem_num();
	if (m_iParamNum < 2) {
		char errorMsg[ERROR_MSG_LEN + 1];
		sprintf(errorMsg, "the params sent to Minus is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	m_lResult = 0;
	char szTmp[RECORD_LENGTH + 1];

	memset(szTmp, 0, sizeof(szTmp));
	pps.getFieldValue(1, szTmp);
	delSpace(szTmp, 0);
	if (!isNo(szTmp)) {
		//add by linyb 20080121
		sprintf(ErrorMsg, "in plugin Minus, minuend %s is not number", szTmp);
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
		//end of add by linyb 20080121
	}
	m_lResult = atol(szTmp);

	for (int i = 2; i <= m_iParamNum; i++) {
		memset(szTmp, 0, sizeof(szTmp));
		pps.getFieldValue(i, szTmp);
		delSpace(szTmp, 0);
		if (!isNo(szTmp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Minus, subtrahend %s is not number",
					szTmp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_lResult = m_lResult - atol(szTmp);
	}

	sprintf(szTmp, "%ld", m_lResult);
	retValue.setFieldValue(1, szTmp, strlen(szTmp));
}

//乘法
C_Multiple::C_Multiple() {
}

C_Multiple::~C_Multiple() {
	zhjs::CommonPluginFactory::dispose(this);
}

void C_Multiple::printMe() {
	printf("\t插件名称:Multiple,版本号：3.0.0 \n");
}

std::string C_Multiple::getPluginName() {
	return "Multiple";
}

std::string C_Multiple::getPluginVersion(){
	return "3.0.0";
}

void C_Multiple::execute(PacketParser& pps, ResParser& retValue) {
	m_iParamNum = pps.getItem_num();
	if (m_iParamNum < 2) {
		char errorMsg[ERROR_MSG_LEN + 1];
		sprintf(errorMsg, "the params sent to Multiple is lack");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	m_lResult = 1;
	char szTmp[RECORD_LENGTH + 1];
	for (int i = 1; i <= m_iParamNum; i++) {
		memset(szTmp, 0, sizeof(szTmp));
		pps.getFieldValue(i, szTmp);
		delSpace(szTmp, 0);

		if (!isNo(szTmp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg,
					"in plugin Multiple, multiplier %s is not number", szTmp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_lResult = m_lResult * atol(szTmp);
	}

	sprintf(szTmp, "%ld", m_lResult);
	retValue.setFieldValue(1, szTmp, strlen(szTmp));
}

C_Divide::C_Divide() {
}

C_Divide::~C_Divide() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string C_Divide::getPluginName() {
	return "Divide";
}

std::string C_Divide::getPluginVersion(){
	return "3.0.0";
}

void C_Divide::printMe() {
	printf("\t插件名称:Divide,版本号：3.0.0 \n");
}

double C_Divide::Round(double d, int iPrecision) {
	double dResult;
	double dTmp = pow((double) 10, iPrecision);

	double integerPart; //整数部分
	double decimalPart;//小数部分

	decimalPart = modf(d, &integerPart);

	decimalPart = floor(decimalPart * dTmp + 0.5) / dTmp;

	dResult = integerPart + decimalPart;

	return (dResult);
}

void C_Divide::execute(PacketParser& pps, ResParser& retValue) {
	char szResult[30];
	if (pps.getItem_num() == 3) {
		char temp[RECORD_LENGTH + 1];

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(1, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, dealMethod %s is not number",
					temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_dealMethod = atoi(temp);

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(2, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, dividend %s is not number",
					temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_dividend = atof(temp);

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(3, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, divisor %s is not number",
					temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_divisor = atof(temp);
		//除数为零
		if (m_divisor == 0) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, divisor %s is zero", temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}

		double dTempResult = m_dividend / m_divisor;
		long lResult;
		//取大
		if (m_dealMethod == 2) {
			//ceil(double num) 返回不小于num的最小整数(表示为浮点值)
			lResult = ceil(dTempResult);
		}
		//取小
		else if (m_dealMethod == 3) {
			//floor(double num) 返回不大于num的最小整数(表示为浮点值)
			lResult = floor(dTempResult);
		} else {
			char errorMsg[ERROR_MSG_LEN + 1];
			sprintf(errorMsg, "plugin Divide undefined dealMethod = [%d]",
					m_dealMethod);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}

		sprintf(szResult, "%ld", lResult);
	}
	//四舍五入
	else if (pps.getItem_num() == 4) {
		char temp[RECORD_LENGTH + 1];

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(1, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, dealmethod %s is not number",
					temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_dealMethod = atoi(temp);
		if (m_dealMethod != 1) {
			char errorMsg[ERROR_MSG_LEN + 1];
			sprintf(errorMsg, "plugin Divide undefined dealMethod = [%d]",
					m_dealMethod);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(2, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg,
					"in plugin Divide, decimalLength %s is not number", temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		//保留decimalLength位小数
		int decimalLength = atoi(temp);
		if (decimalLength < 0) {
			char errorMsg[ERROR_MSG_LEN + 1];
			sprintf(errorMsg,
					"plugin Divide dealMethod = [%d] decimal_length = [%d]",
					m_dealMethod, decimalLength);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		}

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(3, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, dividend %s is not number",
					temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_dividend = atof(temp);

		memset(temp, 0, sizeof(temp));
		pps.getFieldValue(4, temp);
		delSpace(temp, 0);
		if (!isNo(temp)) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, divisor %s is not number",
					temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		m_divisor = atof(temp);

		//除数为零
		if (m_divisor == 0) {
			//add by linyb 20080121
			sprintf(ErrorMsg, "in plugin Divide, divisor %s is zero", temp);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
			//end of add by linyb 20080121
		}
		//NODE:	Round(2.4450, 2) 返回的结果是2.44(不是预计结果2.45)
		//可能跟机器二进制截取有关
		double dResult = Round(m_dividend / m_divisor, decimalLength);

		//注:这里可能会发生地址越界(当dResult长度大于szResult的数组长度时)
		sprintf(szResult, "%.*f", decimalLength, dResult);

	} else {
		char errorMsg[ERROR_MSG_LEN + 1];
		sprintf(errorMsg, "Plugin  Divide param count error ");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}

	retValue.setFieldValue(1, szResult, strlen(szResult));
}

C_TimeOperator::C_TimeOperator() {
}

C_TimeOperator::~C_TimeOperator() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string C_TimeOperator::getPluginName() {
	return "TimeOperator";
}

std::string C_TimeOperator::getPluginVersion(){
	return "3.0.0";
}

void C_TimeOperator::printMe() {
	printf("\t插件名称:TimeOperator,版本号：3.0.0 \n");
}

void C_TimeOperator::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 3) {
		sprintf(ErrorMsg, "the params sent to TimeAdd is Error");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	char buff[RECORD_LENGTH + 1];
	memset(buff, 0, RECORD_LENGTH + 1);

	char cdrtime[RECORD_LENGTH + 1];
	memset(cdrtime, 0, RECORD_LENGTH + 1);
	//先取出第一个时间，判断是正确的时间格式后，正式初始化
	pps.getFieldValue(1, cdrtime);
	if (strlen(cdrtime) != 14) {
		sprintf(ErrorMsg,
				"the first param =%s= which sent to TimeAdd  strlen() != 14",
				cdrtime);
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	org_time.init(cdrtime);

	//开始取出参数列表里面的需要进行操作的参数
	//取出需要增加的数值
	memset(buff, 0, RECORD_LENGTH + 1);
	pps.getFieldValue(2, buff);
	add_time = atoi(buff);

	//取出需要增加数值的位置
	memset(buff, 0, RECORD_LENGTH + 1);
	pps.getFieldValue(3, buff);
	add_position = atoi(buff);

	switch (add_position) {
	case YEAR: {
		org_time.year += add_time;
		break;
	}
	case MONTH: {
		//modi by xuhuiquan 20080602
		//t = (org_time.month + add_time)/12 - 1;
		t = (org_time.month + add_time) / 12;
		org_time.year += t;
		org_time.month = (org_time.month + add_time) % 12;
		if (org_time.month == 0) {
			org_time.month = 12;
			//add by xuhuiquan 20080602
			org_time.year -= 1;
		}
		break;
	}
	case DAY:
	case HOUR:
	case MIN:
	case SECOND: {
		switch (add_position) {
		case DAY: {
			add_seconds = add_time * 24 * 60 * 60;
			break;
		}
		case HOUR: {
			add_seconds = add_time * 60 * 60;
			break;
		}
		case MIN: {
			add_seconds = add_time * 60;
			break;
		}
		case SECOND: {
			add_seconds = add_time;
			break;
		}
		};
		org_seconds = timeStr2Time(cdrtime);
		if (org_seconds != -1) {
			org_seconds += add_seconds;
		}
		time2TimeStr(org_seconds, cdrtime);
		retValue.setFieldValue(1, cdrtime, strlen(cdrtime));
		return;
		break;
	}
	default: {
		sprintf(ErrorMsg, "the add position is error, not between 1 ~ 6");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	}

	sprintf(buff, "%04d%02d%02d%02d%02d%02d", org_time.year, org_time.month,
			org_time.day, org_time.hour, org_time.minute, org_time.sec);
	retValue.setFieldValue(1, buff, strlen(buff));
	return;
}

C_IsDigit::C_IsDigit() {
}

C_IsDigit::~C_IsDigit() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string C_IsDigit::getPluginName() {
	return "IsDigit";
}

std::string C_IsDigit::getPluginVersion(){
	return "3.0.0";
}

void C_IsDigit::printMe() {
	printf("\t插件名称:IsDigit,版本号：3.0.0 \n");
}

//20120918 更改判断逻辑
void C_IsDigit::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 2) {
		sprintf(ErrorMsg, "the params sent to IsDigit is Error");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}

	char buff[RECORD_LENGTH + 1];
	char errorcode[RECORD_LENGTH + 1];
	bool dot = false;

	memset(buff, 0, RECORD_LENGTH + 1);
	memset(errorcode, 0, RECORD_LENGTH + 1);

	pps.getFieldValue(1, buff);
	pps.getFieldValue(2, errorcode);
	int len;
	len = strlen(buff);
	for (int i = 0; i < len; i++) {
		if (i == 0) //开始第一位判断是否有+ - .
		{
			if (buff[i] != '+' && buff[i] != '-' && buff[i] != '.' && (buff[i]
					< '0' || buff[i] > '9')) {
                                retValue.setFieldValue(1,errorcode,strlen(errorcode));
				return;
			}
			if (buff[i] == '.') //如果是.，置位
			{
				dot = true;
			}
		} else {
			if ((buff[i] < '0' || buff[i] > '9') && buff[i] != '.') {
                                retValue.setFieldValue(1,errorcode,strlen(errorcode));
				return;
			}

			if (buff[i] == '.' && dot) {
				return;
			} else if (buff[i] == '.' && !dot) {
				dot = true;
			}
		}
	}
	retValue.setFieldValue(1, buff, strlen(buff));
	return;
}

C_StringReplace::C_StringReplace() {
}

C_StringReplace::~C_StringReplace() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string C_StringReplace::getPluginName() {
	return "StringReplace";
}

std::string C_StringReplace::getPluginVersion(){
	return "3.0.0";
}

void C_StringReplace::printMe() {
	printf("\t插件名称:StringReplace,版本号：3.0.0 \n");
}

void C_StringReplace::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 3) {
		sprintf(ErrorMsg, "the params sent to StringReplace is Error");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}

	//需要操作的字符串
	memset(buff, 0, RECORD_LENGTH + 1);
	pps.getFieldValue(1, buff);
	in = buff;

	//被替换的一段字符
	memset(buff, 0, RECORD_LENGTH + 1);
	pps.getFieldValue(2, buff);
	search_item = buff;

	//替换成的一段字符
	memset(buff, 0, RECORD_LENGTH + 1);
	pps.getFieldValue(3, buff);
	replace_item = buff;

	if (in.find(search_item) != -1) {
		in.replace(in.find(search_item), search_item.size(), replace_item);
	}

	sprintf(buff, "%s", in.c_str());
	retValue.setFieldValue(1, buff, strlen(buff));
}

C_TimeCalculate::C_TimeCalculate() {
}

C_TimeCalculate::~C_TimeCalculate() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string C_TimeCalculate::getPluginName() {
	return "TimeCalculate";
}

std::string C_TimeCalculate::getPluginVersion(){
	return "3.0.0";
}

void C_TimeCalculate::printMe() {
	printf("\t插件名称:TimeCalculate,版本号：3.0.0 \n");
}

bool C_TimeCalculate::check_time(char* date, char* time, char* result, int type) {
	char cdr_time[RECORD_LENGTH + 1];
	memset(cdr_time, 0, RECORD_LENGTH + 1);
	my_time t_check;

	if (strlen(date) != 14 && strlen(date) != 12 && strlen(date) != 8
			&& strlen(date) != 6) {
		if (type == 0) {
			sprintf(
					ErrorMsg,
					"the first param =%s= which sent to TimeCalculate  strlen() != 14 or 12 or 8 or 6",
					date);
			throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg,
					(char *) __FILE__, __LINE__);
		} else {
			sprintf(result, "%s", time);
			return true;
		}
	}

	if (strlen(date) == 14) {
		sprintf(cdr_time, "%s", date);
	} else if (strlen(date) == 12) {
		sprintf(cdr_time, "20%s", date);
	} else if (strlen(date) == 8) {
		sprintf(cdr_time, "%s%s", date, time);
	} else if (strlen(date) == 6) {
		sprintf(cdr_time, "20%s%s", date, time);
	}

	if (strlen(cdr_time) != 14) {
		sprintf(ErrorMsg, "the %d date/time error. Msg=%s=", type + 1, cdr_time);
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}

	t_check.init(cdr_time);
	sprintf(result, "%s", cdr_time);

	return true;
}

void C_TimeCalculate::execute(PacketParser& pps, ResParser& retValue) {
	if (pps.getItem_num() != 5) {
		sprintf(ErrorMsg, "the params sent to TimeCalculate is Error");
		throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
				__LINE__);
	}
	char buff2[RECORD_LENGTH + 1];
	memset(buff2, 0, RECORD_LENGTH + 1);

	char buff1[RECORD_LENGTH + 1];
	memset(buff1, 0, RECORD_LENGTH + 1);

	//先取出所有参数
	memset(date1, 0, RECORD_LENGTH + 1);
	memset(time1, 0, RECORD_LENGTH + 1);
	memset(date2, 0, RECORD_LENGTH + 1);
	memset(time2, 0, RECORD_LENGTH + 1);
	memset(day, 0, RECORD_LENGTH + 1);

	pps.getFieldValue(1, date1);
	pps.getFieldValue(2, time1);
	pps.getFieldValue(3, date2);
	pps.getFieldValue(4, time2);
	pps.getFieldValue(5, day);

	long second_1;
	long second_2;

	if (check_time(date1, time1, buff1, 0)
			&& check_time(date2, time2, buff2, 1)) {
		if (strlen(buff2) != 12 || strlen(buff2) != 14) {
			sprintf(buff2, "%s", buff1);
			buff2[8] = 0;
			sprintf(buff2, "%s%s", buff2, time2);

			long org_seconds = timeStr2Time(buff2);
			if (org_seconds != -1) {
				org_seconds += atoi(day) * 24 * 60 * 60;
			}
			time2TimeStr(org_seconds, buff2);
		}

		second_1 = timeStr2Time(buff1);
		second_2 = timeStr2Time(buff2);

		second_1 = second_2 - second_1;
	}

	retValue.setFieldValue(1, buff1, strlen(buff1));
	retValue.setFieldValue(2, buff2, strlen(buff2));

	sprintf(buff1, "%d", second_1);

	retValue.setFieldValue(3, buff1, strlen(buff1));
}

LackInfo::LackInfo() {
}

LackInfo::~LackInfo() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string LackInfo::getPluginName() {
	return "LackInfo";
}

std::string LackInfo::getPluginVersion(){
	return "3.0.0";
}

void LackInfo::printMe() {
	printf("\t插件名称:LackInfo,版本号：3.0.0 \n");
}

void LackInfo::execute(PacketParser& pps, ResParser& retValue) {

	char buff[RECORD_LENGTH + 1];
	char ch[RECORD_LENGTH + 1];
	std::string msg;
	pps.getFieldValue(1, buff);
	for (int i = 2; i <= pps.getItem_num(); i++) {
		pps.getFieldValue(i, ch);
		msg += ch;
	}
	strncpy(ch, msg.c_str(), RECORD_LENGTH);
	ch[RECORD_LENGTH] = '\0';
	retValue.setAnaResult(eLackInfo, buff, ch);
}

Abnormal::Abnormal() {
}

Abnormal::~Abnormal() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string Abnormal::getPluginName() {
	return "Abnormal";
}

std::string Abnormal::getPluginVersion(){
	return "3.0.0";
}

void Abnormal::printMe() {
	printf("\t插件名称:LackInfo,版本号：3.0.0 \n");
}

void Abnormal::execute(PacketParser& pps, ResParser& retValue) {

	char buff[RECORD_LENGTH + 1];
	char ch[RECORD_LENGTH + 1];
	std::string msg;
	pps.getFieldValue(1, buff);
	for (int i = 2; i <= pps.getItem_num(); i++) {
		pps.getFieldValue(i, ch);
		msg += ch;
	}
	strncpy(ch, msg.c_str(), RECORD_LENGTH);
	ch[RECORD_LENGTH] = '\0';
	retValue.setAnaResult(eAbnormal, buff, ch);
}


Classify::Classify() {
}

Classify::~Classify() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string Classify::getPluginName() {
	return "Classify";
}

std::string Classify::getPluginVersion(){
	return "3.0.0";
}

void Classify::printMe() {
	printf("\t插件名称:Classify,版本号：3.0.0 \n");
}

void Classify::execute(PacketParser& pps, ResParser& retValue) 
{
	char ruleNo[RECORD_LENGTH+1];
	pps.getFieldValue(1, ruleNo);
	retValue.setAnaResult(eClassifiable, ruleNo, "");
}



TimeFormat::TimeFormat() {
}

TimeFormat::~TimeFormat() {
	zhjs::CommonPluginFactory::dispose(this);
}

std::string TimeFormat::getPluginName() {
	return "TimeFormat";
}

std::string TimeFormat::getPluginVersion(){
	return "3.0.0";
}

void TimeFormat::printMe() {
	printf("\t插件名称:TimeFormat,版本号：3.0.0 \n");
}

// configure like:
// $StartTime,%Y-%m-%d %H:%M:%S,%Y%m%d%H%M%S,14
// input:2001-11-12 18:31:01
// output:20011112183101
// 类方法先将时间字符串通过strptime转换成时间结构体tm，然后再将tm通过strftime转换成字符串
// 详细使用方法及配置方法请查看Unix/Linux 系统帮助手册，如下：
//    man strptime
//    man strftime
void TimeFormat::execute(PacketParser& pps, ResParser& retValue) {
	tm tmp;
	char src[RECORD_LENGTH+1], dest[RECORD_LENGTH+1], fformat[100],tformat[100],len_c[20];
	pps.getFieldValue(1, src);
	pps.getFieldValue(2, fformat);
	pps.getFieldValue(3, tformat);
	pps.getFieldValue(4, len_c);
	delSpace(len_c, 0);
	int len = atoi(len_c);
    strptime(src, fformat, &tmp);
    strftime(dest, RECORD_LENGTH, tformat, &tmp);
	retValue.setFieldValue(1, dest, len);
}

C_IsXDigit::C_IsXDigit() {
}

C_IsXDigit::~C_IsXDigit() {
        zhjs::CommonPluginFactory::dispose(this);
}

std::string C_IsXDigit::getPluginName() {
        return "IsXDigit";
}

std::string C_IsXDigit::getPluginVersion(){
        return "3.0.0";
}

void C_IsXDigit::printMe() {
        printf("\t插件名称:C_IsXDigit,版本号：3.0.0 \n");
}

//校验是否是16进制数
void C_IsXDigit::execute(PacketParser& pps, ResParser& retValue) {
        if (pps.getItem_num() != 2) {
                sprintf(ErrorMsg, "the params sent to IsXDigit is Error");
                throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,
                                __LINE__);
        }

        char buff[RECORD_LENGTH + 1];
        char errorcode[RECORD_LENGTH + 1];
        bool dot = false;

        memset(buff, 0, RECORD_LENGTH + 1);
        memset(errorcode, 0, RECORD_LENGTH + 1);

        pps.getFieldValue(1, buff);
        pps.getFieldValue(2, errorcode);
        int len;
        len = strlen(buff);
        //cout<<"buff:"<<buff<<endl;

        for (int i = 0; i < len; i++) {
            if(isxdigit(buff[i]) == 0)
		{			
			retValue.setFieldValue(1, errorcode, strlen(errorcode));
                        return;
		}
        }
        retValue.setFieldValue(1, buff, strlen(buff));
        return;
}

C_MinusTime::C_MinusTime() {
}

C_MinusTime::~C_MinusTime() {
        zhjs::CommonPluginFactory::dispose(this);
}

std::string C_MinusTime::getPluginName() {
        return "MinusTime";
}

std::string C_MinusTime::getPluginVersion(){
        return "3.0.0";
}

void C_MinusTime::printMe() {
        printf("\t插件名称:C_MinusTime,版本号：3.0.0 \n");
}

/*
 *description:
 *	MinusTimes the param
 *input:
 *	the param to be MinusTimes
 *output:
 *  the param
return
 *  
 */

void C_MinusTime::execute(PacketParser& pps,ResParser& retValue)
{
	if ( pps.getItem_num() != 2 )
	{
		sprintf(ErrorMsg, "the params sent to MinusTimes is Error");
    throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,__LINE__);
	}

	char starttime[15];
	char endtime[15];
	int Tmpid=0;
	memset( starttime, 0, sizeof( starttime ) );
	pps.getFieldValue( 0, starttime );
	delSpace( starttime , 0 );
	if(atoi(starttime)==0)
	{
		getCurTime(starttime);
		Tmpid = 1;
	}
	else
		Tmpid = 2;
	
	memset( endtime, 0, sizeof( endtime ) );
	pps.getFieldValue( 1, endtime );
	delSpace( endtime , 0 );
	
/*
	memset( Tmpid, 0, sizeof( Tmpid ) );
	pps.getFieldValue( 2, Tmpid );
	delSpace( Tmpid , 0 );
	//*/
	//printf("time:%s  %s  %d\n",starttime,endtime,Tmpid);
	
	//date
	if(Tmpid==1)
	{
		if(strlen(starttime)!=14 || strlen(endtime)!=14)
			{
				sprintf( ErrorMsg,"starttime or endtime is not date");
				throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,__LINE__);
			}
		
		struct tm *tmp_time = (struct tm*)calloc(1,sizeof(struct tm));
		struct tm *tmp_time2 = (struct tm*)calloc(1,sizeof(struct tm));
		//因为hp的原因，必须把字段用分隔符分割
		char strp_time[20];
		memset(strp_time,0,sizeof(strp_time));
		for(int i=0,j=0;i<strlen(starttime);j++)
		{
				if(j==4 || j==7 || j==10 || j==13 || j==16 ) strp_time[j]='-';
				else	
				{
						strp_time[j]=starttime[i];
						i++;
				}
		}
		strp_time[19]='\0';
		strptime(strp_time,"%Y-%m-%d-%H-%M-%S",tmp_time);
		time_t comp_time = mktime(tmp_time);
		//printf("comp_time=%d\n",comp_time);
		
		
		memset(strp_time,0,sizeof(strp_time));
		for(int i=0,j=0;i<strlen(endtime);j++)
		{
				if(j==4 || j==7 || j==10 || j==13 || j==16 ) strp_time[j]='-';
				else	
				{
						strp_time[j]=endtime[i];
						i++;
				}
		}
		strp_time[19]='\0';
		strptime(strp_time,"%Y-%m-%d-%H-%M-%S",tmp_time2);
		time_t comp_time2 = mktime(tmp_time2);
		//printf("comp_time2=%d\n",comp_time2);
		
		long times = 0;
		char c_times[10];
		times = abs(comp_time2 - comp_time);
		times = times/3600/24 ;
	
		snprintf(c_times,sizeof(c_times),"%ld",times);  

		//printf("%s\n",c_times);
		free(tmp_time);
		free(tmp_time2);
		tmp_time = NULL;
		tmp_time2 = NULL;
		
		retValue.setFieldValue( 1, c_times, strlen( c_times ) );
		
	}
	
	if(Tmpid==2)
	{
		if(strlen(starttime)!=14 || strlen(endtime)!=14)
			{
				sprintf( ErrorMsg,"starttime or endtime is not date");
				throw jsexcp::CException(ERR_LACK_PARAM, (char *) ErrorMsg, (char *) __FILE__,__LINE__);
			}
		
		struct tm *tmp_time = (struct tm*)calloc(1,sizeof(struct tm));
		struct tm *tmp_time2 = (struct tm*)calloc(1,sizeof(struct tm));
		//因为hp的原因，必须把字段用分隔符分割
		char strp_time[20];
		memset(strp_time,0,sizeof(strp_time));
		for(int i=0,j=0;i<strlen(starttime);j++)
		{
				if(j==4 || j==7 || j==10 || j==13 || j==16 ) strp_time[j]='-';
				else	
				{
						strp_time[j]=starttime[i];
						i++;
				}
		}
		strp_time[19]='\0';
		strptime(strp_time,"%Y-%m-%d-%H-%M-%S",tmp_time);
		time_t comp_time = mktime(tmp_time);
		//printf("comp_time=%d\n",comp_time);
		
		
		memset(strp_time,0,sizeof(strp_time));
		for(int i=0,j=0;i<strlen(endtime);j++)
		{
				if(j==4 || j==7 || j==10 || j==13 || j==16 ) strp_time[j]='-';
				else	
				{
						strp_time[j]=endtime[i];
						i++;
				}
		}
		strp_time[19]='\0';
		strptime(strp_time,"%Y-%m-%d-%H-%M-%S",tmp_time2);
		time_t comp_time2 = mktime(tmp_time2);
		//printf("comp_time2=%d\n",comp_time2);
		
		long times = 0;
		char c_times[10];
		times = comp_time2 - comp_time;
	
		snprintf(c_times,sizeof(c_times),"%ld",times);  

		//printf("%s\n",c_times);
				
		free(tmp_time);
		free(tmp_time2);
		tmp_time = NULL;
		tmp_time2 = NULL;
		
		retValue.setFieldValue( 1, c_times, strlen( c_times ) );

	}
		 
}


