#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <cstdarg>
#include <sys/stat.h>
#include <sys/time.h>
#include <cassert>
#include <cstring>

#include <thread>
#include <iostream>

#include "../thread/Thread.h"
#include "../thread/BlockQueue.h"
#include "../base/Buffer.h"

class Log {
public:
    void init(int leve, const char* path = "./log", 
        const char* suffix=".log", int maxQueueSize = 1024);
    
    static Log* getInstance();
    static void FulshLogThread();

    void write(int level, const char* format, ...);
    void fatal();

    void flush();
    void closeAndFlush();

    int getLevel();
    void setLevel(int level);
    bool isOpen() { return isOpen_; };

private:
    Log();
    ~Log();

    void changeFile();
    void appendLogLevelTitle(int level);
    void asyncWrite();

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;
    
    std::string endFlag_ = "@END@";

    const char* path_;
    const char* suffix_;

    int MAX_LINES_;

    uint lineCount_;
    int toDay_;

    bool isOpen_;

    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE* fp_;
    std::unique_ptr<BlockQueue<std::string>> queue_;
    std::unique_ptr<Thread> writeThread_;
    std::mutex mutex_;
    std::mutex fileMutex_;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::getInstance();\
        if (log->isOpen() && log->getLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__);\
        }\
    } while(0);

#define LOG_FATAL_BASE(level, format, ...) \
    do {\
        Log* log = Log::getInstance();\
        if (log->isOpen() && log->getLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__);\
            log->fatal();\
        }\
    } while(0);

#define LEVEL_TRACE 0
#define LEVEL_DEBUG 1
#define LEVEL_INFO 2
#define LEVEL_WARN 3
#define LEVEL_ERROR 4
#define LEVEL_FATAL 5

#define LOG_TRACE(format, ...) do {LOG_BASE(LEVEL_TRACE, format, ##__VA_ARGS__)} while(0);
#define LOG_DEBUG(format, ...) do {LOG_BASE(LEVEL_DEBUG, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(LEVEL_INFO, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(LEVEL_WARN, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(LEVEL_ERROR, format, ##__VA_ARGS__)} while(0);
#define LOG_FATAL(format, ...) do {LOG_FATAL_BASE(LEVEL_FATAL, format, ##__VA_ARGS__)} while(0);

#endif