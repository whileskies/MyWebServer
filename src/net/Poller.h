#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <map>
#include <poll.h>
#include "../base/noncopyable.h"
#include "../base/Timestamp.h"
#include "EventLoop.h"

#include "Channel.h"

class Channel;

class Poller : noncopyable
{
public:
    typedef std::vector<Channel*> channelList;

    Poller(EventLoop *loop);
    ~Poller();

    Timestamp poll(int timeoutMs, channelList* activeChannels);

    void updateChannel(Channel* channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    void fillActiveChannels(int numEvents, channelList* activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

#endif