#pragma once
#include "Thread.hpp"
#include "LockGuard.hpp"
#include "log.hpp"
#include <vector>
#include <queue>
#include <mutex>
#include <pthread.h>

using namespace ThreadNs;

static int gnum = 5;

template <class T>
class ThreadPool;
template <class T>
class ThreadData
{
public:
    ThreadData(ThreadPool<T> *tp, const std::string n) : threadpool(tp), name(n) {}

public:
    ThreadPool<T> *threadpool;
    std::string name;
};

template <class T>
class ThreadPool
{
private:
    static void *handlerTask(void *args)
    {
        ThreadData<T> *td = static_cast<ThreadData<T> *>(args);
        while (true)
        {
            T t;
            {
                LockGuard lockguard(td->threadpool->mutex());
                while (td->threadpool->isQueueEmpty())
                {
                    td->threadpool->threadWait();
                }
                t = td->threadpool->pop();
            }
            t();
        }
        delete td;
        return nullptr;
    }

private:
    bool isQueueEmpty() { return _task_queue.empty(); }
    pthread_mutex_t *mutex() { return &_mutex; }
    void threadWait() { pthread_cond_wait(&_cond, &_mutex); }
    T pop()
    {
        T t = _task_queue.front();
        _task_queue.pop();
        return t;
    }

    ThreadPool(const int &num = gnum) : _num(num)
    {
        pthread_mutex_init(&_mutex, nullptr);
        pthread_cond_init(&_cond, nullptr);

        for (int i = 0; i < _num; i++)
        {
            _threads.push_back(new Thread());
        }
    }

    void operator=(const ThreadPool &) = delete;
    ThreadPool(const ThreadPool &) = delete;

public:
    void create()
    {
        for (const auto &t : _threads)
        {
            ThreadData<T> *td = new ThreadData<T>(this, t->threadname());
            t->start(handlerTask, td);
            // std::cout << t->threadname() << " start..." << std::endl;
            logMessage(DEBUG, "%s start...", t->threadname().c_str());
        }
    }
    void push(const T &in)
    {
        LockGuard lockguard(&_mutex);
        _task_queue.push(in);
        pthread_cond_signal(&_cond);
    }
    ~ThreadPool()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_cond);
        for (const auto &t : _threads)
            delete t;
    }

    static ThreadPool<T> *getInstance()
    {
        if (nullptr == tp)
        {
            _singleLock.lock();
            if (nullptr == tp)
            {
                tp = new ThreadPool<T>();
            }
            _singleLock.unlock();
        }
        return tp;
    }

private:
    int _num;
    std::vector<Thread *> _threads;
    std::queue<T> _task_queue;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;

    static ThreadPool<T> *tp;
    static std::mutex _singleLock;
};

template <class T>
ThreadPool<T> *ThreadPool<T>::tp = nullptr;
template <class T>
std::mutex ThreadPool<T>::_singleLock;
