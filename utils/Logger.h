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
#pragma once
#include <boost/log/sinks/basic_sink_backend.hpp>

namespace logging = boost::log;
namespace keywords = logging::keywords;
namespace attrs = logging::attributes;


class Logger{
public:
    Logger(){
        logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

        typedef logging::sinks::synchronous_sink< logging::sinks::text_ostream_backend > text_sink;
        boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
        sink->set_formatter(&Logger::coloring_formatter);
        logging::core::get()->add_sink(sink);

        logging::add_console_log(
                std::cout,
                keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] - %Message%"
        );
        logging::add_file_log(
                keywords::file_name = "../logs/logs.log",
                keywords::auto_flush = true,
                keywords::rotation_size = 5 * 1024 * 1024,
                keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] - %Message%"
        );

        logging::add_common_attributes();
    };


    static void coloring_formatter(
            logging::record_view const& rec, logging::formatting_ostream& strm)
    {
        auto severity = rec[logging::trivial::severity];
        if (severity)
        {
            // Set the color
            switch (severity.get())
            {
                case logging::trivial::severity_level::info:
                    std::cout << "\033[38;5;82m";
                    break;
                case logging::trivial::severity_level::warning:
                    std::cout << "\033[33m";
                    break;
                case logging::trivial::severity_level::error:
                case logging::trivial::severity_level::fatal:
                    std::cout << "\033[31m";
                    break;
                default:
                    break;
            }
        }
    }

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

Logger LOG;

#endif //REMOTE_BACKUP_LOGGER_H
