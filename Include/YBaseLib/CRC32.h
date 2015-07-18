#pragma once
#include "YBaseLib/Common.h"

class CRC32
{
public:
    CRC32(uint32 start = 0);

    uint32 GetCRC() const { return m_currentCRC; }
    void Update(const void *buf, size_t len);
    void Reset();

private:
    uint32 m_currentCRC;
};
