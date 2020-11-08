//
// Created by alessandro on 06/11/20.
//

#include "Logger.h"

Logger::Logger(){
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    typedef logging::sinks::synchronous_sink< logging::sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
    sink->set_formatter(&Logger::coloring_formatter);
    logging::core::get()->add_sink(sink);

    logging::add_console_log(
            std::cout,
            keywords::format = "[%TimeStamp%] [%Severity%] - %Message%"
    );
    logging::add_file_log(
            keywords::file_name = "../logs/logs.log",
            keywords::auto_flush = true,
            keywords::rotation_size = 5 * 1024 * 1024 * 1024,
            keywords::format = "[%TimeStamp%] [%Severity%] - %Message%"
    );

    logging::add_common_attributes();
}

void Logger::coloring_formatter(
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

Logger LOG;

