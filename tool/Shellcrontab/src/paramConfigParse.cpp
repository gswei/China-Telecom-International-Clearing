#include "paramConfigParse.h"
#include "Poco/String.h"
#include "Poco/DateTimeParser.h"
#include "Poco/NumberFormatter.h"
#include "Poco/NumberParser.h"
#include <Poco/Environment.h>
#include "Poco/Util/SystemConfiguration.h"
#include "Poco/Util/MapConfiguration.h"
#include "Poco/Util/Application.h"
#include "LatnStruct.h"



paramParse::paramParse()
{
    config = new paramConfiguration();
    config->add( new Poco::Util::SystemConfiguration, Poco::Util::Application::PRIO_SYSTEM, false, false );
    config->add( new Poco::Util::MapConfiguration, Poco::Util::Application::PRIO_APPLICATION, true, false );
}
paramParse::~paramParse()
{
    if ( !config )
    {
        delete config;
    }
}
void paramParse::init( int argc, char** argv )
{
    std::string paramShortName, paramValue, paramName, tempParamValue;
    for ( int i = 1; i < argc; i++ )
    {
        tempParamValue = std::string( argv[i] );
     /*   if ( tempParamValue.substr( 0, 2 ) == "-l" )
        {
            paramValue = tempParamValue.substr( 2, tempParamValue.length() );
            int ctg_corp_id( 0 ), city_id( 0 );
            std::string corp_name;



            bool flag = false;
            for ( int j = 0; j < sizeof( sLatnInfo ) / sizeof( sLatnInfo[0] ); j++ )
            {
                if ( paramValue == sLatnInfo[ j ].latn )
                {
                    flag = true;
                    setInt( "CITY_ID", sLatnInfo[ j].id );
                    setInt( "CORP_ID", sLatnInfo[ j ].id );
                    setString( "POST_CODE", sLatnInfo[ j ].post );
                    setInt( "BW_CODE", sLatnInfo[ j ].bw_code );
                    setString( "LATN", paramValue );
                    break;
                }
            }
            if ( !flag )
            {
                std::cout<<"参数配置错误，应配置为如 -lgz"<<std::endl;
                return;
            }

		  }
 
        else */
		if ( tempParamValue.substr( 0, 2 ) == "-t" )
        {
            paramValue = tempParamValue.substr( 2, tempParamValue.length() );
            int tempValue( 0 );
            if ( !Poco::NumberParser::tryParse( paramValue, tempValue ) )
            {
                std::cout<<"参数配置错误，需配置正确的唯一数据源标识，如 -t111"<<std::endl;
                return;
            }
            setInt( "TASK", tempValue );
        }
    }
}
std::string paramParse::expand( const std::string& value ) const
{
    return config->expand( value );
}
bool paramParse::hasProperty( const std::string& key ) const
{
    return config->hasProperty( key );
}
bool paramParse::hasOption( const std::string& key ) const
{
    return config->hasOption( key );
}
std::string paramParse::getString( const std::string& key ) const
{
    return config->getString( key );
}
std::string paramParse::getString( const std::string& key, const std::string& defaultValue ) const
{
    return config->getString( key, defaultValue );
}
std::string paramParse::getRawString( const std::string& key ) const
{
    return config->getRawString( key );
}
std::string paramParse::getRawString( const std::string& key, const std::string& defaultValue ) const
{
    return config->getRawString( key, defaultValue );
}
int paramParse::getInt( const std::string& key ) const
{
    return config->getInt( key );
}
int paramParse::getInt( const std::string& key, int defaultValue ) const
{
    return config->getInt( key, defaultValue );
}
double paramParse::getDouble( const std::string& key ) const
{
    return config->getDouble( key );
}
double paramParse::getDouble( const std::string& key, double defaultValue ) const
{
    return config->getDouble( key, defaultValue );
}
bool paramParse::getBool( const std::string& key ) const
{
    return config->getBool( key );
}
bool paramParse::getBool( const std::string& key, bool defaultValue ) const
{
    return config->getBool( key, defaultValue );
}
void paramParse::setString( const std::string& key, const std::string& value )
{
    return config->setString( key, value );
}
void paramParse::setInt( const std::string& key, int value )
{
    return config->setInt( key, value );
}
void paramParse::setDouble( const std::string& key, double value )
{
    return config->setDouble( key, value );
}
void paramParse::setBool( const std::string& key, bool value )
{
    return config->setBool( key, value );
}


bool paramConfiguration::getRaw( const std::string& key, std::string& value ) const
{
    if ( LayeredConfiguration::getRaw( key, value ) )
    {
        return true;
    }

    //环境变量
    if ( Poco::Environment::has( key ) )
    {
        value = Poco::Environment::get( key );
        return true;
    }
    return false;
}


