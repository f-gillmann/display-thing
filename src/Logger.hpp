#pragma once

#include <Arduino.h>
#include <cstdarg>
#include <cstring>

enum class LogLevel
{
    NONE = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4
};

class LoggerClass
{
public:
    void begin(Stream& output, const LogLevel level = LogLevel::DEBUG)
    {
        _output = &output;
        _logLevel = level;
    }

    void setLevel(const LogLevel level)
    {
        _logLevel = level;
    }

    template <typename... Args>
    void error(const char* file, int line, const char* format, Args... args)
    {
        print(LogLevel::ERROR, "ERROR", file, line, format, args...);
    }

    template <typename... Args>
    void warn(const char* file, int line, const char* format, Args... args)
    {
        print(LogLevel::WARN, "WARN ", file, line, format, args...);
    }

    template <typename... Args>
    void info(const char* file, int line, const char* format, Args... args)
    {
        print(LogLevel::INFO, "INFO ", file, line, format, args...);
    }

    template <typename... Args>
    void debug(const char* file, int line, const char* format, Args... args)
    {
        print(LogLevel::DEBUG, "DEBUG", file, line, format, args...);
    }

private:
    Stream* _output = nullptr;
    LogLevel _logLevel = LogLevel::NONE;
    static constexpr int LOG_BUFFER_SIZE = 256;

    template <typename... Args>
    void print(const LogLevel level, const char* levelStr, const char* file, const int line, const char* format,
               Args... args)
    {
        if (level <= _logLevel && _output)
        {
            char buffer[LOG_BUFFER_SIZE];

            const char* short_file = strrchr(file, '/');
            short_file = short_file ? short_file + 1 : file;

            snprintf(buffer, LOG_BUFFER_SIZE, "[%8lu] [%s] [%s:%d] ", millis(), levelStr, short_file, line);
            _output->print(buffer);

            snprintf(buffer, LOG_BUFFER_SIZE, format, args...);
            _output->println(buffer);
        }
    }
};

extern LoggerClass Logger;

#define LOG_ERROR(format, ...) Logger.error(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  Logger.warn(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  Logger.info(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) Logger.debug(__FILE__, __LINE__, format, ##__VA_ARGS__)
