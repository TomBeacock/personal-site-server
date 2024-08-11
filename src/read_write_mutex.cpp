#include "read_write_mutex.h"

namespace Pss {
ReadWriteMutex::ReadWriteMutex() {}

void ReadWriteMutex::lock_read()
{
    {
        std::shared_lock<std::shared_mutex> lock(this->guard);
    }
    this->mutex.lock_shared();
}

void ReadWriteMutex::unlock_read()
{
    this->mutex.unlock_shared();
}

void ReadWriteMutex::lock_write()
{
    std::lock_guard<std::shared_mutex> lock(this->guard);
    this->mutex.lock();
}

void ReadWriteMutex::unlock_write()
{
    this->mutex.unlock();
}

ReadLock::ReadLock(ReadWriteMutex &mutex) : mutex(mutex)
{
    mutex.lock_read();
}

ReadLock::~ReadLock()
{
    mutex.unlock_read();
}

WriteLock::WriteLock(ReadWriteMutex &mutex) : mutex(mutex)
{
    mutex.lock_write();
}

WriteLock::~WriteLock()
{
    mutex.unlock_write();
}
}  // namespace Pss