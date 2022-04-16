#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "thread/Thread.h"
#include "thread/BlockQueue.h"
#include "base/Buffer.h"
#include "log/log.h"

using namespace std;

int main()
{
    Log::getInstance()->init(0, "./log", ".log", 1024);

    vector<thread> threads;

    for (int i = 0; i < 10; i++) {
        threads.emplace_back(thread([=]{
            while (1)
                LOG_DEBUG("thread %d", i);
            // cout << "hhh" <<endl;
        }));
    }

    while(1) {}







    return 0;
}