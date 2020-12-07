#include "Logger.h"

Logger::Logger(){
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    typedef logging::sinks::synchronous_sink< logging::sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
    sink->set_formatter(&Logger::coloring_formatter);
    logging::core::get()->add_sink(sink);
//    logging::core::get()->set_filter
//            (
//                    logging::trivial::severity >= logging::trivial::fatal
//            );

    logging::add_console_log(
            std::cout,
            keywords::format = "[%TimeStamp%] [%Severity%] - %Message%"
    );
    logging::add_file_log(
            keywords::file_name = "../logs/logs.log",
            keywords::auto_flush = true,
            keywords::enable_final_rotation = false,
            keywords::rotation_size = 5 * 1024 * 1024,
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
        std::cout << "\033[0m";
        switch (severity.get())
        {
            case logging::trivial::severity_level::info:
                std::cout << "\033[38;5;16;48;5;71m";
                break;
            case logging::trivial::severity_level::warning:
                std::cout << "\033[38;5;16;48;5;172m";
                break;
            case logging::trivial::severity_level::debug:
                std::cout << "\033[38;5;81m";
                break;
            case logging::trivial::severity_level::trace:
                std::cout << "\033[38;5;16;48;5;219m";
                break;
            case logging::trivial::severity_level::error:
                std::cout << "\033[38;5;16;48;5;124m";
                break;
            case logging::trivial::severity_level::fatal:
                std::cout << "\033[38;5;124;48;5;16m";
                break;
            default:
                break;
        }
    }
}

Logger LOG;

