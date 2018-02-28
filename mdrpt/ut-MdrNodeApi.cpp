// $Header$

//#include <Timestamp.h>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <MdrNodeApi.h>

int _Loops = 0;
int _Pause = 0;

void usage(const char* exename);
void pause(const char* fmt, ...);
MdrStatus getters();
void masterAudit(MdrAuditInfo& audit_info, int count);
void cmtResult(const MdrAuditInfo& audit_info);
void fillMasterAuditInfo(MdrAuditInfo& audit_info, int count);
void slaveSyncVarList(std::vector<MdrVarInfo>& var_list);
void slaveAudit(const MdrVarInfo& var_info, MdrAuditInfo& audit_info);

int main(int argc, char* const* argv)
{
    if (argc != 3)
        usage(argv[0]);
    _Loops = atoi(argv[1]);
    _Pause = atoi(argv[2]);

    int c;
    MdrStatus stat_ = getters();
    if (stat_ == MDR_STATUS_SLAVE) {
        while (_Loops > 0) {
            std::vector<MdrVarInfo> var_list_;
            slaveSyncVarList(var_list_);
            for (int i = 0; i < var_list_.size(); ++ i) {
                MdrVarInfo var_info_ = var_list_.at(i);
                MdrAuditInfo audit_info_;

                pause("%s,%d-->press enter continue SLAVE mdr_Audit", __FILE__, __LINE__);
                slaveAudit(var_info_, audit_info_);

                pause("%s,%d-->press enter continue SLAVE mdr_CmtResult", __FILE__, __LINE__);
                cmtResult(audit_info_);
                _Loops --;
            }
        }
        printf("%s,%d-->SLAVE loops done\n", __FILE__, __LINE__);
    } else {
        for (int i = 1; i <= _Loops; ++ i) {
            pause("%s,%d-->press enter continue mdr_Audit", __FILE__, __LINE__);
            MdrAuditInfo audit_info_;
            masterAudit(audit_info_, i);

            pause("%s,%d-->press enter continue mdr_CmtResult", __FILE__, __LINE__);
            cmtResult(audit_info_);
        }
    }

    ::sleep(1);
    exit(0);
}

void usage(const char* exename)
{
    printf("usage: %s loops pause\n", exename);
    printf("eg   : %s 1 1    ## just once, pause for mdr_Audit & mdr_CmtResult\n", exename);
    printf("eg   : %s 10 0   ## 10 times, no pause\n", exename);
    exit(1);
}

void pause(const char* fmt, ...)
{
    if (_Pause == 0)
        return;
    char buf_[2048];
    if (fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf_, sizeof(buf_) - 1, fmt, ap);
        va_end(ap);
        printf("%s\n", buf_);
        int c = getchar();
    }
}

MdrStatus getters()
{
    MdrRetCode rc_;

    rc_ = mdr_InitPlatform();
    printf("%s,%d-->mdr_InitPlatform, rc_=%d, (rc=>0:SUCCESS,1:FAILURE,-1:INVALID)\n", __FILE__, __LINE__, rc_);
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_InitPlatform failed\n", __FILE__, __LINE__);
        exit(1);
    }

    MdrStatus stat_;
    rc_ = mdr_GetDRStatus(stat_);
    printf("%s,%d-->mdr_GetDRStatus, rc_=%d, stat_=%d, (stat=>0:MASTER,1:SLAVE,2:NODR)\n", __FILE__, __LINE__, rc_, stat_);
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_GetDRStatus failed\n", __FILE__, __LINE__);
        exit(1);
    }

    MdrNodeType node_type_;
    rc_ = mdr_GetNodeType(node_type_);
    printf("%s,%d-->mdr_GetNodeType, rc_=%d, node_type_=%d (node_type=>0:DUPLEX,1:SINGLE)\n", __FILE__, __LINE__, rc_, node_type_);
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_GetNodeType failed\n", __FILE__, __LINE__);
        exit(1);
    }

    rc_ = mdr_GetNodeTypeSR(node_type_);
    printf("%s,%d-->mdr_GetNodeTypeSR, rc_=%d, sr_node_type_=%d\n", __FILE__, __LINE__, rc_, node_type_);
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_GetNodeTypeSR failed\n", __FILE__, __LINE__);
        exit(1);
    }

    return stat_;
}

void masterAudit(MdrAuditInfo& audit_info, int count)
{
    fillMasterAuditInfo(audit_info, count);
    MdrRetCode rc_ = mdr_Audit(audit_info);
    std::string audit_info_str_;
    audit_info.toStr(audit_info_str_);
    printf("%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data());
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__);
        exit(1);
    }
}

void cmtResult(const MdrAuditInfo& audit_info)
{
    MdrRetCode rc_ = mdr_CmtResult(audit_info);
    std::string audit_info_str_;
    audit_info.toStr(audit_info_str_);
    printf("%s,%d-->mdr_CmtResult, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data());
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_CmtResult failed\n", __FILE__, __LINE__);
        exit(1);
    }
}

void fillMasterAuditInfo(MdrAuditInfo& audit_info, int count)
{
    int pid_ = getpid();
    char buf_[1024];
    audit_info.node = "TPSS1";
    audit_info.srvContext = "srvContext";
    sprintf(buf_, "auditKey_%d_%d", pid_, count);
    audit_info.auditKey = buf_;
    sprintf(buf_, "auditVal_%d_%d", pid_, count);
    audit_info.auditVal = buf_;
    sprintf(buf_, "sessionId_%d_%d", pid_, count);
    audit_info.sessionId = buf_;
    audit_info.rflag = 2;                       // must fill 2
    audit_info.syncVar = audit_info.auditVal;	// use '$syncVar' to transfer '$auditVal' to SLAVE

    struct timeval tv_;
    ::gettimeofday(&tv_, NULL);
    tv_.tv_sec --;
    struct tm tm_;
    ::localtime_r(&tv_.tv_sec, &tm_);
    char tmp_[64];
    strftime(tmp_, sizeof(tmp_), "%Y%m%d%H%M%S", &tm_);
    sprintf(buf_, "%s.%06d", tmp_, (int)(tv_.tv_usec % 1000000));
    audit_info.ccrRcvTime = buf_;

    tv_.tv_sec --;
    ::localtime_r(&tv_.tv_sec, &tm_);
    strftime(tmp_, sizeof(tmp_), "%Y%m%d%H%M%S", &tm_);
    sprintf(buf_, "%s.%06d", tmp_, (int)(tv_.tv_usec % 1000000));
    audit_info.ccrEvtTime = buf_;
    
    audit_info.result = 0;

    /*
    Timestamp now_(true);
    now_ += (-1000000) * 2;
    audit_info.ccrEvtTime = now_.toStr(Timestamp::TsFmt14, 6, '.');
    now_ += (-1000000);
    audit_info.ccrRcvTime = now_.toStr(Timestamp::TsFmt14, 6, '.');
    */
}

void slaveSyncVarList(std::vector<MdrVarInfo>& var_list)
{
    MdrRetCode rc_;
    while (true) {
        rc_ = mdr_SyncVarList(var_list);
        if (rc_ != MDR_SUCCESS) {
            printf("%s,%d-->HEY! mdr_SyncVarList error, rc_=%d\n", __FILE__, __LINE__, rc_);
            sleep(1);
            return;
        }
        if (var_list.size() > 0) {
            printf("%s,%d-->mdr_SyncVarList got %ld elems, %s\n",
                __FILE__, __LINE__, var_list.size(), var_list[0].toStr().data());
            return;
        }
        sleep(1);
    }
}

void slaveAudit(const MdrVarInfo& var_info, MdrAuditInfo& audit_info)
{
    audit_info.node = "TPSS2";
    audit_info.srvContext = "srvContext";
    audit_info.rflag = 2;                               // must fill 2
    audit_info.auditKey = var_info.sKey;
    audit_info.sessionId = var_info.sSessionID;
    audit_info.auditVal = var_info.syncVar;
    audit_info.ccrEvtTime = "20000101000000";           // SLAVE side: dummy value to pass fmt validation
    audit_info.ccrRcvTime = "20000101000000.000000";    // SLAVE side: dummy value to pass fmt validation

    MdrRetCode rc_ = mdr_Audit(audit_info);
    printf("%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info.toStr().data());
    if (rc_ != MDR_SUCCESS) {
        printf("%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__);
        exit(1);
    }
}

