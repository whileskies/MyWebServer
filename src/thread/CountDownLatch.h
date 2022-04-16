#ifndef COUNTDOWN_LATCH_H
#define COUNTDOWN_LATCH_H

#include "../base/noncopyable.h"
#include <mutex>
#include <condition_variable>


class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);

    void wait();
    void countDown();
    int getCount();


private:
    int count_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};


#endif