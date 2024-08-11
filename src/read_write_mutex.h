#pragma once

#include <shared_mutex>

namespace Pss {
class ReadWriteMutex {
  public:
    ReadWriteMutex();

    void lock_read();
    void unlock_read();
    void lock_write();
    void unlock_write();

  private:
    std::shared_mutex guard;
    std::shared_mutex mutex;
};

class ReadLock {
  public:
    ReadLock(ReadWriteMutex &mutex);
    ~ReadLock();

  private:
    ReadWriteMutex &mutex;
};

class WriteLock {
  public:
    WriteLock(ReadWriteMutex &mutex);
    ~WriteLock();

  private:
    ReadWriteMutex &mutex;
};
}  // namespace Pss