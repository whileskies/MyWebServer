#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <thread>
#include <condition_variable>
#include <mutex>
#include <deque>
#include "../base/CircularQueue.h"

template<class T>
class BlockQueue : noncopyable {
public:
    explicit BlockQueue(size_t cap = 1024);

    ~BlockQueue();

    bool empty() const;

    bool full() const;

    size_t size() const;

    size_t capacity() const;

    void put(const T&);

    void put(const T&&);

    T take();

private:
size_t cap_;
    CircularQueue<T> queue_;
    mutable std::mutex mutex_;

    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
};

template<class T>
BlockQueue<T>::BlockQueue(size_t cap) : cap_(cap), queue_(cap)
{
    // assert(cap >= 0);
}

template<class T>
BlockQueue<T>::~BlockQueue()
{}

template<class T>
bool BlockQueue<T>::empty() const
{
    std::unique_lock<std::mutex> lk(mutex_);
    return queue_.empty();
}

template<class T>
bool BlockQueue<T>::full() const
{
    std::unique_lock<std::mutex> lk(mutex_);
    return queue_.full();
}

template<class T>
size_t BlockQueue<T>::size() const
{
    std::unique_lock<std::mutex> lk(mutex_);
    return queue_.size();
}

template<class T>
size_t BlockQueue<T>::capacity() const
{
    std::unique_lock<std::mutex> lk(mutex_);
    return queue_.capacity();
}

template<class T>
void BlockQueue<T>::put(const T& x)
{
    std::unique_lock<std::mutex> lk(mutex_);
    while (queue_.full())
    {
        notFull_.wait(lk);
    }

    assert(!queue_.full());
    queue_.push(x);
    notEmpty_.notify_one();
}

template<class T>
void BlockQueue<T>::put(const T&& x)
{
    std::unique_lock<std::mutex> lk(mutex_);
    while (queue_.full())
    {
        notFull_.wait(lk);
    }

    assert(!queue_.full());
    queue_.push(std::move(x));
    notEmpty_.notify_one();
}

template<class T>
T BlockQueue<T>::take()
{
    std::unique_lock<std::mutex> lk(mutex_);
    while (queue_.empty())
    {
        notEmpty_.wait(lk);
    }

    assert(!queue_.empty());
    T front(queue_.front());
    queue_.pop();
    notFull_.notify_one();

    return front;
}


#endif