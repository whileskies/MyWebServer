#include "Log.h"

Log::Log()
{
    lineCount_ = 0;
    isAsync_ = false;
    writeThread_ = nullptr;
    queue_ = nullptr;
    toDay_ = 0;
    fp_ = nullptr;
}

Log::~Log()
{
    if (writeThread_)
    {
        queue_->put(endFlag_);
        writeThread_->join();
    }
    
    if (fp_)
    {
        fileMutex_.lock();
        fflush(fp_);
        fclose(fp_);
        fileMutex_.unlock();
    }
}

// 仅由主线程初始化
void Log::init(int level, const char* path, const char* suffix, int maxQueueSize)
{
    isOpen_ = true;
    level_ = level;
    if (maxQueueSize > 0)
    {
        isAsync_ = true;
        if (!queue_)
        {
            std::unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
            queue_ = std::move(newQueue);

            std::unique_ptr<Thread> newThread(new Thread(FulshLogThread, "log-thread"));
            writeThread_ = std::move(newThread);
            writeThread_->start();
        }
    }
    else
    {
        isAsync_ = false;
    }

    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;

    path_ = path;
    suffix_ = suffix;

    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
        path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        buff_.retrieveAll();
        if (fp_)
        {
            fflush(fp_);
            fclose(fp_);
        }

        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr)
        {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }

        assert(fp_ != nullptr);
    }
}

void Log::changeFile()
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;

    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0)))
    {
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 1, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else
        {
            snprintf(newFile, LOG_NAME_LEN - 1, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
        }

        fileMutex_.lock();
        fflush(fp_);
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
        fileMutex_.unlock();
    }

    lineCount_++;
}

void Log::write(int level, const char* format, ...)
{
    va_list vaList;

    mutex_.lock();
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;

    int n = snprintf(buff_.beginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
    buff_.hasWritten(n);
    appendLogLevelTitle(level_);

    va_start(vaList, format);
    int m = vsnprintf(buff_.beginWrite(), buff_.writeableBytes(), format, vaList);
    va_end(vaList);

    buff_.hasWritten(m);
    buff_.append("\n", 1);
    std::string line = buff_.retrieveAllToStr();
    mutex_.unlock();

    if (isAsync_)
    {
        if (queue_->full())
        {
            return;
        }
        queue_->put(std::move(line));
    }
    else
    {
        mutex_.lock();
        changeFile();
        mutex_.unlock();
        
        fileMutex_.lock();
        fputs(line.c_str(), fp_);
        fileMutex_.unlock();
    }
}

void Log::appendLogLevelTitle(int level)
{
    switch (level)
    {
    case 0:
        buff_.append("[debug]: ", 9);
        break;
    case 1:
        buff_.append("[info]: ", 9);
        break;
    case 2:
        buff_.append("[warn]: ", 9);
        break;
    case 3:
        buff_.append("[error]: ", 9);
        break;
    default:
        buff_.append("[info]: ", 9);
        break;
    }       
}

int Log::getLevel()
{
    std::lock_guard<std::mutex> lk(mutex_);
    return level_;
}

void Log::setLevel(int level)
{
    std::lock_guard<std::mutex> lk(mutex_);
    level_ = level;
}

void Log::asyncWrite()
{
    while (true)
    {
        std::string log = queue_->take();
        if (log == endFlag_)
        {
            return;
        }

        mutex_.lock();
        changeFile();
        mutex_.unlock();

        fileMutex_.lock();
        fputs(log.c_str(), fp_);
        fileMutex_.unlock();
    }
}


Log* Log::getInstance()
{
    static Log instance;
    return &instance;
}

void Log::FulshLogThread()
{
    Log::getInstance()->asyncWrite();
}