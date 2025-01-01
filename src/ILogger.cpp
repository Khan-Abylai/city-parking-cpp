#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "ILogger.h"

using namespace std;

ILogger::ILogger() = default;

ILogger::ILogger(string prependMessage) {
    LOG_PREPEND_MESSAGE = move(prependMessage) + ": ";
}

void ILogger::LOG_INFO(const string &format, ...) const {
    va_list args;
    auto newFormat = LOG_PREPEND_MESSAGE + format + "\n";
    va_start(args, format);
    vfprintf(stdout, newFormat.data(), args);
    fflush(stdout);
    va_end(args);
}

void ILogger::LOG_ERROR(const string &format, ...) const {
    va_list args;
    auto newFormat = LOG_PREPEND_MESSAGE + format + "\n";
    va_start(args, format);
    vfprintf(stderr, newFormat.data(), args);
    va_end(args);
}
