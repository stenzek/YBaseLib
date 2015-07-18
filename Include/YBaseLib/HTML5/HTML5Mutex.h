#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/Assert.h"

class Mutex
{
    friend class ConditionVariable;

public:
    Mutex()
    {

    }

    ~Mutex()
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
