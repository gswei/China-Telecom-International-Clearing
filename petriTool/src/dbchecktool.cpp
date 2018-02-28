#include "dbchecktool.h"

bool dbchecktool::readConfig( std::string & t_param_id )
{
    std::cout << "��ȡ����������Ϣ" << std::endl;
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
           writelog( 0, "����PARAM_ID="+t_param_id+"��PW_SQL_LOG��δ����" );
		   return false;
        }
        _task_config.reset();
		
        stmt >> _task_config.param_id >> _task_config.config_sql >> _task_config.operation
             >> _task_config.expect_result >> _task_config.success_log >> _task_config.failed_log;

        conn.close();
    } catch( SQLException & e ) {
        std::cout << "��ȡ��������ʱ�����ݿ�����쳣: " << e.what() << std::endl;

        sprintf( buffer, "��ȡ��������ʱ�����ݿ�����쳣:%s,%s", e.what(), _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    } catch( std::exception & e ) {
        std::cout << "�����쳣: " << e.what() << std::endl;

        sprintf( buffer, "�����쳣:%s,%s", e.what(), _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    } catch( ... ) {
        std::cout << "����δ֪�쳣: " << std::endl;

        sprintf( buffer, "����δ֪�쳣:%s", _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    }

    return true;
}

bool dbchecktool::processTask()
{
    std::cout << "��������" << std::endl;
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
        std::cout << "����δ֪�쳣: " << std::endl;
        sprintf( buffer, "����δ֪�쳣:%s", _task_config.failed_log );
        writelog( LOG_CODE_DB_EXECUTE_ERR, buffer );
        return false;
    }
}

int main( int argc, char *argv[] )
{
    std::string param_id = "";
    int n_log_type = 0;			 //��־���ͣ�0 ������1 ����ȱʧ��2 ��������3 ������֧��
    std::string s_log_msg = "";	 //��־�����Ϣ

    if( argc == 1 ) {
        std::cout << "�����ʽ��" << argv[0] << " -p[param_id] [-d|-h]" << std::endl;
        std::cout << "����ʹ�÷�������ͨ������" << argv[0] << " -h��ѯ�����ʽ˵��" << std::endl;
        n_log_type = 1;
        s_log_msg = std::string( argv[0] ) + std::string( "����ȱ�ٱ���������������-p" );
    }

    bool b_debug = false;

    for( int i = 1; i < argc; i++ ) {
        std::string sarg = std::string( argv[i] );

        if( sarg.substr( 0, 1 ) == "-" ) {
            if( sarg.substr( 1, 1 ) == "h" ) {
                std::cout << "���ݿ�����У�����" << "\n"
                          << "�����ʽ��" << argv[0] << " -p[param_id] [-d|-h]" << "\n"
                          << "��ѡ������"  << "\n"
                          << "      -p �������param_id�����磺-pr1 "  << "\n"
                          << "��ѡ������"  << "\n"
                          << "      -d �������й����У���ʾ������Ϣ "  << "\n"
                          << "������Ϣ��PW_SQL_LOG " << "\n"
                          << std::endl;
                return 1;
            } else if( sarg.substr( 1, 1 ) == "p" ) {
                if( sarg.length() >= 3 ) {
                    param_id = sarg.substr( 2, sarg.length() - 2 );
                } else {
                    std::cout << "����Ų���ȷ�� -pXXX,����ͨ������" << argv[0] << " -h��ѯ�����ʽ˵��" << std::endl;
                    n_log_type = 2;
                    s_log_msg = "����Ų���ȷ�� -pXXX";
                    break;
                }
            } else if( sarg.substr( 1, 1 ) == "d" && sarg.length() == 2 ) {
                b_debug = true;
            } else {
                std::cout << "��֧�ֵĲ���" << sarg << ",��ͨ������" << argv[0] << " -h��ѯ�����ʽ˵��" << std::endl;
                n_log_type = 3;
                s_log_msg = "��֧�ֵĲ���" + sarg;
                break;
            }
        } else {
            std::cout << "��֧�ֵĲ���" << sarg << ",��ͨ������" << argv[0] << " -h��ѯ�����ʽ˵��" << std::endl;
            n_log_type = 3;
            s_log_msg = "��֧�ֵĲ���" + sarg;
            break;
        }
    }

    //����"��־��������ʼ���߼�"
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

