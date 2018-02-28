/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG YOSON TECHNOLOGY CO., LTD.
All rights reserved.

Created:		2012-1-12
File:			example.cpp
$Id: example.cpp 1797 2012-10-27 08:39:53Z raozh $
Description:	样例程序，主要涉及以下内容：
				.数据库操作
				.文件读写（基于STL）
				.XML解析（基于POCO）
				.DataTable内存表
				.公式解析
History:
  <table>
	revision	author            date                description
	--------    ------            ----                -----------
	v1.0		饶志华          2012-8-21			完成样例程序初稿

  </table>
**************************************************************************/

#include "psutil.h"
#include "Poco/Glob.h"
#include "dataset/DataTable.h"
#include "es/util/parser/FormulaParser.h"
#include <iostream>
#include <fstream>

using datasource::DataSet;
using datasource::DataTable;
using datasource::DataRow;
using datasource::DataColumn;
using datasource::ColumnDataType;
using namespace tpss;

const int ReadFileLineBufferSize = 40000;

int main( int argc, char** argv )
{
    writelog( 1001, "test error desc" );

    //1.数据库操作(读取)
    std::cout << "1.数据库操作举例" << std::endl;
    DBConnection conn;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select sysdate, 'test' as tcol from dual";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            std::string sysdate, tmpstr;

            while( stmt >> sysdate >> tmpstr ) {
                //测试案例：输出到屏幕；具体程序可调整
                std::cout << "查询结果: " << sysdate << " " << tmpstr << std::endl;
            }

            conn.close();
        }
    }

    //2.读文件（基于STL），写请参考STL手册
    std::cout << "2.读文件举例" << std::endl;
    char line[ReadFileLineBufferSize];   //定义行缓存
    int totalRowCnt = 0, rowCnt = 0, fileCnt = 0;
    std::string homePath = getenv( "HOME" );
    std::string dataFile = homePath.append( "/optps/src/app/project/example/testdata/a.txt" );
    std::cout << "filepath=" << dataFile << std::endl;
    std::set<std::string> files;
    Poco::Glob::glob( dataFile, files );

    for( std::set<std::string>::const_iterator it = files.begin(); it != files.end(); it++ ) {
        std::string curFile = *it;
        std::ifstream infile( curFile.c_str() );

        while( !infile.eof() ) {
            //读取文件行
            std::memset( line, 0, ReadFileLineBufferSize );
            infile.getline( line, ReadFileLineBufferSize );

            //跳过空行与注释行
            if( line[0] != '\0' && line[0] != '#' ) {
                //行数据处理逻辑 begin
                //...具体处理逻辑省略
                //行数据处理逻辑 end
            }

            rowCnt++;
            totalRowCnt++;
        }

        std::cout << "读取文件" << curFile << "的数据" << rowCnt << "行" << std::endl;

        rowCnt = 0;
        fileCnt++;
    }

    std::cout << "共读取文件" << fileCnt << "个，数据" << totalRowCnt << "行" << std::endl;

    //3.XML解析（基于POCO）
    //不举例了，请参看POCO的开发手册
    std::cout << "3.XML解析举例" << std::endl;


    //4.DataTable内存表
    std::cout << "4.DataTable举例" << std::endl;
    DataTable* pTable = new DataTable( "test" );

    try {
        //4.1构建
        DataColumn* pNewColumn1 = new DataColumn( "acc_nbr", ColumnDataType::String );
        pTable->AddColumn( pNewColumn1 );
        DataColumn* pNewColumn2 = new DataColumn( "amount", ColumnDataType::Integer );
        pTable->AddColumn( pNewColumn2 );
        //4.2填充测试数据
        DataRow row1 = pTable->NewDataRow();
        row1["acc_nbr"] = "18912345678";
        row1["amount"] = "18900";
        DataRow row2 = pTable->NewDataRow();
        row2["acc_nbr"] = "18987654321";
        row2["amount"] = "12900";
        //4.3遍历数据
        std::cout << "打印DataTable的数据如下：" << std::endl;

        for( int rowIndex = 0; rowIndex < pTable->GetRowsCount(); rowIndex++ ) {
            DataRow& tmpRow = pTable->GetDataRow( rowIndex );
            std::string acc_nbr = static_cast<std::string>( tmpRow["acc_nbr"] );
            std::cout << acc_nbr << " ";
            std::string amount = static_cast<std::string>( tmpRow["amount"] );
            std::cout << amount << " ";

            std::cout << std::endl;
        }

        delete pTable;
    } catch( ... ) {
        delete pTable;
    }

    //4.*DataTable的其它功能请参看相关头文件


    //5.公式解析
    std::cout << "5.公式解析举例" << std::endl;
    es::FormulaParser fParser;
    fParser.compile( "a+b" );
    fParser.setInt( "a", 3 );
    fParser.setInt( "b", 1 );
    fParser.evaluate();
    int evalResullt = fParser.getInt();
    std::cout << "公式(a+b)的参数值(a=3,b=1)对应的计算结果为" << evalResullt << std::endl;
}
