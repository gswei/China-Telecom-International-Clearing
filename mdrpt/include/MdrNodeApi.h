// $Header$
/* vim:set ts=8 sw=4: */

#ifndef _MdrNodeApi_h_
#define _MdrNodeApi_h_

#include <sys/time.h>
#include <string>
#include <vector>

class MdrAdtInfRec;

struct MdrAuditInfo {		// 仲裁、提交API用到的结构体
public:
    MdrAuditInfo();
    ~MdrAuditInfo();
    std::string toStr() const;	// for debug convenience
    void toStr(std::string& debug_str) const;
    friend class MdrAdtInfRec;
public:
    std::string node;		// 业务网元名称,不带域名,不能为空
    std::string srvContext;	// 业务能力,不带网元、域名,不能为空
    std::string auditKey;	// 配对串,不能为空
    std::string auditVal;	// 仲裁串,不能为空
    std::string sessionId;	// 不能为空
    int         rflag;		// 2:syncVar, 1:CCR, 0:CCA
    std::string ccrEvtTime;	// 14位时戳,精确到秒 yyyymmddHHMMSS
    std::string ccrRcvTime;	// 21位时戳,精确到微秒 yyyymmddHHMMSS.ssssss
    std::string syncVar;	// 随机变量,可空
    int         result;		// mdr_Audit的输出参数; mdr_CmtResult的输入参数
private:			// API内部friend类使用的字段
    struct timeval adtTime;
    struct timeval cmtTime;
    struct timeval sndTime;
    int64_t dbId;
    std::string msgSeqId;
};

struct MdrVarInfo {		// mdr_SyncVarList API用到的结构体
public:
    MdrVarInfo() {};
    MdrVarInfo(const MdrVarInfo& obj) :
	sKey(obj.sKey), sSessionID(obj.sSessionID), syncVar(obj.syncVar) {};
    ~MdrVarInfo() {};
    MdrVarInfo& operator=(const MdrVarInfo& obj);
    std::string toStr() const;	// for debug convenience
    void toStr(std::string& debug_str) const;
public:
    std::string sKey;		//配对串
    std::string sSessionID;	//session id
    std::string syncVar;	//主系统同步的信息，例如扣费信息，字符串，格式自定义
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

// 参数is_msg == true : 与MdrMgr建立连接
// 参数is_msg == false: 不与MdrMgr建立连接
MdrRetCode mdr_InitPlatform(bool is_msg = true);

MdrRetCode mdr_GetDRStatus(MdrStatus& dr_status);

MdrRetCode mdr_GetNodeType(MdrNodeType& node_type);

MdrRetCode mdr_GetNodeTypeSR(MdrNodeType& node_type);

MdrRetCode mdr_Audit(MdrAuditInfo& audit_info);

MdrRetCode mdr_CmtResult(const MdrAuditInfo& audit_info);

// 简介：批量获取主系统同步的信息，例如扣费信息
// 输出参数： varList
MdrRetCode mdr_SyncVarList(std::vector<MdrVarInfo>& varList);

#endif

