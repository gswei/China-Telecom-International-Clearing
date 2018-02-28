/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG YOSON TECHNOLOGY CO., LTD.
All rights reserved.

Created:		2012-1-12
File:			example.cpp
$Id: example.cpp 1797 2012-10-27 08:39:53Z raozh $
Description:	����������Ҫ�漰�������ݣ�
				.���ݿ����
				.�ļ���д������STL��
				.XML����������POCO��
				.DataTable�ڴ��
				.��ʽ����
History:
  <table>
	revision	author            date                description
	--------    ------            ----                -----------
	v1.0		��־��          2012-8-21			��������������

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

    //1.���ݿ����(��ȡ)
    std::cout << "1.���ݿ��������" << std::endl;
    DBConnection conn;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select sysdate, 'test' as tcol from dual";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            std::string sysdate, tmpstr;

            while( stmt >> sysdate >> tmpstr ) {
                //���԰������������Ļ���������ɵ���
                std::cout << "��ѯ���: " << sysdate << " " << tmpstr << std::endl;
            }

            conn.close();
        }
    }

    //2.���ļ�������STL����д��ο�STL�ֲ�
    std::cout << "2.���ļ�����" << std::endl;
    char line[ReadFileLineBufferSize];   //�����л���
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
            //��ȡ�ļ���
            std::memset( line, 0, ReadFileLineBufferSize );
            infile.getline( line, ReadFileLineBufferSize );

            //����������ע����
            if( line[0] != '\0' && line[0] != '#' ) {
                //�����ݴ����߼� begin
                //...���崦���߼�ʡ��
                //�����ݴ����߼� end
            }

            rowCnt++;
            totalRowCnt++;
        }

        std::cout << "��ȡ�ļ�" << curFile << "������" << rowCnt << "��" << std::endl;

        rowCnt = 0;
        fileCnt++;
    }

    std::cout << "����ȡ�ļ�" << fileCnt << "��������" << totalRowCnt << "��" << std::endl;

    //3.XML����������POCO��
    //�������ˣ���ο�POCO�Ŀ����ֲ�
    std::cout << "3.XML��������" << std::endl;


    //4.DataTable�ڴ��
    std::cout << "4.DataTable����" << std::endl;
    DataTable* pTable = new DataTable( "test" );

    try {
        //4.1����
        DataColumn* pNewColumn1 = new DataColumn( "acc_nbr", ColumnDataType::String );
        pTable->AddColumn( pNewColumn1 );
        DataColumn* pNewColumn2 = new DataColumn( "amount", ColumnDataType::Integer );
        pTable->AddColumn( pNewColumn2 );
        //4.2����������
        DataRow row1 = pTable->NewDataRow();
        row1["acc_nbr"] = "18912345678";
        row1["amount"] = "18900";
        DataRow row2 = pTable->NewDataRow();
        row2["acc_nbr"] = "18987654321";
        row2["amount"] = "12900";
        //4.3��������
        std::cout << "��ӡDataTable���������£�" << std::endl;

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

    //4.*DataTable������������ο����ͷ�ļ�


    //5.��ʽ����
    std::cout << "5.��ʽ��������" << std::endl;
    es::FormulaParser fParser;
    fParser.compile( "a+b" );
    fParser.setInt( "a", 3 );
    fParser.setInt( "b", 1 );
    fParser.evaluate();
    int evalResullt = fParser.getInt();
    std::cout << "��ʽ(a+b)�Ĳ���ֵ(a=3,b=1)��Ӧ�ļ�����Ϊ" << evalResullt << std::endl;
}
