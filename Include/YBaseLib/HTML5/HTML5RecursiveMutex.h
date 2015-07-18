#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/Assert.h"

class RecursiveMutex
{
    friend class ConditionVariable;

public:
    RecursiveMutex()
    {

    }

    ~RecursiveMutex()
    {

    }

    void Lock()
    {

    }

    bool TryLock()
    {
        return true;
    }

    void Unlock()
    {

    }

private:
};
