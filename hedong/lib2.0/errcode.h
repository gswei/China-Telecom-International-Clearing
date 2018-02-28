#ifndef _ERRCODE_H_
#define _ERRCODE_H_ 
/********************************************************************
 *	Usage		: �������ģ��Ĵ������
 *	Author		: ���Ǿ�
 *  Create date	: 2005-04-29
 *	Version		: 1.0.0
 *	Updatelist	:
 *			1)2005-05-08	���Ǿ�	�������Ͻӿ��е�ERR_FILE_OPEN(706)��Ϊ
 *									ERR_OPEN_FILE, �����ERR_FILE_OPEN(604)��ͻ;
 *									��OCCI���ݿ�ӿ��еĴ������Ϊ��ֵ;
 *	version: 1.1.2
 ********************************************************************/

//��̬�������漰�Ĵ�����루H��
#define COMPILE_ERROR_MISSING_RIGHT_BRACKET			101	//�����������Ų�ƥ��	
#define COMPILE_ERROR_NOT_EXPECTED_CHARACTER		102	//���������ֵķ���	
#define COMPILE_ERROR_AND_PARAM						103	//'&'�����������
#define COMPILE_ERROR_OR_PARAM						104	//'|'�����������
#define COMPILE_ERROR_MISSING_COMMA					105	//©��','����
#define COMPILE_ERROR_INVALID_PARAM					106	//��Ч�Ĳ���
#define COMPILE_ERROR_VARIABLE_NOT_DEFINE			107	//δ���������
#define COMPILE_ERROR_ZERO_DIVID					108	//0������
#define COMPILE_ERROR_FUNCTION_NOT_DEFINE			109	//δ����ĺ�����
#define COMPILE_ERROR_MISSING_SINGLE_QUOTATION_MARK 110	//ȱ���ҵ�����
#define COMPILE_ERROR_INVALID_NUMBER				111	//��Ч������,���溬����ĸ
#define COMPILE_ERROR_INVALID_EXP					112	//��Ч���ӱ��ʽ���߱��ʽΪ��
#define COMPILE_ERROR_CASEEXP_MISSING_SUBEXP		113	//case�������ʽȱ���ӱ��ʽ
#define COMPILE_ERROR_INEXP_MISSING_SUBEXP			114	//in�������ʽȱ���ӱ��ʽ
#define COMPILE_ERROR_SETEXP_FIRSTEXP_NOT_VAR		115	//set�����е�һ�����ʽ���Ǳ���
#define COMPILE_ERROR_DECODEEXP_MISSING_SUBEXP		116	//decode������ȱ���ӱ��ʽ
#define COMPILE_ERROR_COMMAEXP_MISSING_SUBEXP		117	//comma������ȱ���ӱ��ʽ
#define COMPILE_RUNTIME_ERROR_UNKNOWN				120	//��̬��������ʱ����
#define COMPILE_RUNTIME_ERROR_EXPOVERLIMIT			121	//���ʽ��Ŀ����255

//�����ǿ����ò�������漰�Ĵ������
#define	NOT_ENOUGH_MEMORY_TO_APPLY		201	//�����ڴ�ռ�ʱ����
#define SELECT_ERROR_FROM_DB			211 //��ѯ���ݿ����
#define INSERT_ERROR_TO_DB				212 //�������ݿ����
#define UPDATE_ERROR_TO_DB				213 //�������ݿ����
#define COMPILE_EXCUTE_ERROR			221	//������ִ������������
#define OPEN_PLUGIN_ERROR				231	//�򿪶�̬��������
#define CLOSE_PLUGIN_ERROR				232	//�رն�̬��������
#define CAN_NOT_FIND_PLUGIN_CLASS		233	//��̬������Ҳ����ò��
#define PLUGIN_STRING_DEFINE_ERR		234	//������崮��ʽ����
#define INIT_CFMT_CHANGE_ERROR			241	//��ʼ���������������¼ʵ������
#define ERR_FILETYPE_MATCH          242 //������ʽת����                          	

//CFileIn.cpp(M)                    	
#define ERR_IN_FILE_CLOSED				301	//�ļ����ڹر�״̬
#define ERR_IN_FILE_HEAD				302	//�ļ�ͷ����
#define ERR_IN_FILE_BYTE				303	//�ļ��ֽ���������
#define ERR_IN_FILE_END					304	//�ļ�β����
#define ERR_IN_FILE_NUM					305	//�ļ���¼��������
#define ERR_IN_FILE_OPEN				306	//���ļ�ʧ��
#define ERR_IN_READ_REC					307	//��ȡ��¼ʧ��
                                    	
//CFileOut.cpp(M)                   	
#define ERR_OUT_FILE_OPEN				311	//�ļ��򿪴���
#define ERR_OUT_WRITE_FILE_HEAD			312	//д�ļ�ͷ����
#define ERR_OUT_WRITE_REC				313	//�ļ�д��ʧ��
#define ERR_OUT_FILE_CLOSED				314	//�ļ����ڹر�״̬
#define ERR_OUT_WRITE_FILE_END			315	//�ļ�βдʧ��

//�ļ���¼��ʽ�ࣺ
#define ERR_TYPE						401	//��¼���ʹ���
#define ERR_REQ_MEM						402	//�����ڴ����
#define ERR_FILETYPEID					403	//��¼���Ͷ��岻��
#define ERR_COUNT_LESS					404	//��¼�ֶ�̫��
#define ERR_LENGTH_LONG					405	//��¼̫��
#define ERR_INPUTCOUNT_OVER_UPLIMIT		406	//�����ֶ�����������
#define ERR_INPUTLENGTH_OVER_UPLIMIT	407	//�����ֶγ��ȳ�������
#define ERR_INPUTLENGTH_LESS			408	//�����¼����С�ڶ���

//OCCI���ݿ�ӿڣ���H��
#define	INVALID_PARAMETER				-502	//��Ч�������
#define INVALID_PRE_CONDITION			-503	//ǰ�����������㣨��Ч���ã�
#define DATA_TYPE_NOT_SUPPORTED			-504	//�ݲ�֧�ֵ���������
#define DATA_TYPE_INVALID_CONVERSION	-505	//�޷�ת������������
#define INVALID_INPUT_BIND_VAR_COUNT	-506	//����󶨱�����Ŀ����
#define INVALID_OUTPUT_BIND_VAR_COUNT	-507	//�������ֵ������Ŀ����

//wangds������д
#define ERR_MMAP                  		601 //�����ڴ�ռ����
#define ERR_DEL_SPACE             		602 //�ͷ��ڴ�ռ����
#define ERR_FILELEN_CHANGE        		603 //�ı��ļ���С����
#define ERR_FILE_OPEN             		604 //���ļ�����
#define ERR_FOR_BILL_BE           		605 //�ļ�ͷβ��¼����
#define ERR_CLOSE_FILE            		606 //�ر��ļ�����       
#define ERR_FOR_ACCESS_FILE       		607 //�ļ�������
#define ERR_FILE                  		609 //�����ļ����ֽ���С��23
#define ERR_OPEN_FOR_READ_FAIL			610 //ֻ����ʽ���ļ�ʧ��
#define ERR_OPEN_FOR_WRITE_FAIL			611 //��д��ʽ���ļ�ʧ��
#define ERR_FILE_NOT_OPEN 	  			612 //�ļ����ڹر�״̬
#define ERR_RECORD_TOO_LONG         613//�ļ���¼̫��

//�����Ͻӿ�
#define ERR_FILE_WRITE					701 //�ļ���д���ʱ��������ϵͳæ����I/O����
#define ERR_GET_RECORD					702 //��ȡ�����ֶγ����ĳ��������CFmt_Change����
#define ERR_DIR_CREATE					703 //������Ŀ¼����������Ŀ¼ʱ��Ϊϵͳԭ������Ŀ¼���ɴ���
#define ERR_DIR_CHANGE					704 //������Ŀ¼����û��Ȩ�޷���ָ����Ŀ¼���û����ļ���Ȩ�޲���
#define ERR_DIR_NULLITY					705 //���������й�����Ҫ��ʹ�þ���·����
#define ERR_OPEN_FILE					706 //�ļ��򿪴����û�û��Ȩ�޴򿪸��ļ�
#define ERR_FILE_CLOSE					707 //�ļ��ر�ʱ����ϵͳæ�����ļ����ƶ�
#define ERR_RENAME_FILE					708 //���ļ���������ϵͳæ��������ʱ�ļ����ƶ�
#define ERR_REMOVE_FILE					709 //ɾ����ʱ�ļ�ʱ����ϵͳæ��������ʱ�ļ����ƶ�
#define ERR_DIR_OPEN					710 //ָ����Ŀ¼�����ڣ������û�û��Ȩ�޷��ʸ�Ŀ¼

//��ˮ������Ϣ�����ȱ���Ϣ�ӿ�
#define SCHINFO_ERR_CONFIG_PIPE				801 //PIPE�����Ҳ�����Ӧ����
#define SCHINFO_ERR_CONFIG_INTERFACE		802 //INTERFACE�����Ҳ�����Ӧ����
#define SCHINFO_ERR_CONFIG_MODEL_INTERFACE	803 //MODEL_INTERFACE�����Ҳ�����Ӧ����
#define SCHINFO_ERR_CONFIG_FILETYPE_DEFINE	804 //FILETYPE_DEFINE�����Ҳ�����Ӧ����
#define SCHINFO_ERR_CONFIG_SOURCE			804 //SOURCE�����Ҳ�����Ӧ����
#define SCHINFO_ERR_UPDATE_TABLE			805 //������ڵ��ȱ�ʧ��
#define SCHINFO_ERR_INSERT_TABLE			806 //����ڵ��ȱ�����¼ʧ��

//���̼����Ϣ�����ڴ�ӿ�
#define PROCINFO_ERR_CREATE_FAIL			901 //���������Ϣ�����ڴ�ʧ��
#define PROCINFO_ERR_DESTROY_FAIL			902 //ɾ�������Ϣ�����ڴ�ʧ��
#define PROCINFO_ERR_ATTACH_FAIL			903 //���Ӽ����Ϣ�����ڴ�ʧ��
#define PROCINFO_ERR_DETACH_FAIL			904 //�Ͽ��ͼ����Ϣ�����ڴ�����ʧ��
#define PROCINFO_ERR_READ_FAIL				904 //��ȡ�����Ϣ�����ڴ�ʧ��
#define PROCINFO_ERR_WRITE_FAIL				905 //д������Ϣ�����ڴ�ʧ��
#define PROCINFO_ERR_MEMIDX					906 //����Ľ���������
#define PROCINFO_ERR_PROCESS_ALREADY_RUN	907 //���н���������

//������־�ӿڴ�������
#define DEALLOG_ERR_IN_OPEN_FILE      1001
#define DEALLOG_ERR_IN_WRITE_FILE     1002
#define DEALLOG_ERR_IN_GET_ENV        1003

#endif//_ERRCODE_H_
