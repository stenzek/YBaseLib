#include "YBaseLib/Assert.h"
#include "YBaseLib/Barrier.h"

#ifdef Y_PLATFORM_ANDROID

#define MAX_THREADS 1024

Barrier::Barrier(uint32 threadCount) : m_threadCount(threadCount), m_enteredThreadCount(0), m_exitedThreadCount(0)
{
  sem_init(&m_hEnterSemaphore, 0, 0);
  sem_init(&m_hExitSemaphore, 0, 0);
}

Barrier::~Barrier()
{
  sem_destroy(&m_hExitSemaphore);
  sem_destroy(&m_hEnterSemaphore);
}

void Barrier::SetThreadCount(uint32 threadCount)
{
  Assert(m_enteredThreadCount == 0);
  m_threadCount = threadCount;
}

void Barrier::Wait()
{
  if (Y_AtomicIncrement(m_enteredThreadCount) == m_threadCount)
  {
    m_exitedThreadCount = 0;
    for (uint32 i = 1; i < m_threadCount; i++)
      sem_post(&m_hEnterSemaphore);
  }
  else
  {
    sem_wait(&m_hEnterSemaphore);
  }

  if (Y_AtomicIncrement(m_exitedThreadCount) == m_threadCount)
  {
    m_enteredThreadCount = 0;
    for (uint32 i = 1; i < m_threadCount; i++)
      sem_post(&m_hExitSemaphore);
  }
  else
  {
    sem_wait(&m_hExitSemaphore);
  }
}

#endif
