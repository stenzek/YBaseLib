#include "YBaseLib/Memory.h"
#include "YBaseLib/Assert.h"
#include "YBaseLib/String.h"
#include <cstring>
#include <cstdlib>

#if defined(Y_PLATFORM_LINUX)
    #include <malloc.h>
#endif  

static void MemoryAllocationFailedHandler(size_t request_size)
{
    char msg[128];
    Y_snprintf(msg, countof(msg), "failed to allocate %u bytes of memory", (uint32)request_size);
    Y_OnPanicReached(msg, __FUNCTION__, __FILE__, __LINE__);
}

static void MemoryAllocationOverflowHandler(size_t num, size_t size)
{
    char msg[128];
    Y_snprintf(msg, countof(msg), "heap overflow when allocating %u of %u elements", (uint32)num, (uint32)size);
    Y_OnPanicReached(msg, __FUNCTION__, __FILE__, __LINE__);
}

// allocation overflow helper
#define REQUEST_SIZE_OVERFLOW_CHECK(num, size) MULTI_STATEMENT_MACRO_BEGIN \
                                                    if (size > 0 && num > SIZE_MAX / size) \
                                                        MemoryAllocationOverflowHandler(num, size); \
                                               MULTI_STATEMENT_MACRO_END

void Y_memcpy(void *pDestination, const void *pSource, size_t ByteCount)
{
    memcpy(pDestination, pSource, ByteCount);
}

void Y_memmove(void *pDestination, const void *pSource, size_t ByteCount)
{
    memmove(pDestination, pSource, ByteCount);
}

void Y_memzero(void *pDestination, size_t ByteCount)
{
    memset(pDestination, 0, ByteCount);
}

void Y_memset(void *pDestination, byte Value, size_t ByteCount)
{
    memset(pDestination, Value, ByteCount);
}

int32 Y_memcmp(const void *pMem1, const void *pMem2, size_t ByteCount)
{
    return memcmp(pMem1, pMem2, ByteCount);
}

void Y_memcpy_stride(void *pDestination, size_t DestinationStride, const void *pSource, size_t SourceStride, size_t Size, size_t Count)
{
    // Optimize me!
    const byte *pByteSource = (const byte *)pSource;
    byte *pByteDestination = (byte *)pDestination;

    for (size_t i = 0; i < Count; i++)
    {
        Y_memcpy(pByteDestination, pByteSource, Size);
        pByteSource += SourceStride;
        pByteDestination += DestinationStride;
    }
}

int32 Y_memcmp_stride(const void *pMem1, size_t mem1Stride, const void *pMem2, size_t mem2Stride, size_t elementSize, size_t count)
{
    // Optimize me!
    const byte *pByteMem1 = (const byte *)pMem1;
    const byte *pByteMem2 = (const byte *)pMem2;

    for (size_t i = 0; i < count; i++)
    {
        int res = memcmp(pByteMem1, pByteMem2, elementSize);
        if (res != 0)
            return res;
        
        pByteMem1 += mem1Stride;
        pByteMem2 += mem2Stride;
    }

    return 0;
}

void *Y_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return ptr;
}

void *Y_malloczero(size_t size)
{
    void *ptr = Y_malloc(size);
    Y_memzero(ptr, size);
    return ptr;
}

void *Y_mallocarray(size_t num, size_t size)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_malloc(num * size);
}

void *Y_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return new_ptr;
}

void *Y_reallocarray(void *ptr, size_t num, size_t size)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_realloc(ptr, num * size);
}

void Y_free(void *ptr)
{
    free(ptr);
}

#if defined(Y_COMPILER_MSVC)

void *Y_aligned_malloc(size_t size, size_t alignment)
{
    void *ptr = _aligned_malloc(size, alignment);
    if (ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return ptr;
}

void *Y_aligned_malloczero(size_t size, size_t alignment)
{
    void *ptr = Y_aligned_malloc(size, alignment);
    Y_memzero(ptr, size);
    return ptr;
}

void *Y_aligned_mallocarray(size_t num, size_t size, size_t alignment)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_aligned_malloc(num * size, alignment);
}

void *Y_aligned_realloc(void *ptr, size_t size, size_t alignment)
{
    void *new_ptr = _aligned_realloc(ptr, size, alignment);
    if (new_ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return new_ptr;
}

void *Y_aligned_reallocarray(void *ptr, size_t num, size_t size, size_t alignment)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_aligned_realloc(ptr, num * size, alignment);
}

void Y_aligned_free(void *ptr)
{
    return _aligned_free(ptr);
}

#elif defined(Y_PLATFORM_LINUX)

void *Y_aligned_malloc(size_t size, size_t alignment)
{
    void *ptr = memalign(alignment, size);
    if (ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return ptr;
}

void *Y_aligned_malloczero(size_t size, size_t alignment)
{
    void *ptr = Y_aligned_malloc(alignment, size);
    Y_memzero(ptr, size);
    return ptr;
}

void *Y_aligned_mallocarray(size_t num, size_t size, size_t alignment)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_aligned_malloc(num * size, alignment);
}

void *Y_aligned_realloc(void *ptr, size_t size, size_t alignment)
{
    void *new_ptr = memalign(size, alignment);
    if (new_ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    if (ptr != nullptr)
    {
        size_t copy_size = malloc_usable_size(ptr);
        if (copy_size > 0)
            memcpy(new_ptr, ptr, copy_size);

        free(ptr);
    }

    return ptr;
}

void *Y_aligned_reallocarray(void *ptr, size_t num, size_t size, size_t alignment)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_aligned_realloc(ptr, num * size, alignment);
}

void Y_aligned_free(void *ptr)
{
    free(ptr);
}

#elif defined(Y_PLATFORM_OSX)

void *Y_aligned_malloc(size_t size, size_t alignment)
{
    DebugAssert(alignment <= 16);
    void *ptr = malloc(size);
    if (ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return ptr;
}

void *Y_aligned_malloczero(size_t size, size_t alignment)
{
    DebugAssert(alignment <= 16);
    void *ptr = Y_aligned_malloc(alignment);
    Y_memzero(ptr, size);
    return ptr;
}

void *Y_aligned_mallocarray(size_t num, size_t size, size_t alignment)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_aligned_malloc(num * size, alignment);
}

void *Y_aligned_realloc(void *ptr, size_t size, size_t alignment)
{
    DebugAssert(alignment <= 16);
    void *new_ptr = realloc(ptr, size, alignment);
    if (new_ptr == nullptr)
        MemoryAllocationFailedHandler(size);

    return ptr;
}

void *Y_aligned_reallocarray(void *ptr, size_t num, size_t size, size_t alignment)
{
    REQUEST_SIZE_OVERFLOW_CHECK(num, size);
    return Y_aligned_realloc(ptr, num * size, alignment);
}

void Y_aligned_free(void *ptr)
{
    free(ptr);
}

#endif

void Y_qsort(void *pBase, size_t nElements, size_t ElementSize, int(*CompareFunction)(const void *, const void *))
{
    qsort(pBase, nElements, ElementSize, CompareFunction);
}

#if defined(Y_COMPILER_MSVC)

void Y_qsort_r(void *pBase, size_t nElements, size_t ElementSize, void *pContext, int(*CompareFunction)(void *, const void *, const void *))
{
   qsort_s(pBase, nElements, ElementSize, CompareFunction, pContext);
}

#elif defined(Y_PLATFORM_OSX)

void Y_qsort_r(void *pBase, size_t nElements, size_t ElementSize, void *pContext, int(*CompareFunction)(void *, const void *, const void *))
{
    qsort_r(pBase, nElements, ElementSize, pContext, CompareFunction);
}

#elif defined(Y_PLATFORM_HTML5) || defined(Y_PLATFORM_ANDROID)

// have to remap it.. this really needs a better solution :/
struct RecursiveQSortData
{
    int(*CompareFunction)(void *, const void *, const void *);
    void *pRealUserData;
};

Y_DECLARE_THREAD_LOCAL(RecursiveQSortData *g_pCurrentQSortData) = nullptr;

static int Y_qsort_trampoline(const void *pLeft, const void *pRight)
{
    return g_pCurrentQSortData->CompareFunction(g_pCurrentQSortData->pRealUserData, pLeft, pRight);
}

void Y_qsort_r(void *pBase, size_t nElements, size_t ElementSize, void *pContext, int(*CompareFunction)(void *, const void *, const void *))
{
    RecursiveQSortData *pOldValue = g_pCurrentQSortData;
    
    RecursiveQSortData data;
    data.CompareFunction = CompareFunction;
    data.pRealUserData = pContext;
    g_pCurrentQSortData = &data;

    qsort(pBase, nElements, ElementSize, Y_qsort_trampoline);

    g_pCurrentQSortData = pOldValue;
}

#else

// have to remap it.. this really needs a better solution :/
struct RecursiveQSortData
{
    int(*CompareFunction)(void *, const void *, const void *);
    void *pRealUserData;
};

static int Y_qsort_r_trampoline(const void *pLeft, const void *pRight, void *pUserData)
{
    RecursiveQSortData *pData = reinterpret_cast<RecursiveQSortData *>(pUserData);
    return pData->CompareFunction(pData->pRealUserData, pLeft, pRight);
}

void Y_qsort_r(void *pBase, size_t nElements, size_t ElementSize, void *pContext, int(*CompareFunction)(void *, const void *, const void *))
{
    RecursiveQSortData data;
    data.CompareFunction = CompareFunction;
    data.pRealUserData = pContext;
    qsort_r(pBase, nElements, ElementSize, Y_qsort_r_trampoline, reinterpret_cast<void *>(&data));
}

#endif

void *Y_bsearch(const void *pKey, const void *pBase, size_t nElements, size_t ElementSize, int(*CompareFunction)(const void *, const void *))
{
    return bsearch(pKey, pBase, nElements, ElementSize, CompareFunction);
}

// ---------------------------------------------------------------------------------------------------------------------
// overload the new operators
#ifndef Y_PLATFORM_HTML5

void *operator new(size_t uNumBytes)
{
    return Y_malloc(uNumBytes);
}

#if defined(Y_COMPILER_MSVC)
void operator delete(void *pMemory) 
#else
void operator delete(void *pMemory) noexcept
#endif
{
    return Y_free(pMemory);
}

#endif

