#include <iostream>
#include <string>
#include <utility>
#include <unistd.h>

void ExportHeader( std::string& header, const std::string& delimiter = "|" )
{
    header = "acct_item_id"			+ delimiter + "serv_id"			+ delimiter + "billing_cycle_id"			+ delimiter + "acct_item_type_id"			+ delimiter + "charge|acct_id"			+ delimiter + "offer_inst_id"			+ delimiter + "item_source_id"			+ delimiter + "offer_id"			+ delimiter + "create_date";
}

int main( int argc, char** argv )
{
    //std::string tmp_str = "123";
    //std::pair<int, std::string> tmp = std::make_pair(999, tmp_str);
    //std::cout << tmp.first << tmp.second << std::endl;

    /*
    printf("argc = %d, argv[0] = %s, argv[1] = %s \n", argc, argv[0], argv[1]);
    if (argc == 2)
    {
    std::string path(argv[1]);
    std::cout << path << std::endl;
    if (access(argv[1], F_OK) < 0)
    {
      printf("path %s not exists \n", argv[1]);
    }
    else
    {
      printf("path exists \n");
    }
    }
    */

    std::string line;
    ExportHeader( line );
    std::cout << line << std::endl;
}
