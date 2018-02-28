#include "dbchecktool.h"

bool dbchecktool::readConfig( std::string & t_param_id )
{
    std::cout << "读取任务配置信息" << std::endl;
    char buffer[128];
    memset( buffer, 0, sizeof( buffer ) );

    try {
        DBConnection conn;
        dbConnect( conn );
        Statement stmt = conn.createStatement();

        std::string attr_element = "";
        std::string sql = " select param_id,sql,operator,result_value,success_log,fail_log "\
                          "  from PW_SQL_LOG where param_id = :v0 ";
        stmt.setSQLString( sql );
        stmt << t_param_id;
        stmt.execute();
        //writelog( 0, std::string( "sql:" ) + sql );
        if(stmt.getCompleteRows()==0)
		{
           writelog( 0, "输入PARAM_ID="+t_param_id+"在PW_SQL_LOG中未配置" );
		   return false;
        }
        _task_config.reset();
		
        stmt >> _task_config.param_id >> _task_config.config_sql >> _task_config.operation
             >> _task_config.expect_result >> _task_config.success_log >> _task_config.failed_log;

        conn.close();
    } catch( SQLException & e ) {
        std::cout << "读取任务配置时，数据库操作异常: " << e.what() << std::endl;

        sprintf( buffer, "读取任务配置时，数据库操作异常:%s,%s", e.what(), _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    } catch( std::exception & e ) {
        std::cout << "捕获异常: " << e.what() << std::endl;

        sprintf( buffer, "捕获异常:%s,%s", e.what(), _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    } catch( ... ) {
        std::cout << "捕获未知异常: " << std::endl;

        sprintf( buffer, "捕获未知异常:%s", _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    }

    return true;
}

bool dbchecktool::processTask()
{
    std::cout << "处理任务" << std::endl;
	char buffer[128];
    memset( buffer, 0, sizeof( buffer ) );
	
    try {
        DBConnection conn;
        dbConnect( conn );
        Statement stmt = conn.createStatement();

        long t_result = 0;
        stmt.setSQLString( _task_config.config_sql );
		std::cout<<"sql:"<<_task_config.config_sql<<" operator:"<<_task_config.operation<<" result_value:"<<_task_config.expect_result<<std::endl;
        stmt.execute();
        stmt >> t_result;

        bool result_flag = false;

        if( _task_config.operation == "!="
                && t_result != _task_config.expect_result ) {
            result_flag = true;
        } else if( _task_config.operation == "="
                   && t_result == _task_config.expect_result ) {
            result_flag = true;
        } else if( _task_config.operation == "<"
                   && t_result < _task_config.expect_result ) {
            result_flag = true;
        } else if( _task_config.operation == ">"
                   && t_result > _task_config.expect_result ) {
            result_flag = true;
        }

        if( result_flag ) {
            writelog( 0, _task_config.success_log );
        } else {
            writelog( 0, _task_config.failed_log );
        }

        conn.close();
		return result_flag;
    } catch( SQLException & e ) {
        std::cout << "sql exception: " << e.what() << std::endl;
        sprintf( buffer, "sql exception(%s),%s", e.what(), _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    } catch( std::exception & e ) {
        std::cout << "exception: " << e.what() << std::endl;
        sprintf( buffer, "exception:%s,%s", e.what(), _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    } catch( ... ) {
        std::cout << "捕获未知异常: " << std::endl;
        sprintf( buffer, "捕获未知异常:%s", _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    }
}

int main( int argc, char *argv[] )
{
    std::string param_id = "";
    int n_log_type = 0;			 //日志类型：0 正常，1 参数缺失，2 参数错误，3 参数不支持
    std::string s_log_msg = "";	 //日志输出信息

    if( argc == 1 ) {
        std::cout << "命令格式：" << argv[0] << " -p[param_id] [-d|-h]" << std::endl;
        std::cout << "具体使用方法可以通过命令" << argv[0] << " -h查询命令格式说明" << std::endl;
        n_log_type = 1;
        s_log_msg = std::string( argv[0] ) + std::string( "命令缺少必须参数：任务序号-p" );
    }

    bool b_debug = false;

    for( int i = 1; i < argc; i++ ) {
        std::string sarg = std::string( argv[i] );

        if( sarg.substr( 0, 1 ) == "-" ) {
            if( sarg.substr( 1, 1 ) == "h" ) {
                std::cout << "数据库数据校验程序" << "\n"
                          << "命令格式：" << argv[0] << " -p[param_id] [-d|-h]" << "\n"
                          << "必选参数："  << "\n"
                          << "      -p 输入参数param_id，例如：-pr1 "  << "\n"
                          << "可选参数："  << "\n"
                          << "      -d 程序运行过程中，显示调试信息 "  << "\n"
                          << "任务信息表：PW_SQL_LOG " << "\n"
                          << std::endl;
                return 1;
            } else if( sarg.substr( 1, 1 ) == "p" ) {
                if( sarg.length() >= 3 ) {
                    param_id = sarg.substr( 2, sarg.length() - 2 );
                } else {
                    std::cout << "任务号不正确： -pXXX,详情通过命令" << argv[0] << " -h查询命令格式说明" << std::endl;
                    n_log_type = 2;
                    s_log_msg = "任务号不正确： -pXXX";
                    break;
                }
            } else if( sarg.substr( 1, 1 ) == "d" && sarg.length() == 2 ) {
                b_debug = true;
            } else {
                std::cout << "不支持的参数" << sarg << ",请通过命令" << argv[0] << " -h查询命令格式说明" << std::endl;
                n_log_type = 3;
                s_log_msg = "不支持的参数" + sarg;
                break;
            }
        } else {
            std::cout << "不支持的参数" << sarg << ",请通过命令" << argv[0] << " -h查询命令格式说明" << std::endl;
            n_log_type = 3;
            s_log_msg = "不支持的参数" + sarg;
            break;
        }
    }

    //增加"日志管理对象初始化逻辑"
    initializeLog( argc, argv, b_debug );

    switch( n_log_type ) {
        case 1:
            writelog( LOG_CODE_APP_PARAM_LACK, s_log_msg );
            return 1;

        case 2:
            writelog( LOG_CODE_APP_PARAM_VALUE_ERR, s_log_msg );
            return 1;

        case 3:
            writelog( LOG_CODE_APP_PARAM_NONSUPPORT, s_log_msg );
            return 1;
    }

    dbchecktool t_tool;

    if( !t_tool.readConfig( param_id ) ) {
        return 1;
    }

    if( !t_tool.processTask() ) {
        return 1;
    }

    return 0;
}

