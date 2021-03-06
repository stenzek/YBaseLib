#pragma once
#include "YBaseLib/Assert.h"
#include "YBaseLib/Common.h"
#include <poll.h>

class Event
{
public:
  Event(bool autoReset = false) : m_autoReset(autoReset)
  {
    m_pipeFds[0] = m_pipeFds[1] = -1;
#if defined(Y_PLATFORM_LINUX)
    pipe2(m_pipeFds, O_NONBLOCK);
#else
    pipe(m_pipeFds);
    fcntl(m_pipeFds[0], F_SETFL, fcntl(m_pipeFds[0], F_GETFL) | O_NONBLOCK);
    fcntl(m_pipeFds[1], F_SETFL, fcntl(m_pipeFds[1], F_GETFL) | O_NONBLOCK);
#endif
  }

  ~Event()
  {
    close(m_pipeFds[0]);
    close(m_pipeFds[1]);
  }

  void Signal()
  {
    char buf[1] = {0};
    write(m_pipeFds[1], buf, sizeof(buf));
  }

  void Wait()
  {
    pollfd pollDescriptors;
    pollDescriptors.fd = m_pipeFds[0];
    pollDescriptors.events = POLLRDNORM;
    poll(&pollDescriptors, 1, -1);

    if (m_autoReset)
      Reset();
  }

  bool TryWait(uint32 timeout)
  {
    pollfd pollDescriptors;
    pollDescriptors.fd = m_pipeFds[0];
    pollDescriptors.events = POLLRDNORM;
    if (poll(&pollDescriptors, 1, timeout) == 0)
      return false;

    if (m_autoReset)
      Reset();

    return true;
  }

  void Reset()
  {
    char buf[1];
    while (read(m_pipeFds[0], buf, sizeof(buf)) > 0)
      ;
  }

  static void WaitForMultipleEvents(Event** ppEvents, uint32 nEvents)
  {
    DebugAssert(nEvents > 0);

    uint32 i;
    pollfd pollDescriptors;
    pollDescriptors.events = POLLRDNORM;
    for (i = 0; i < nEvents; i++)
    {
      pollDescriptors.fd = ppEvents[i]->m_pipeFds[0];
      poll(&pollDescriptors, 1, -1);
    }
  }

private:
  int m_pipeFds[2];
  bool m_autoReset;
};
