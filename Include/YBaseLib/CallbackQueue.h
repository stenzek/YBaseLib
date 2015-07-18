#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/Functor.h"
#include "YBaseLib/PODArray.h"
#include "YBaseLib/Mutex.h"

class CallbackQueue
{
public:
    CallbackQueue();
    ~CallbackQueue();

    void AddCallback(Functor *pCallback);

    void RunCallbacks();

private:
    typedef PODArray<Functor *> CallbackFunctionQueue;
    CallbackFunctionQueue m_CallbackQueue;
    Mutex m_CallbackQueueLock;
};

