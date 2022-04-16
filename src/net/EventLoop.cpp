#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "../log/Log.h"
#include <assert.h>


thread_local EventLoop* t_loopInThreadThread = nullptr;
const int kPollTimeMs = 1000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(CurrentThread::tid()),
      poller_(new Poller(this))
{
    LOG_TRACE("EventLoop created %p in thread %d", this, threadId_);
    if (t_loopInThreadThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThreadThread, threadId_);
    }
    else
    {
        t_loopInThreadThread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThreadThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        LOG_TRACE("looooop....");
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (auto chan : activeChannels_)
        {
            chan->handleEvent();
        }
    }

    LOG_TRACE("EventLoop %p stop looping", this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // wakeup
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL("EventLoop::abortNotInLoopThread - EventLoop %p was created in thread id = %d, current thread id = %d", 
        this, threadId_, CurrentThread::tid());
}

