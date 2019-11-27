#pragma once
#include "YBaseLib/Atomic.h"
#include "YBaseLib/Event.h"
#include <semaphore.h>

class Barrier
{
public:
  Barrier(uint32 threadCount);
  ~Barrier();

  const uint32 GetThreadCount() const { return m_threadCount; }
  void SetThreadCount(uint32 threadCount);

  void Wait();

private:
  uint32 m_threadCount;
  Y_ATOMIC_DECL uint32 m_enteredThreadCount;
  Y_ATOMIC_DECL uint32 m_exitedThreadCount;
  sem_t m_hEnterSemaphore;
  sem_t m_hExitSemaphore;
};
