/****************************************************************
filename: CF_LackInfo_Code.h
module: CF - Common Function
created by: weixy
create date: 2007-11-19
update list: 
version: 1.0.0
description:
    the header file of the classes for lack or abnormal
    �������ͱ��룺ABBCCDD
    A��1��ʾ�����ϣ�2��ʾ�쳣3 ��ʾ�澯
    BB��ҵ����࣬�μ�SERVICE_CAT���LACKINFO_CODE�ֶ�
    CC����ʾ������
    	00 :   �����쳣& �����쳣
    	10������
    	11������
    	12������
    	13���ƷѺ���
    	14����3������
    	15��������
    	16��SP����
    	17��������
    	18:    ��������ҵ��
    	19:    ���ڷ���
    	20��·��
    	21����·��
    	22����·��
    	31������
    	41��������
    	51���ײ�����
    	52����������
    	53��ͨ��ʱ��
    	61���û����
    	91����������
    	92���Ʒ����
    	93��������
    	94��ҵ�����
    	95��ҵ�����
    	96���Ʒ�ģʽ
    	97����������
    	99������
    DD:���    		
*****************************************************************/

#ifndef _CF_LACKINFO_CODE_H_
#define _CF_LACKINFO_CODE_H_ 1

/*������������ϴ���*/
const int LACK_CALLING_GSM2TOLLCODE				= 1001101;
const int LACK_CALLING_TOLLCODE 					= 1001102;
const int LACK_CALLING_TELENO_PROPERTY	 	= 1001104;
const int LACK_CALLING_TELENO_DEFPRO	 	= 1001108;
//add by weixy 20080422
const int LACK_CALLING_FEE                                           =1001116;

const int LACK_CHARGE_GSM2TOLLCODE				= 1001301;
const int LACK_CHARGE_TOLLCODE						= 1001302;
const int LACK_CHARGE_TELENO_PROPERTY			= 1001304;
const int LACK_CHARGE_TELENO_DEFPRO	 	= 1001308;


const int LACK_CALLED_GSM2TOLLCODE				= 1001201;
const int LACK_CALLED_TOLLCODE 						= 1001202;
const int LACK_CALLED_TELENO_PROPERTY	 		= 1001204;
const int LACK_CALLED_TELENO_DEFPRO	 	= 1001208;

const int LACK_DEF_TOLLCODE								= 1001002;
const int LACK_TOLLCODE									  = 1001001;
/*·�ɷ���������*/
const int LACK_INROUTE_ROUTE							= 1002100;
const int LACK_OUTROUTE_ROUTE							= 1002200;
const int LACK_OUTROUTE_ROUTE_TYPE				= 1002201;
const int LACK_INROUTE_ROUTE_TYPE					= 1002101;
const int LACK_ROUTE_ROUTE_USEWAY					= 1002000;

/*��������*/
const int LACK_CPATTACH                                           =1001800;
const int LACK_CPID                                                    =1001801;
const int LACK_DOWN_RATE						=1001802;
const int LACK_UP_RATE							=1001803;
const int LACK_USER_PROPERTY					=1001804;

/*�쳣�������*/
const int ABN_CALLING_SHORTLEN	    			= 2001101;		
const int ABN_CALLING_GDLEN   						= 2001102;		
const int ABN_CALLING_GSMLEN   						= 2001104;		
const int ABN_CALLING_DIGIT 	  					= 2001116;		
const int ABN_CALLING_OCCOR_2TOLL1   			= 2001132;		
const int ABN_CALLING_OCCOR_2TOLL2   			= 2001164;		
const int ABN_CALLING_BEFTEL					=2001165;


const int ABN_CALLED_SHORTLEN	    				= 2001201;		
const int ABN_CALLED_GDLEN   							= 2001202;	
const int ABN_CALLED_GSMLEN   						= 2001204;		
const int ABN_CALLED_DIGIT 	  						= 2001216;		
const int ABN_CALLED_OCCOR_2TOLL1   			= 2001232;		
const int ABN_CALLED_OCCOR_2TOLL2   			= 2001264;		
const int ABN_CALLED_BEFTEL					=2001265;

const int ABN_CHARGE_SHORTLEN	    				= 2001301;		
const int ABN_CHARGE_GDLEN   							= 2001302;	
const int ABN_CHARGE_GSMLEN   						= 2001304;		
const int ABN_CHARGE_DIGIT 	  						= 2001316;		
const int ABN_CHARGE_OCCOR_2TOLL1   			= 2001332;		
const int ABN_CHARGE_OCCOR_2TOLL2   			= 2001364;		
const int ABN_CHARGE_BEFTEL					=2001365;

//modify by weixy 20080513
const int ABN_CALCULATE							=2000001;
const int ABN_FIELDLEN                                               =2000002;

//modify by weixy 
const int ABN_CALLING_GSMTOLLCODE   			= 3001108;		
const int ABN_CALLED_GSMTOLLCODE   				= 3001208;		
const int ABN_CHARGE_GSMTOLLCODE   				= 3001308;		
//end modify by weixy 

const int LACK_RATECYCLE=1001900;

//modify by weixy 
const int ABN_CALLING_138   				= 2001108;		
const int ABN_CALLED_138   				= 2001208;		
const int ABN_CHARGE_138   				= 2001308;		
//end modify by weixy 

//add by weixy 20080526
const int LACK_DURATION = 1000003;
const int ABN_DURATION   =  3000003;
//end add by weixy 20080526


//spjf
const int LACK_TELENO_SERVICE				=	1001901;
const int ABN_CALLED_LEN					= 2001203;	
const int LACK_CARD_LOCATION				= 1003101;

const int LACK_CALLING_REGION 				= 1001164;
const int LACK_CALLED_REGION 				= 1001264;

/*
#define CALLINGGSM_LACK       1
#define CALLINGTOLLCODE_LACK  2
#define CALLINGTELENO_LACK    4
#define CALLINGSEG_LACK       8
#define CALLINGREG_LACK       16
#define CALLINGBEFSVR_LACK    32

#define CALLEDGSM_LACK        1000
#define CALLEDTOLLCODE_LACK   2000
#define CALLEDTELENO_LACK     4000
#define CALLEDSEG_LACK        8000
#define CALLEDREG_LACK        16000
#define CALLEDBEFSVR_LACK     32000

#define CALLING_LOCLACK	      1024
#define CALLED_LOCLACK        1024000

#define OUT_CYCYLE	      128
#define INROUTE_LACK	      64
#define OUTROUTE_LACK	      64000

#define INROUTETYPE_LACK      256
#define OUTROUTETYPE_LACK     256000
#define ROUTE_USEWAY_LACK     512

#define REGION_LACK	      2048
  */



#endif

