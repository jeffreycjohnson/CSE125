#include "ThreadPool.h"

ThreadPool * workerPool;
thread_local ThreadPool::Job* ThreadPool::currentJob;
thread_local int ThreadPool::currentThread;

ThreadPool::Job* ThreadPool::Job::queue()
{
    std::unique_lock<std::mutex> lock(pool->jobLock);
    pool->readyQueue.insert(this);
    pool->condition.notify_one();
    return this;
}

void ThreadPool::Job::markComplete()
{
    std::unique_lock<std::mutex> lock(pool->jobLock);
    for (auto dependent : dependents)
    {
        dependent->dependencies.erase(this);
    }
}

ThreadPool::Job* ThreadPool::Job::addDependency(Job* other)
{
    dependencies.insert(other);
    other->dependents.insert(this);
    return this;
}

ThreadPool::Job* ThreadPool::Job::setAffinity(int thread)
{
    affinity = thread;
    return this;
}

ThreadPool::Job* ThreadPool::Job::setPriority(int amount)
{
    priority = amount;
    return this;
}

ThreadPool::Job::Job(std::function<void()> func, ThreadPool* pool) : pool(pool), func(func)
{
}

bool ThreadPool::jobComparator(Job* first, Job* second)
{
    if(first->priority != second->priority) return first->priority > second->priority;
    if(first->affinity != second->affinity) return first->affinity > second->affinity;
    return first > second;
}

ThreadPool::ThreadPool(size_t threadCount) : threadCount(threadCount > 0 ? threadCount : std::thread::hardware_concurrency()), readyQueue(&jobComparator)
{
    activeJobs.resize(this->threadCount);
    for (size_t i = 0; i < this->threadCount; i++)
    {
        threads.emplace_back(runThread, this, i);
    }
}

ThreadPool::~ThreadPool()
{
    shutdown = true;
    condition.notify_all();
    for(auto& thread : threads)
    {
        thread.join();
    }
}

ThreadPool::Job* ThreadPool::createJob(const std::function<void()>& func)
{
    return new Job(func, this);
}

bool ThreadPool::completed(Job* job)
{
    std::unique_lock<std::mutex> lock(jobLock);
    if (readyQueue.count(job) != 0) return false;
    bool ret = true;
    for (auto active : activeJobs) {
        if (active == job) ret = false;
    }
    return ret;
}

void ThreadPool::wait(Job* job)
{
    while(!completed(job)) std::this_thread::yield();
}

void ThreadPool::runThread(ThreadPool* pool, size_t id)
{
    currentThread = id;
    while(!pool->shutdown)
    {
        std::unique_lock<std::mutex> lock(pool->jobLock);
        bool wake = false, blocked = true;
        for(auto job : pool->readyQueue)
        {
            if (job->dependencies.size() > 0) continue;
            if (job->affinity >= 0 && job->affinity != id) {
                wake = true;
                continue;
            }
            blocked = false;
            pool->readyQueue.erase(job);
            pool->activeJobs[id] = job;
            lock.unlock();

            currentJob = job;
            job->func();

            lock.lock();
            for (auto dependent : job->dependents)
            {
                dependent->dependencies.erase(job);
                if (dependent->dependencies.size() == 0) wake = true;
            }
            delete job;
            pool->activeJobs[id] = nullptr;
            break;
        }
        if (wake) pool->condition.notify_all();
        if (blocked && !pool->shutdown) pool->condition.wait(lock);
    }
}
