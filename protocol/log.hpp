#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <ctime>

#define NUM 1024

#define DEBUG 0
#define NORMAL 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

const char *to_levelstr(int level)
{
    switch (level)
    {
    case DEBUG:
        return "DEBUG";
    case NORMAL:
        return "NORMAL";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        return nullptr;
    }
}

void logMessage(int level, const char *format, ...)
{
    //[日志等级] [时间戳/时间] [pid] [message]
    char logprefix[NUM];
    time_t currentTime = time(nullptr);
    struct tm *localTime = localtime(&currentTime);
    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1;
    int day = localTime->tm_mday;
    int hour = localTime->tm_hour;
    int minute = localTime->tm_min;
    snprintf(logprefix, sizeof(logprefix), "[%s][%d-%d-%d %d:%d]",
             to_levelstr(level), year, month, day, hour, minute);

    char logcontent[NUM];
    va_list arg;
    va_start(arg, format);
    vsnprintf(logcontent, sizeof(logcontent), format, arg);
    std::cout << logprefix << logcontent << std::endl;
}