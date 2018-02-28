#include "psutil.h"
#include "Poco/Glob.h"
#include "tp_log_code.h"

#include <string>
#include <iostream>
#include <exception>

using namespace tpss;

struct TaskConfig {
    std::string  param_id;
    std::string  config_sql;
    std::string  operation;
    long 		 expect_result;
    std::string  success_log;
    std::string  failed_log;

    void reset() {
        param_id = "";
        config_sql = "";
        operation = "";
        expect_result = 0;
        success_log = "";
        failed_log = "";
    }
};

class dbchecktool
{
public:
    dbchecktool() {};
    ~dbchecktool() {};

public:
    bool readConfig( std::string & t_param_id );
    bool processTask();

    /* data */
private:
    TaskConfig _task_config;
};
