#include <iostream>
#include <string>
#include <utility>

int main( int argc, char** argv )
{
    std::string tmp_str = "123";
    std::pair<int, std::string> tmp = std::make_pair( 999, tmp_str );
    std::cout << tmp.first << tmp.second << std::endl;
}
