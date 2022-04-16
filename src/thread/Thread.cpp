#include "Thread.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <cassert>

pid_t getpid() 
{
    return static_cast<pid_t>(::syscall(SYS_getpid));
}

namespace CurrentThread
{

thread_local int t_cachedTid = 0;
thread_local const char* t_threadName = nullptr;

}

void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0) {
        t_cachedTid = gettid();
    }
}

struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    explicit ThreadData(const ThreadFunc& func, const std::string name,
                        pid_t* tid, CountDownLatch *latch)
                        : func_(func), name_(name), tid_(tid), latch_(latch)
    {}

    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        
        latch_->countDown();

        CurrentThread::t_threadName = name_.data();
        prctl(PR_SET_NAME, CurrentThread::t_threadName);

        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void* startThread(void* arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);
    data->runInThread();
    delete data;

    return nullptr;
}

std::atomic<int> Thread::threadNum_(0);

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(std::move(name)),

    latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pthread_detach(pthreadId_);
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;

    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    if (pthread_create(&pthreadId_, nullptr, &startThread, data))
    {
        started_ = false;
        delete data;
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);

    joined_ = true;

    return pthread_join(pthreadId_, nullptr);
}

void Thread::setDefaultName()
{
    int num = threadNum_++;
    if (name_.empty()) 
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}
    




