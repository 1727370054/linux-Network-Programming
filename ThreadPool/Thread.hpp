#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <functional>
#include <pthread.h>

namespace ThreadNs
{
    typedef std::function<void *(void *)> func_t;
    const int num = 1024;

    class Thread
    {
    private:
        static void *start_routine(void *args)
        {
            Thread *this_ = static_cast<Thread *>(args);
            return this_->callback();
        }

        void *callback()
        {
            return func_(args_);
        }

    public:
        Thread()
        {
            char namebuffer[num];
            snprintf(namebuffer, sizeof namebuffer, "thread-%d", threadnum++);
            name_ = namebuffer;
        }

        void start(func_t func, void *args)
        {
            func_ = func;
            args_ = args;
            int n = pthread_create(&tid_, nullptr, start_routine, this);
            assert(n == 0);
            (void)n;
        }

        void join()
        {
            int n = pthread_join(tid_, nullptr);
            assert(n == 0);
            (void)n;
        }

        std::string threadname()
        {
            return name_;
        }

        ~Thread()
        {
            // do nothing
        }

    private:
        std::string name_;
        func_t func_;
        void *args_;
        pthread_t tid_;

        static int threadnum;
    };
    int Thread::threadnum = 1;
}