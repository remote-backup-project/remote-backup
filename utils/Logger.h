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
    Logger();

    static void coloring_formatter(logging::record_view const& rec, logging::formatting_ostream& strm);

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

extern Logger LOG;

#endif //REMOTE_BACKUP_LOGGER_H
