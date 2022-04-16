#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <signal.h>
#include "thread/Thread.h"
#include "thread/BlockQueue.h"
#include "base/Buffer.h"
#include "log/Log.h"
#include "base/Timestamp.h"
#include "net/EventLoop.h"

#include "net/Callbacks.h"
#include "net/Timer.h"

using namespace std;

EventLoop* g_loop;

void threadFunc()
{
    g_loop->loop();
}

int main()
{
    Log::getInstance()->init(LEVEL_TRACE, "./log", ".log", 1024);

    EventLoop loop;
    g_loop = &loop;

    Thread t(threadFunc);
    t.start();

    t.join();

    

    return 0;
}