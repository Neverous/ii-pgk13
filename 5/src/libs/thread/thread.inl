#ifndef __THREAD_INL__
#define __THREAD_INL__

#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "thread.h"

#define _inline inline

extern ThreadMonitor threads;

using namespace std;

// THREAD
_inline
Thread::Thread(void)
:state(Thread::STOPPED)
,id(0)
,handle()
{
    threads.addThread(this);
}

_inline
Thread::~Thread(void)
{
    threads.removeThread(this);
    if(handle.joinable())
        handle.join();
}

_inline
void Thread::activate(void)
{
    {
        unique_lock<mutex> lock(threads.lock);
        threads.active.wait(lock, [&]{return state == Thread::STARTING;});
    }

    try
    {
        start();
        state = Thread::STARTED;
        run();
    }
    catch(...)
    {
        if(state != Thread::STOPPED)
        {
            state = Thread::STOPPING;
            stop();
            state = Thread::STOPPED;
        }

        throw;
    }

    if(state != Thread::STOPPED)
    {
        state = Thread::STOPPING;
        stop();
        state = Thread::STOPPED;
    }
}

// THREADMONITOR
_inline
ThreadMonitor::ThreadMonitor(void)
:_active(false)
,threads()
,lock()
,cnt(1)
,active()
{
}

_inline
ThreadMonitor::~ThreadMonitor(void)
{
    cleanup();
}

_inline
void ThreadMonitor::cleanup(void)
{
    while(!this->threads.empty())
        delete this->threads.front();
}

_inline
void ThreadMonitor::activate(void)
{
    assert(!_active);
    for(Thread *_thread: this->threads)
        _thread->handle = thread(&Thread::activate, _thread);

    unique_lock<mutex> _lock(lock);
    for(Thread *_thread: this->threads)
        _thread->state = Thread::STARTING;

    active.notify_all();
    _active = true;
}

_inline
void ThreadMonitor::monitor(void)
{
    while(_active)
    {
        this_thread::sleep_for(chrono::seconds(1));
        if(!_active)
            break;

        for(Thread *_thread: this->threads)
            if(_thread->state == Thread::STOPPED && _active)
            {
                if(_thread->handle.joinable())
                    _thread->handle.join();

                unique_lock<mutex> _lock(lock);
                _thread->handle = thread(&Thread::activate, _thread);
                _thread->state = Thread::STARTING;
                active.notify_one();
            }
    }
}

_inline
void ThreadMonitor::deactivate(void)
{
    assert(_active);
    _active = false;
    for(Thread *_thread: this->threads)
        if(_thread->state != Thread::STOPPED)
        {
            _thread->state = Thread::STOPPING;
            _thread->terminate();
        }

    for(Thread *_thread: this->threads)
        if(_thread->handle.joinable())
            _thread->handle.join();
}

_inline
void ThreadMonitor::addThread(Thread *_thread)
{
    assert(!_thread->id);
    _thread->id = cnt ++;
    lock_guard<mutex> _lock(lock);
    this->threads.push_back(_thread);
    assert(cnt < 255);
}

_inline
void ThreadMonitor::removeThread(Thread *_thread)
{
    assert(_thread->id);
    _thread->id = 0;
    lock_guard<mutex> _lock(lock);
    this->threads.remove(_thread);
}

#undef _inline
#endif // __THREAD_INL__
