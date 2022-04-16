#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include "../thread/Thread.h"
#include "../base/noncopyable.h"
#include "../log/Log.h"

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    void updateChannel(Channel* channel);

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void abortNotInLoopThread();

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    const pid_t threadId_;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
};



#endif 
