// $Header$
/* vim:set ts=8 sw=4: */

#ifndef _MdrNodeApi_h_
#define _MdrNodeApi_h_

#include <sys/time.h>
#include <string>
#include <vector>

class MdrAdtInfRec;

struct MdrAuditInfo {		// �ٲá��ύAPI�õ��Ľṹ��
public:
    MdrAuditInfo();
    ~MdrAuditInfo();
    std::string toStr() const;	// for debug convenience
    void toStr(std::string& debug_str) const;
    friend class MdrAdtInfRec;
public:
    std::string node;		// ҵ����Ԫ����,��������,����Ϊ��
    std::string srvContext;	// ҵ������,������Ԫ������,����Ϊ��
    std::string auditKey;	// ��Դ�,����Ϊ��
    std::string auditVal;	// �ٲô�,����Ϊ��
    std::string sessionId;	// ����Ϊ��
    int         rflag;		// 2:syncVar, 1:CCR, 0:CCA
    std::string ccrEvtTime;	// 14λʱ��,��ȷ���� yyyymmddHHMMSS
    std::string ccrRcvTime;	// 21λʱ��,��ȷ��΢�� yyyymmddHHMMSS.ssssss
    std::string syncVar;	// �������,�ɿ�
    int         result;		// mdr_Audit���������; mdr_CmtResult���������
private:			// API�ڲ�friend��ʹ�õ��ֶ�
    struct timeval adtTime;
    struct timeval cmtTime;
    struct timeval sndTime;
    int64_t dbId;
    std::string msgSeqId;
};

struct MdrVarInfo {		// mdr_SyncVarList API�õ��Ľṹ��
public:
    MdrVarInfo() {};
    MdrVarInfo(const MdrVarInfo& obj) :
	sKey(obj.sKey), sSessionID(obj.sSessionID), syncVar(obj.syncVar) {};
    ~MdrVarInfo() {};
    MdrVarInfo& operator=(const MdrVarInfo& obj);
    std::string toStr() const;	// for debug convenience
    void toStr(std::string& debug_str) const;
public:
    std::string sKey;		//��Դ�
    std::string sSessionID;	//session id
    std::string syncVar;	//��ϵͳͬ������Ϣ������۷���Ϣ���ַ�������ʽ�Զ���
};

enum MdrRetCode {
    MDR_INVALID = -1,
    MDR_SUCCESS = 0,
    MDR_FAILURE = 1
};

enum MdrNodeType {
    MDR_NODE_DUPLEX = 0,
    MDR_NODE_SINGLE = 1
};

enum MdrStatus {
    MDR_STATUS_MASTER = 0,
    MDR_STATUS_SLAVE  = 1,
    MDR_STATUS_NODR   = 2
};

enum MdrAuditResult {
    MDR_AUDIT_SUC     = 1,
    MDR_AUDIT_FAILURE = 2,
    MDR_AUDIT_EXP     = 3
};

enum MdrCmtResult {
    MDR_CMT_SUC     = 0,
    MDR_CMT_FAILURE = 1
};

// ����is_msg == true : ��MdrMgr��������
// ����is_msg == false: ����MdrMgr��������
MdrRetCode mdr_InitPlatform(bool is_msg = true);

MdrRetCode mdr_GetDRStatus(MdrStatus& dr_status);

MdrRetCode mdr_GetNodeType(MdrNodeType& node_type);

MdrRetCode mdr_GetNodeTypeSR(MdrNodeType& node_type);

MdrRetCode mdr_Audit(MdrAuditInfo& audit_info);

MdrRetCode mdr_CmtResult(const MdrAuditInfo& audit_info);

// ��飺������ȡ��ϵͳͬ������Ϣ������۷���Ϣ
// ��������� varList
MdrRetCode mdr_SyncVarList(std::vector<MdrVarInfo>& varList);

#endif

