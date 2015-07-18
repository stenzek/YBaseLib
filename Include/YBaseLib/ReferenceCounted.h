#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/Atomic.h"

// ReferenceCountedObject is NonCopyable.
class ReferenceCounted
{
    // we protect the constructor and destructor so they cannot be deleted from outside
protected:
    ReferenceCounted();
    virtual ~ReferenceCounted();

public:
    void AddRef() const;
    uint32 Release() const;

private:
    mutable Y_ATOMIC_DECL uint32 m_uReferenceCount;
    mutable uint32 m_uReferenceCountValid;

    // make noncopyable
private:
    ReferenceCounted(const ReferenceCounted &);
    ReferenceCounted &operator=(const ReferenceCounted &);
};

#define SAFE_RELEASE(x) MULTI_STATEMENT_MACRO_BEGIN \
                            if ((x) != NULL) { (x)->Release(); (x) = nullptr; } \
                        MULTI_STATEMENT_MACRO_END

#define FAST_RELEASE(x) MULTI_STATEMENT_MACRO_BEGIN \
                            { (x)->Release(); } \
                        MULTI_STATEMENT_MACRO_END

