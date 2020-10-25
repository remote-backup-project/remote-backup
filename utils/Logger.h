//
// Created by alessandro on 25/10/20.
//

#ifndef REMOTE_BACKUP_LOGGER_H
#define REMOTE_BACKUP_LOGGER_H

#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;

class Logger{
public:
    Logger(){
        logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

        logging::add_console_log(
                std::cout,
                keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] [%ProcessID%] - %Message%"
        );
        logging::add_file_log(
                keywords::file_name = "../logs/logs.log",
                keywords::auto_flush = true,
                keywords::rotation_size = 5 * 1024 * 1024,
                keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] [%ProcessID%] - %Message%"
        );

        logging::add_common_attributes();
    };

    void info(const std::string& message){
        BOOST_LOG_TRIVIAL(info) << message;
    }

    void debug(const std::string& message){
        BOOST_LOG_TRIVIAL(debug) << message;
    }

    void trace(const std::string& message){
        BOOST_LOG_TRIVIAL(trace) << message;
    }

    void warning(const std::string& message){
        BOOST_LOG_TRIVIAL(warning) << message;
    }

    void error(const std::string& message){
        BOOST_LOG_TRIVIAL(error) << message;
    }

    void fatal(const std::string& message){
        BOOST_LOG_TRIVIAL(fatal) << message;
    }
};

#endif //REMOTE_BACKUP_LOGGER_H
