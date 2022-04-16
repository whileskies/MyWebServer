#ifndef CURRENT_THREAD_H
#define CURRENT_THREAD_H

namespace CurrentThread 
{
extern thread_local int t_cachedTid;
extern thread_local const char *t_threadName;

void cacheTid();

inline int tid()
{
    if (t_cachedTid == 0)
    {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* name()
{
    return t_threadName;
}

} // namespace CurrentThread

#endif