#include "Poller.h"
#include "Channel.h"
#include "../log/Log.h"

#include <assert.h>

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop)
{}

Poller::~Poller()
{}

Timestamp Poller::poll(int timeOutMs, channelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeOutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) 
    {
        LOG_TRACE("%d events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    } 
    else if (numEvents == 0)
    {
        LOG_TRACE("nothing happended");
    } 
    else
    {
        LOG_ERROR("Poller::poll()");
    }

    return now;
}

void Poller::fillActiveChannels(int numEvents, channelList* activeChannels) const
{
    for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->setRevents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE("fd = %d, events = %d", channel->fd(), channel->events());
    if (channel->index() < 0)
    {
        // new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;

        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);

        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->setIndex(idx);
        channels_[pfd.fd] = channel;
    } 
    else 
    {
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        
        if (channel->isNoneEvent())
        {
            // ignore this pollfd
            pfd.fd = -1;
        }
    }
}