#include "Thread.hpp"
#include <Windows.h>

Thread::Thread(std::unique_ptr<Runnable>&& task)
  : isRunning(false)
  , task(nullptr)
{ 
  this->task.swap(task);
}

void Thread::interrupt()
{
  if (task) {
    task->isInterrupted = true;
  }
}

void Thread::run()
{
  if (task && !isRunning) {
    isRunning = true;
    result = std::async(std::launch::async, &Runnable::run, task.get());
  }
}

void Thread::join()
{
  result.wait();
}

bool Thread::joinable() const
{
  return running();
}

bool Thread::interrupted() const
{
  if (task) {
    task->isInterrupted;
  }
  return false;
}

bool Thread::running() const
{
  return isRunning && !result.valid();
}

bool Runnable::interrupted() const
{
  return isInterrupted;
}

ThreadPool::ThreadPool(int threadN)
  : threads(threadN)
{
  
  for (Thread& thread : threads) {
    swap(thread, std::move(Thread(std::move(std::make_unique<PoolTask>(this)))));
    thread.run();
  }
}

ThreadPool::~ThreadPool()
{
  interrupt();
  join();
}

void ThreadPool::Push(std::unique_ptr<Runnable>& task)
{
  queueLock.lock();
  queue.push(std::move(task));
  queueLock.unlock();
}

void ThreadPool::interrupt()
{
  for (Thread& thread : threads) {
    thread.interrupt();
  }
}

void ThreadPool::join()
{
  for (Thread& thread : threads) {
    thread.join();
  }
}

void ThreadPool::Pop(std::unique_ptr<Runnable>& task)
{
  if (queueLock.try_lock()) {
    if (!queue.empty()) {
      task.swap(queue.back());
      queue.pop();
    }
    queueLock.unlock();
  }
}

ThreadPool::PoolTask::PoolTask(ThreadPool* pool)
  : pool(pool)
{
}

void ThreadPool::PoolTask::run()
{
  while (!interrupted()) {
    ::Sleep(10);
    std::unique_ptr<Runnable> task = nullptr;
      pool->Pop(task);
    if (task) {
      task->run();
    }
  }
}

void swap(Thread & a, Thread & b)
{
  using std::swap;
  swap(a.task, b.task);
  swap(a.result, b.result);
  swap(a.isRunning, b.isRunning);
}
