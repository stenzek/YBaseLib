#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/Memory.h"
#include "YBaseLib/Assert.h"

class BitSet8
{
public:
    BitSet8()
    {
        m_bitSize = 0;
        m_byteSize = 0;
        m_pHeapPtr = nullptr;
        m_pPointer = nullptr;
    }

    BitSet8(const BitSet8 &copy)
        : m_pHeapPtr(nullptr),
          m_pPointer(nullptr),
          m_byteSize(0),
          m_bitSize(0)
    {
        Copy(copy);
    }

    BitSet8(uint32 length)
    {
        m_bitSize = length;
        m_byteSize = length / 8 + 1; 
        m_pHeapPtr = new uint8[m_byteSize];
        Y_memzero(m_pHeapPtr, sizeof(uint8) * m_byteSize);
        m_pPointer = m_pHeapPtr;
    }

    // ptr should be at least length/8+1 bytes long.
    BitSet8(uint8 *ptr, uint32 byteLength, uint32 bitLength)
    {
        m_bitSize = bitLength;
        m_byteSize = byteLength;
        m_pHeapPtr = NULL;
        m_pPointer = ptr;
    }

    ~BitSet8()
    {
        delete[] m_pHeapPtr;
    }

    inline bool operator[](uint32 index) const { return _IsSet(index); }
    inline void operator|=(uint32 index) { _Set(index); }
    inline void operator&=(uint32 index) { _Unset(index); }
    inline bool operator&(const BitSet8 &other) const { return TestSetsAnd(other); }
    inline bool operator==(const BitSet8 &other) const { return TestSetsEqual(other); }
    inline void operator|=(const BitSet8 &other) { _Combine(other); }
    inline void operator&=(const BitSet8 &other) { _Mask(other); }
    inline BitSet8 &operator=(const BitSet8 &other) { Copy(other); return *this; }

    inline const uint8 *GetPointer() const { return m_pPointer; }
    inline const uint32 GetByteCount() const { return m_byteSize; }
    inline uint8 *GetPointer() { return m_pPointer; }

    inline bool IsSet(uint32 index) const { return _IsSet(index); }
    inline void Set(uint32 index) { _Set(index); }
    inline void Unset(uint32 index) { _Unset(index); }

    void Clear()
    {
        Y_memzero(m_pHeapPtr, sizeof(uint8) * m_byteSize);
    }

    void Resize(uint32 length)
    {
        uint32 newByteSize = length / 8 + 1; 

        if (newByteSize != m_byteSize)
        {
            uint8 *pNewPointer = new uint8[newByteSize];
            uint32 copyLength = Min(newByteSize, m_byteSize);
            uint32 zeroLength = newByteSize - copyLength;
            if (copyLength > 0)
                Y_memcpy(pNewPointer, m_pPointer, sizeof(uint8) * copyLength);
            if (zeroLength > 0)
                Y_memzero(pNewPointer + copyLength, sizeof(uint8) * zeroLength);

            delete[] m_pHeapPtr;
            m_pHeapPtr = pNewPointer;
            m_pPointer = pNewPointer;
            m_byteSize = newByteSize;
        }

        m_bitSize = length;
    }

    bool TestSetsAnd(const BitSet8 &other) const
    {
        DebugAssert(other.m_byteSize == m_byteSize);
        for (uint32 i = 0; i < m_byteSize; i++)
        {
            if ((m_pPointer[i] & other.m_pPointer[i]) != 0)
                return true;
        }

        return false;
    }

    bool TestSetsEqual(const BitSet8 &other) const
    {
        DebugAssert(other.m_byteSize == m_byteSize);
        for (uint32 i = 0; i < m_byteSize; i++)
        {
            if (m_pPointer[i] != other.m_pPointer[i])
                return false;
        }

        return true;
    }

    void Copy(const BitSet8 &copy)
    {
        if (m_pHeapPtr != nullptr)
            delete[] m_pHeapPtr;

        if (copy.m_pPointer == nullptr)
        {
            m_bitSize = 0;
            m_byteSize = 0;
            m_pHeapPtr = NULL;
            m_pPointer = NULL;
        }
        else
        {
            m_pPointer = m_pHeapPtr = new uint8[copy.m_byteSize];
            m_bitSize = copy.m_bitSize;
            m_byteSize = copy.m_byteSize;
            Y_memcpy(m_pPointer, copy.m_pPointer, sizeof(uint8) * copy.m_byteSize);
        }            
    }

    void Swap(BitSet8 &other)
    {
        if (m_pHeapPtr == nullptr || other.m_pHeapPtr == nullptr)
        {
            BitSet8 temp(*this);
            Copy(other);
            other.Copy(temp);
        }
        else
        {
            ::Swap(m_pHeapPtr, other.m_pHeapPtr);
            ::Swap(m_pPointer, other.m_pPointer);
            ::Swap(m_byteSize, other.m_byteSize);
            ::Swap(m_bitSize, other.m_bitSize);
        }
    }

private:
    bool _IsSet(uint32 bit) const
    {
        uint32 arrayindex = (bit / 8);
        uint8 mask = 1 << (bit % 8);
        DebugAssert(bit < m_bitSize);
        DebugAssert(arrayindex < m_byteSize);
        return (m_pPointer[arrayindex] & mask) != 0;
    }

    void _Set(uint32 bit)
    {
        uint32 arrayindex = (bit / 8);
        uint8 mask = 1 << (bit % 8);
        DebugAssert(bit < m_bitSize);
        DebugAssert(arrayindex < m_byteSize);
        m_pPointer[arrayindex] |= mask;
    }

    void _Unset(uint32 bit)
    {
        uint32 arrayindex = (bit / 8);
        uint8 mask = 1 << (bit % 8);
        DebugAssert(bit < m_bitSize);
        DebugAssert(arrayindex < m_byteSize);
        m_pPointer[arrayindex] &= ~mask;
    }

    void _Combine(const BitSet8 &other)
    {
        DebugAssert(other.m_byteSize == m_byteSize);
        for (uint32 i = 0; i < m_byteSize; i++)
            m_pPointer[i] |= other.m_pPointer[i];
    }

    void _Mask(const BitSet8 &other)
    {
        DebugAssert(other.m_byteSize == m_byteSize);
        for (uint32 i = 0; i < m_byteSize; i++)
            m_pPointer[i] &= other.m_pPointer[i];
    }

    uint8 *m_pHeapPtr;
    uint8 *m_pPointer;
    uint32 m_byteSize;
    uint32 m_bitSize;
};

class BitSet32
{
public:
    BitSet32()
        : m_pHeapPtr(nullptr),
          m_pPointer(nullptr),
          m_dwordSize(0),
          m_bitSize(0)
    {

    }

    BitSet32(uint32 length)
    {
        m_bitSize = length;
        m_dwordSize = length / 32 + 1; 
        m_pHeapPtr = new uint32[m_dwordSize];
        Y_memzero(m_pHeapPtr, sizeof(uint32) * m_dwordSize);
        m_pPointer = m_pHeapPtr;
    }
    
    BitSet32(const BitSet32 &other)
        : m_pHeapPtr(nullptr),
          m_pPointer(nullptr),
          m_dwordSize(0),
          m_bitSize(0)
    {
        Copy(other);
    }

    // ptr should be at least length/32+1 dwords long.
    BitSet32(uint32 *ptr, uint32 byteLength, uint32 bitLength)
    {
        m_bitSize = bitLength;
        m_dwordSize = byteLength;
        m_pHeapPtr = NULL;
        m_pPointer = ptr;
    }

    ~BitSet32()
    {
        delete[] m_pHeapPtr;
    }

    inline bool operator[](uint32 index) const { return _IsSet(index); }
    inline void operator|=(uint32 index) { _Set(index); }
    inline void operator&=(uint32 index) { _Unset(index); }
    inline bool operator&(const BitSet32 &other) const { return TestSetsAnd(other); }
    inline bool operator==(const BitSet32 &other) const { return TestSetsEqual(other); }
    inline void operator|=(const BitSet32 &other) { _Combine(other); }
    inline void operator&=(const BitSet32 &other) { _Mask(other); }
    inline BitSet32 &operator=(const BitSet32 &other) { Copy(other); return *this; }

    inline const uint32 *GetPointer() const { return m_pPointer; }
    inline const uint32 GetDWordCount() const { return m_dwordSize; }
    inline uint32 *GetPointer() { return m_pPointer; }

    inline bool IsSet(uint32 index) const { return _IsSet(index); }
    inline void Set(uint32 index) { _Set(index); }
    inline void Unset(uint32 index) { _Unset(index); }

    void Clear()
    {
        Y_memzero(m_pHeapPtr, m_dwordSize * 4);
    }

    void Resize(uint32 length)
    {
        uint32 newDWordSize = length / 32 + 1; 
        if (newDWordSize != m_dwordSize)
        {
            uint32 *pNewPointer = new uint32[newDWordSize];
            uint32 copyLength = Min(newDWordSize, m_dwordSize);
            uint32 zeroLength = newDWordSize - copyLength;
            if (copyLength > 0)
                Y_memcpy(pNewPointer, m_pPointer, sizeof(uint32) * copyLength);
            if (zeroLength > 0)
                Y_memzero(pNewPointer + copyLength, sizeof(uint32) * zeroLength);

            delete[] m_pHeapPtr;
            m_pHeapPtr = pNewPointer;
            m_pPointer = pNewPointer;
            m_dwordSize = newDWordSize;
        }

        m_bitSize = length;
    }

    bool TestSetsAnd(const BitSet32 &other) const
    {
        DebugAssert(other.m_dwordSize == m_dwordSize);
        for (size_t i = 0; i < m_dwordSize; i++)
        {
            if ((m_pPointer[i] & other.m_pPointer[i]) != 0)
                return true;
        }

        return false;
    }

    bool TestSetsEqual(const BitSet32 &other) const
    {
        DebugAssert(other.m_dwordSize == m_dwordSize);
        for (size_t i = 0; i < m_dwordSize; i++)
        {
            if (m_pPointer[i] != other.m_pPointer[i])
                return false;
        }

        return true;
    }

    void Copy(const BitSet32 &copy)
    {
        if (m_pHeapPtr != nullptr)
            delete[] m_pHeapPtr;

        if (copy.m_pPointer == nullptr)
        {
            m_bitSize = 0;
            m_dwordSize = 0;
            m_pHeapPtr = nullptr;
            m_pPointer = nullptr;
        }
        else
        {
            m_pPointer = m_pHeapPtr = new uint32[copy.m_dwordSize];
            m_bitSize = copy.m_bitSize;
            m_dwordSize = copy.m_dwordSize;
            Y_memcpy(m_pPointer, copy.m_pPointer, sizeof(uint32)* copy.m_dwordSize);
        }
    }

    void Swap(BitSet32 &other)
    {
        if (m_pHeapPtr == nullptr || other.m_pHeapPtr == nullptr)
        {
            BitSet32 temp(*this);
            Copy(other);
            other.Copy(temp);
        }
        else
        {
            ::Swap(m_pHeapPtr, other.m_pHeapPtr);
            ::Swap(m_pPointer, other.m_pPointer);
            ::Swap(m_dwordSize, other.m_dwordSize);
            ::Swap(m_bitSize, other.m_bitSize);
        }
    }

private:
    bool _IsSet(uint32 bit) const
    {
        uint32 arrayindex = (bit / 32);
        uint32 mask = 1 << (bit % 32);
        DebugAssert(bit < m_bitSize);
        DebugAssert(arrayindex < m_dwordSize);
        return (m_pPointer[arrayindex] & mask) != 0;
    }

    void _Set(uint32 bit)
    {
        uint32 arrayindex = (bit / 32);
        uint32 mask = 1 << (bit % 32);
        DebugAssert(bit < m_bitSize);
        DebugAssert(arrayindex < m_dwordSize);
        m_pPointer[arrayindex] |= mask;
    }

    void _Unset(uint32 bit)
    {
        uint32 arrayindex = (bit / 32);
        uint32 mask = 1 << (bit % 32);
        DebugAssert(bit < m_bitSize);
        DebugAssert(arrayindex < m_dwordSize);
        m_pPointer[arrayindex] &= ~mask;
    }

    void _Combine(const BitSet32 &other)
    {
        DebugAssert(other.m_dwordSize == m_dwordSize);
        for (size_t i = 0; i < m_dwordSize; i++)
            m_pPointer[i] |= other.m_pPointer[i];
    }

    void _Mask(const BitSet32 &other)
    {
        DebugAssert(other.m_dwordSize == m_dwordSize);
        for (size_t i = 0; i < m_dwordSize; i++)
            m_pPointer[i] &= other.m_pPointer[i];
    }

    uint32 *m_pHeapPtr;
    uint32 *m_pPointer;
    uint32 m_dwordSize;
    uint32 m_bitSize;
};


