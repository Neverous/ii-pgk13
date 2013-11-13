#ifndef __THREAD_H__
#define __THREAD_H__

#include <unistd.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <list>

using namespace std;

class Thread
{
    friend class ThreadMonitor;
    public:
        enum State
        {
            STARTED,
            STOPPED,
            STARTING,
            STOPPING,
        }; // enum State

        Thread(void);
        virtual ~Thread(void);

    protected:
        State   state;
        int     id;
        thread  handle;

        // Called right before run
        virtual void start(void) = 0;

        // Running
        virtual void run(void) = 0;

        // Called right before thread terminates
        virtual void stop(void) = 0;

        // Called from main thread when termination is required
        virtual void terminate(void) = 0;

    private:
        // Activating thread
        void activate(void);
}; // class Thread

class ThreadMonitor
{
    friend class Thread;
    public:
        ThreadMonitor(void);
        ~ThreadMonitor(void);
        void cleanup(void);

        // Activates threads
        void activate(void);

        // Monitors thread
        void monitor(void);

        // Deactivates threads
        void deactivate(void);

    private:
        bool                _active;
        list<Thread *>      threads;
        mutex               lock;
        int                 cnt;
        condition_variable  active;

        // Register threads
        void addThread(Thread *_thread);
        void removeThread(Thread *_thread);
}; // class ThreadMonitor

#include "thread.inl"

#endif // __THREAD_H__
