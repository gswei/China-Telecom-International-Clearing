//
// Copyright (c) 2009-2015 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
// All rights reserved.
//
// Created:     ${date} ${time}
// Module:      ${project_name}
// Author:      ${user}
// Revision:    $$Id$$
// Description:参数解析 仅限-l    CITY_ID   LATN
//
#ifndef PARAM_CONFIG_PARSE_H_
#define PARAM_CONFIG_PARSE_H_
#include "Poco/Util/Util.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/LayeredConfiguration.h"

using Poco::Util::AbstractConfiguration;
using Poco::Util::LayeredConfiguration;



class paramConfiguration: public LayeredConfiguration
{
public:
    paramConfiguration() {};
    ~paramConfiguration() {};
    //获取本地网标识
protected:
    bool getRaw( const std::string& key, std::string& value ) const;
//public:
    //Poco::Util::AbstractConfiguration* config;
};

class paramParse
{
public:
    paramParse();
    ~paramParse();
    void init( int argc, char** argv );
    std::string expand( const std::string& value ) const;
    bool hasProperty( const std::string& key ) const;
    bool hasOption( const std::string& key ) const;
    std::string getString( const std::string& key ) const;
    std::string getString( const std::string& key, const std::string& defaultValue ) const;
    std::string getRawString( const std::string& key ) const;
    std::string getRawString( const std::string& key, const std::string& defaultValue ) const;
    int getInt( const std::string& key ) const;
    int getInt( const std::string& key, int defaultValue ) const;
    double getDouble( const std::string& key ) const;
    double getDouble( const std::string& key, double defaultValue ) const;
    bool getBool( const std::string& key ) const;
    bool getBool( const std::string& key, bool defaultValue ) const;
    void setString( const std::string& key, const std::string& value );
    void setInt( const std::string& key, int value );
    void setDouble( const std::string& key, double value );
    void setBool( const std::string& key, bool value );

private:
    paramConfiguration* config;
};

#endif