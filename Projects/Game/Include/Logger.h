#pragma once

#include <format>
#include <iostream>
#include <unordered_map>

namespace NS
{
    enum class ELogLevel : uint8_t
    {
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    inline std::unordered_map<ELogLevel, const char*> G_LOGLEVEL_TOSTRING
    {
        {ELogLevel::INFO, "INFO"},
        {ELogLevel::WARNING, "WARNING"},
        {ELogLevel::ERROR, "ERROR"},
        {ELogLevel::FATAL, "FATAL"}
    };

    inline std::unordered_map<ELogLevel, const char*> G_LOGLEVEL_COLOR
    {
        {ELogLevel::INFO, "\033[0m"},
        {ELogLevel::WARNING, "\033[1;33m"},
        {ELogLevel::ERROR, "\033[1;31m"},
        {ELogLevel::FATAL, "\033[1;35m"}
    };

    template <class... Args>
	void GLogInternal(const ELogLevel Level, const char* FileName, const int LineNum , const std::format_string<Args...> Format, Args... Arguments)
    {
        const std::string FormattedMessage = std::vformat(Format.get(), std::make_format_args(Arguments...));
        std::cout << G_LOGLEVEL_COLOR.at(Level) << G_LOGLEVEL_TOSTRING.at(Level) << " : [" << FileName << " Ln " << LineNum << "] : " << FormattedMessage << "\033[0m\n";
    }
}

#define LOGINFO NS::ELogLevel::INFO
#define LOGWARN NS::ELogLevel::WARNING
#define LOGERROR NS::ELogLevel::ERROR
#define LOGFATAL NS::ELogLevel::FATAL
#define NSLOG(loglevel, format, ...) NS::GLogInternal(loglevel, __FILE__, __LINE__ ,format, ##__VA_ARGS__);
#pragma once
