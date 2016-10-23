#ifndef _THREAD_HPP_
#define _THREAD_HPP_

#include <future>
#include <queue>
#include <vector>

class Runnable {
  friend class Thread;
  friend class ThreadPool;
public:
  bool interrupted() const;
protected:
  virtual void run() = 0;
private:
  volatile bool isInterrupted = false;
};

class Thread {
public:
  Thread() = default;
  Thread(Thread&&) = default;
  Thread(const Thread&) = delete;
  Thread(std::unique_ptr<Runnable>&& task);
  void interrupt();
  void run();
  void join();
  bool joinable() const;
  bool interrupted() const;
  bool running() const;

  friend void swap(Thread& a, Thread& b);

private:
  std::future<void> result;
  std::unique_ptr<Runnable> task;
  volatile bool isRunning;
};

class ThreadPool {
  class PoolTask;
public:
  ThreadPool(int threadN);
  ~ThreadPool();
  void Push(std::unique_ptr<Runnable>& task);
  void interrupt();
  void join();
private:
  void Pop(std::unique_ptr<Runnable>& task);
  std::queue<std::unique_ptr<Runnable>> queue;
  std::mutex queueLock;
  std::vector<Thread> threads;

  class PoolTask : public Runnable {
  public:
    PoolTask(ThreadPool* pool = nullptr);
  protected:
    virtual void run() override;
  private:
    ThreadPool* pool;
  };
};

#endif

