#include "YBaseLib/Common.h"

#if defined(Y_PLATFORM_POSIX)

#include "YBaseLib/POSIX/POSIXConditionVariable.h"
#include "YBaseLib/POSIX/POSIXMutex.h"
#include "YBaseLib/POSIX/POSIXRecursiveMutex.h"

ConditionVariable::ConditionVariable()
{
  pthread_cond_init(&m_ConditionVariable, NULL);
}

ConditionVariable::~ConditionVariable()
{
  pthread_cond_destroy(&m_ConditionVariable);
}

void ConditionVariable::SleepAndRelease(Mutex* pMutex)
{
  pthread_cond_wait(&m_ConditionVariable, &pMutex->m_Mutex);
}

void ConditionVariable::SleepAndRelease(RecursiveMutex* pMutex)
{
  pthread_cond_wait(&m_ConditionVariable, &pMutex->m_Mutex);
}

void ConditionVariable::WakeAll()
{
  pthread_cond_broadcast(&m_ConditionVariable);
}

void ConditionVariable::Wake()
{
  pthread_cond_signal(&m_ConditionVariable);
}

#endif
