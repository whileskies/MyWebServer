#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include "../base/noncopyable.h"

class EventLoop;

class Channel : noncopyable
{
public:
    typedef std::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);

    void handleEvent();
    void setReadCallback(const EventCallback& cb)
    { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb)
    { writeCallback_ = cb; }
    void setErorCallback(const EventCallback& cb)
    { errorCallback_ = cb; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    void setRevents(int revt) { revents_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    int index() { return index_; }
    void setIndex(int index) { index_ = index; }

    EventLoop* ownerLoop() { return loop_; }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
};

#endif