#pragma once

#include "YBaseLib/Common.h"

// declaration macro
#define Y_ATOMIC_DECL ALIGN_DECL(4) volatile
#define Y_ATOMIC_DECL64 ALIGN_DECL(8) volatile

// atomic functions
template<typename T> T Y_AtomicIncrement(volatile T &Value);
template<typename T> T Y_AtomicDecrement(volatile T &Value);
template<typename T> T Y_AtomicCompareExchange(volatile T &Result, T Exchange, T Comparand);
template<typename T> T Y_AtomicExchange(volatile T &Result, T Exchange);
template<typename T> T Y_AtomicAnd(volatile T &Value, T AndVal);
template<typename T> T Y_AtomicAdd(volatile T &Value, T AddVal);

// for pointer types
void *Y_AtomicCompareExchangeVoidPointer(volatile void *&Pointer, void *Exchange, void *Comparand);
void *Y_AtomicExchangeVoidPointer(volatile void *&Pointer, void *Exchange);
template<typename T> T Y_AtomicCompareExchangePointer(volatile T &Pointer, T Exchange, T Comparand)
{ 
    return reinterpret_cast<T>(
        Y_AtomicCompareExchangeVoidPointer(reinterpret_cast<volatile void *&>(&Pointer),
                                           reinterpret_cast<void *>(Exchange),
                                           reinterpret_cast<void *>(Comparand)));
}

template<typename T> T Y_AtomicExchangePointer(volatile T &Pointer, T Exchange)
{
    return reinterpret_cast<T>(
        Y_AtomicExchangeVoidPointer(reinterpret_cast<volatile void *&>(&Pointer),
                                    reinterpret_cast<void *>(Exchange)));
}
