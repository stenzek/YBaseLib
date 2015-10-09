#include "YBaseLib/CircularBuffer.h"
#include "YBaseLib/Memory.h"
#include "YBaseLib/Assert.h"

CircularBuffer::CircularBuffer()
    : m_pBuffer(nullptr)
    , m_pRegionAHead(m_pBuffer)
    , m_pRegionATail(m_pBuffer)
    , m_pRegionBTail(nullptr)
    , m_bufferSize(0)
    , m_ownsBuffer(true)
{

}

CircularBuffer::CircularBuffer(size_t bufferSize)
    : m_pBuffer((byte *)Y_malloc(bufferSize))
    , m_pRegionAHead(m_pBuffer)
    , m_pRegionATail(m_pBuffer)
    , m_pRegionBTail(nullptr)
    , m_bufferSize(bufferSize)
    , m_ownsBuffer(true)
{
    // @TODO safe malloc
    DebugAssert(bufferSize > 0);
}

CircularBuffer::CircularBuffer(byte *pBuffer, size_t bufferSize)
    : m_pBuffer(pBuffer)
    , m_pRegionAHead(m_pBuffer)
    , m_pRegionATail(m_pBuffer)
    , m_pRegionBTail(nullptr)
    , m_bufferSize(bufferSize)
    , m_ownsBuffer(false)
{

}

CircularBuffer::~CircularBuffer()
{
    if (m_ownsBuffer)
        Y_free(m_pBuffer);
}

void CircularBuffer::ResizeBuffer(size_t newBufferSize)
{
    DebugAssert(m_ownsBuffer && newBufferSize >= m_bufferSize);
    byte *pNewBuffer = (byte *)Y_realloc(m_pBuffer, newBufferSize);

    // re-align pointers to new buffer
    DebugAssert(pNewBuffer != nullptr);
    m_pRegionAHead = (m_pRegionAHead - m_pBuffer) + pNewBuffer;
    m_pRegionATail = (m_pRegionATail - m_pBuffer) + pNewBuffer;
    if (m_pRegionBTail != nullptr)
        m_pRegionBTail = (m_pRegionBTail - m_pBuffer) + pNewBuffer;
    
    // swap base pointer
    m_pBuffer = pNewBuffer;
    m_bufferSize = newBufferSize;
}

size_t CircularBuffer::GetBufferSpace() const
{
    // prefer using buffer B
    if (m_pRegionBTail != nullptr)
        return m_pRegionAHead - m_pRegionBTail;
    else
        return m_bufferSize - (m_pRegionATail - m_pRegionAHead);
}

size_t CircularBuffer::GetContiguousBufferSpace() const
{
    // prefer using buffer B
    if (m_pRegionBTail != nullptr)
        return m_pRegionAHead - m_pRegionBTail;
    else
    {
        size_t spaceA = (m_pBuffer + m_bufferSize) - m_pRegionATail;
        size_t spaceB = m_pRegionAHead - m_pBuffer;
        return Max(spaceA, spaceB);
    }
}

size_t CircularBuffer::GetBufferUsed() const
{
    // only include B's figures if it exists
    if (m_pRegionBTail != nullptr)
        return (m_pRegionBTail - m_pBuffer) + (m_pRegionATail - m_pRegionAHead);
    else
        return (m_pRegionATail - m_pRegionAHead);
}

size_t CircularBuffer::GetContiguousUsedBytes() const
{
    return m_pRegionATail - m_pRegionAHead;
}

bool CircularBuffer::GetReadPointer(const void **ppReadPointer, size_t *pByteCount) const
{
    // check for empty buffer
    if (m_pRegionATail == m_pRegionAHead)
        return false;

    // read from A first, keep in mind this is a contiguous pointer, 
    // so even if B exists, we can't use it yet. we'll sort that out 
    // in MoveReadPointer afterwards.
    *ppReadPointer = m_pRegionAHead;
    *pByteCount = m_pRegionATail - m_pRegionAHead;
    return true;
}

void CircularBuffer::MoveReadPointer(size_t byteCount)
{
    // move A's read pointer forward
    DebugAssert(byteCount <= static_cast<size_t>(m_pRegionATail - m_pRegionAHead));
    m_pRegionAHead += byteCount;

    // consumed region A?
    if (m_pRegionAHead == m_pRegionATail)
    {
        // if region B exists, "rename" it to region A
        if (m_pRegionBTail != nullptr)
        {
            m_pRegionAHead = m_pBuffer;
            m_pRegionATail = m_pRegionBTail;
            m_pRegionBTail = nullptr;
        }
        // otherwise just move the pointers back to the start
        else
        {
            m_pRegionAHead = m_pBuffer;
            m_pRegionATail = m_pBuffer;
        }
    }
}

bool CircularBuffer::GetWritePointer(void **ppWritePointer, size_t *pByteCount)
{
    size_t requiredBytes = *pByteCount;
    size_t freeSpace;

    // always allocate to B if it exists
    if (m_pRegionBTail != nullptr)
    {
        freeSpace = (m_pRegionAHead - m_pRegionBTail);
        if (requiredBytes > freeSpace)
            return false;

        *ppWritePointer = m_pBuffer;
        *pByteCount = freeSpace;
        return true;
    }

    // calculate space should B be created
    size_t spaceA = (m_pBuffer + m_bufferSize) - m_pRegionATail;
    size_t spaceB = m_pRegionAHead - m_pBuffer;

    // would creating region B allow more space than after A?
    if (spaceB > spaceA)
    {
        // size check
        if (requiredBytes > spaceB)
            return false;

        // allocate B
        m_pRegionBTail = m_pBuffer;
        *ppWritePointer = m_pRegionBTail;
        *pByteCount = spaceB;
        return true;
    }
    else
    {
        // leaves everything left in A
        if (requiredBytes > spaceA)
            return false;

        *ppWritePointer = m_pRegionATail;
        *pByteCount = spaceA;
        return true;
    }
}

void CircularBuffer::MoveWritePointer(size_t byteCount)
{
    if (m_pRegionBTail != nullptr)
    {
        // even if B was just allocated, it'll be non-null from above
        DebugAssert(byteCount <= static_cast<size_t>(m_pRegionAHead - m_pRegionBTail));
        m_pRegionBTail += byteCount;
    }
    else
    {
        // add to A size
        DebugAssert(byteCount <= static_cast<size_t>(m_pBuffer + m_bufferSize - m_pRegionATail));
        m_pRegionATail += byteCount;
    }
}

bool CircularBuffer::Read(void *pDestination, size_t byteCount)
{
    // @TODO since this type of operation does not require contiguous allocations, 
    // we can make the bip buffer behave more like a traditional circular buffer
    // as an operation, to skip multiple calls.
    const void *pReadPointer;
    size_t availableBytes;
    if (!GetReadPointer(&pReadPointer, &availableBytes) || byteCount > availableBytes)
        return false;

    // copy data
    Y_memcpy(pDestination, pReadPointer, byteCount);
    MoveReadPointer(byteCount);
    return true;
}

bool CircularBuffer::Write(const void *pSource, size_t byteCount)
{
    // @TODO same as above
    void *pWritePointer;
    size_t availableBytes;
    if (!GetWritePointer(&pWritePointer, &availableBytes))
        return false;

    // caveat here: GetWritePointer can initialize an empty region B
    if (byteCount > availableBytes && m_pRegionBTail == m_pBuffer)
    {
        // so clear it again just for consistency's sake
        m_pRegionBTail = nullptr;
        return false;
    }

    // copy data
    Y_memcpy(pWritePointer, pSource, byteCount);
    MoveWritePointer(byteCount);
    return true;
}
