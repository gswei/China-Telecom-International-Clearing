#include "encrypt.h"
#include "HugeCalc.h"

#define GENE "37"
////////////////////////////////////////////////////////////////////////////
//class CEncryptAsc
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////
//static key
////////////////////////////////////////
unsigned char CEncryptAsc::m_KeyAsc[128] = {
	0x48, 0xEE, 0x76, 0x1D, 0x67, 0x69, 0xA1, 0x1B, 
	0x7A, 0x86, 0x47, 0xF8, 0x54, 0x95, 0x97, 0x5F, 
 	0x78, 0xD9, 0xDA, 0x6C, 0x59, 0xD7, 0x6B, 0x35, 
 	0xC5, 0x77, 0x85, 0x18, 0x2A, 0x0E, 0x52, 0xFF, 
	0x00, 0xE3, 0x1B, 0x71, 0x8D, 0x34, 0x63, 0xEB, 
  	0x91, 0xC3, 0x24, 0x0F, 0xB7, 0xC2, 0xF8, 0xE3, 
  	0xB6, 0x54, 0x4C, 0x35, 0x54, 0xE7, 0xC9, 0x49, 
  	0x28, 0xA3, 0x85, 0x11, 0x0B, 0x2C, 0x68, 0xFB, 
  	0xEE, 0x7D, 0xF6, 0x6C, 0xE3, 0x9C, 0x2D, 0xE4, 
  	0x72, 0xC3, 0xBB, 0x85, 0x1A, 0x12, 0x3C, 0x32, 
  	0xE3, 0x6B, 0x4F, 0x4D, 0xF4, 0xA9, 0x24, 0xC8, 
  	0xFA, 0x78, 0xAD, 0x23, 0xA1, 0xE4, 0x6D, 0x9A, 
  	0x04, 0xCE, 0x2B, 0xC5, 0xB6, 0xC5, 0xEF, 0x93, 
  	0x5C, 0xA8, 0x85, 0x2B, 0x41, 0x37, 0x72, 0xFA, 
  	0x57, 0x45, 0x41, 0xA1, 0x20, 0x4F, 0x80, 0xB3, 
  	0xD5, 0x23, 0x02, 0x64, 0x3F, 0x6C, 0xF1, 0x0F };

/********************************************************************
函数说明： 加密函数
输入： sInput -- 待加密的密码串, 长度最大不能超过128
	  sOutput -- 密码串加密后的ascii字符串输出缓冲区, 应保证大于257个字节
返回： 0--成功 !0--失败
********************************************************************/
int CEncryptAsc::Encrypt(char* sInput, char* sOutput)
{
	int iBuf[256], i;
	char sTemp[256] = "";

	sOutput[0] = 0x00;
	
	int iLen = strlen(sInput);
	//if( iLen == 0 || iLen > 128 )
	if (iLen > 128)
		return -1; //长度错误
	
	for (i = 0; i < iLen; i++)
		iBuf[i] = (m_KeyAsc[i]^sInput[i]);
	
	for (i=0; i < iLen; i++)
	{
		sprintf(sTemp, "%02x", iBuf[i]);
		strcat(sOutput, sTemp);
	}			

	return 0;
}

/******************************************************************
函数说明： 解密函数
输入： sInput -- 待解密的密码串, 长度最大不能超过256
	  sOutput -- 密码串解密后的ascii字符串输出缓冲区, 应保证大于129个字节
返回： 0--成功 !0--失败
*******************************************************************/
int CEncryptAsc::Decrypt(char *sInput, char *sOutput)
{
	char sBuf[256] = "";
	char sTemp[256] = "";
	int i = 0, j = 0;
	int nTemp;

	int iLen = strlen(sInput);
	//if( iLen == 0 || iLen > 256 ) 
	if (iLen > 256)
		return -1; //长度错误	

	while (sInput[i]!=0x00)
	{
		memcpy(sTemp, sInput+i, 2);
		sTemp[2] = 0x00;
		sscanf(sTemp, "%x",&nTemp);
		sBuf[j] = nTemp;
		sOutput[j] = (sBuf[j]^m_KeyAsc[j]);
		j++;
		i +=2;
	}

	sOutput[j] = 0x00;

	return 0;	
}

int CEncryptAsc::UltraEncrypt(char* strInput, char* strOutput)
{
	char strBuf1[256];
	char strBuf2[256];
	int iLen;

	iLen = strlen(strInput);
	if(iLen > 50)
		return -1;//待加密串太长

	iLen = iLen+19;

	if(!Check(strInput))
		return -2;//有非数字

	sprintf(strBuf1, "%d%s%d", iLen, strInput, iLen);//字串首位都加上长度信息

	Calculate(strBuf1, GENE, strBuf2, "*");//字串乘以加密因子

	iLen = strlen(strBuf2);
	int i, j;
	for(i = 0, j = iLen-1; i < iLen; i++, j--)
	{
		strBuf1[i] = strBuf2[j];	
	}

	strBuf1[i] = 0;
	
	return Encrypt(strBuf1, strOutput);
}

int CEncryptAsc::UltraDecrypt(char* strInput, char* strOutput)
{
	char strBuf1[256];
	char strBuf2[256];
	int iLen;

	int nRet = Decrypt(strInput, strBuf1);
	if(nRet < 0)
		return nRet;

	iLen = strlen(strBuf1);
	int i, j;
	for(i = 0, j = iLen-1; i < iLen; i++, j--)
	{
		strBuf2[i] = strBuf1[j];	
	}

	strBuf2[i] = 0;
	
	Calculate(strBuf2, GENE, strBuf1, "/");
	
	iLen = strlen(strBuf1);

	memset(strOutput, 0, sizeof(strOutput));
	strncpy(strOutput, strBuf1 + 2, iLen - 4);//去掉字串首尾的长度信息
	
	return 0;
}

/////////////////////////////////
//main()
/////////////////////////////////
/*int main(int argc, char* argv[])
{	
	char sInput[256] = "", sOutput[256] = "", sBuf[256] = "";
	CEncryptAsc ce;

	while(1)
	{
		cout << "Please input pwd([exit] to exit): ";
		cin >> sInput;

		if(!strcmp(sInput, "exit")) break;	

		if (!strcmp(sInput,"space")) strcpy(sInput, " ");
		if (!strcmp(sInput,"null")) sInput[0] = 0;
		
		if( !ce.Encrypt(sInput, sOutput) ) {
			cout << "Encrypt success: " << sOutput << endl;						
		}
		else cout << "Encrypt fail" << endl;
		
		if( !ce.Decrypt(sOutput, sBuf) ) {
			cout << "Decrypt success: " << sBuf << endl << endl << endl;
		}
		else cout << "Decrypt fail" << endl << endl << endl;
	}
	
	return 0;
}*/
