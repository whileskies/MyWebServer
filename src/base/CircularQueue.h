#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <cassert>
#include <stddef.h>
#include <mutex>
#include <queue>
#include "noncopyable.h"

template <class T>
class CircularQueue : noncopyable {
public:
    explicit CircularQueue(size_t cap = 1024) 
    {
        assert(cap >= 0);
        data_ = new T[cap];
        front_ = 0;
        rear_ = 0;
        cap_ = cap;
    }

    ~CircularQueue() 
    {
        delete[] data_;
        data_ = nullptr;
    }

    bool empty() const
    {
        return front_ == rear_;
    }

    bool full() const
    {
        return (rear_ + 1) % cap_ == front_;
    }

    void push(const T& x) 
    {
        if (full())
        {
            resize();
        }

        data_[rear_] = x;
        rear_ = (rear_ + 1) % cap_;
    }

    T front()
    {
        assert(!empty());
        return data_[front_];
    }

    void pop()
    {
        if (!empty())
        {
            front_ = (front_ + 1) % cap_;
        }
    }

    size_t size() const
    {
        return (rear_ - front_ + cap_) % cap_;
    }

    size_t capacity() const
    {
        return cap_;
    }


    void resize()
    {
        std::cout << "resize" << std::endl;
        size_t newCap = cap_ * 1.5;
        T* newData = new T[newCap];

        size_t rear = 0;
        for (size_t i = front_; i != rear_; i = (i + 1) % cap_) 
        {
            newData[rear++] = data_[i];
        }

        front_ = 0;
        rear_ = rear;
        cap_ = newCap;
        delete[] data_;
        data_ = newData;
    }

    
private:
    T* data_;
    size_t front_;
    size_t rear_;
    size_t cap_;
};


#endif 