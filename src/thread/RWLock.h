#ifndef RWLOCK_H
#define RWLOCK_H

#include <thread>
#include <mutex>
#include <condition_variable>

class RWLock {
    using mutex_t = std::mutex;
    using cond_t = std::condition_variable;

public:
    RWLock() : readerCount_(0), writerEnter_(false) {}

    void RLock() {
        std::unique_lock<mutex_t> lk(mutex_);
        while (writerEnter_) {
            reader_.wait(lk);
        }

        readerCount_++;
    }
    
    void RUnLock() {
        std::unique_lock<mutex_t> lk(mutex_);
        readerCount_--;
        if (writerEnter_ && readerCount_ == 0) {
            writer_.notify_one();
        }
    }

    void WLock() {
        std::unique_lock<mutex_t> lk(mutex_);
        while (writerEnter_) {
            writer_.wait(lk);
        }

        writerEnter_ = true;
        while (readerCount_ > 0) {
            writer_.wait(lk);
        }
    }

    void WUnLock() {
        std::unique_lock<mutex_t> lk(mutex_);
        writerEnter_ = false;
        reader_.notify_one();
    }

private:
    int readerCount_;
    mutex_t mutex_;
    cond_t reader_;
    cond_t writer_;
    bool writerEnter_;
};

#endif