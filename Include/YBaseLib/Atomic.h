#pragma once

#include "YBaseLib/Common.h"

// declaration macro
#define Y_ATOMIC_DECL ALIGN_DECL(4) volatile
#define Y_ATOMIC_DECL64 ALIGN_DECL(8) volatile
#ifdef Y_CPU_X64
    #define Y_ATOMIC_PTR_DECL(type) ALIGN_DECL(8) type *volatile
#else
    #define Y_ATOMIC_PTR_DECL(type) ALIGN_DECL(4) type *volatile
#endif

// atomic functions
template<typename T> T Y_AtomicIncrement(volatile T &Value);
template<typename T> T Y_AtomicDecrement(volatile T &Value);
template<typename T> T Y_AtomicCompareExchange(volatile T &Result, T Exchange, T Comparand);
template<typename T> T Y_AtomicExchange(volatile T &Result, T Exchange);
template<typename T> T Y_AtomicAnd(volatile T &Value, T AndVal);
template<typename T> T Y_AtomicAdd(volatile T &Value, T AddVal);

// for pointer types
void *Y_AtomicCompareExchangeVoidPointer(void *volatile &Pointer, void *Exchange, void *Comparand);
void *Y_AtomicExchangeVoidPointer(void *volatile &Pointer, void *Exchange);
template<typename T> T Y_AtomicCompareExchangePointer(T volatile &Pointer, T Exchange, T Comparand)
{ 
    return reinterpret_cast<T>(
        Y_AtomicCompareExchangeVoidPointer(reinterpret_cast<void *volatile &>(Pointer),
                                           reinterpret_cast<void *>(Exchange),
                                           reinterpret_cast<void *>(Comparand)));
}

template<typename T> T Y_AtomicExchangePointer(T volatile &Pointer, T Exchange)
{
    return reinterpret_cast<T>(
        Y_AtomicExchangeVoidPointer(reinterpret_cast<void *volatile &>(Pointer),
                                    reinterpret_cast<void *>(Exchange)));
}

// atomic pointer class
template<class T>
class AtomicPointer
{
public:
    AtomicPointer() : m_ptr(nullptr) {}
    AtomicPointer(T *ptr) : m_ptr(ptr) {}

    T *Load() const { return m_ptr; }

    bool Set(T *ptr) { return (Y_AtomicCompareExchangePointer<T *>(m_ptr, ptr, nullptr) == nullptr); }
    bool Clear(T *ptr) { return (Y_AtomicCompareExchangePointer<T *>(m_ptr, nullptr, ptr) == nullptr); }

    const T &operator*() const { return *Load(); }
    T &operator*() { return *Load(); }
    const T *operator->() const { return Load(); }
    T *operator->() { return Load(); }
    operator const T *() const { return Load(); }
    operator T *() { return Load(); }

    bool operator==(const AtomicPointer<T> &Other) const { return (Load() == Other.Load()); }
    bool operator!=(const AtomicPointer<T> &Other) const { return (Load() != Other.Load()); }
    bool operator==(const T *pPtr) const { return (Load() == Load()); }
    bool operator!=(const T *pPtr) const { return (Load() != Load()); }

private:
    Y_ATOMIC_PTR_DECL(T) m_ptr;
};
