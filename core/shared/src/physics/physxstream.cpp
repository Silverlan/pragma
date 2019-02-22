#include "stdafx_shared.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/physics/physxstream.h"

namespace physx
{
	MemoryOutputStream::MemoryOutputStream() : m_Data(NULL),
		m_Size(0),
		m_Capacity(0)
	{
	}

	MemoryOutputStream::~MemoryOutputStream()
	{
		if(m_Data)
			delete[] m_Data;
	}

	PxU32 MemoryOutputStream::write(const void* src, PxU32 size)
	{
		PxU32 expectedSize = m_Size + size;
		if(expectedSize > m_Capacity)
		{
			m_Capacity = expectedSize + 4096;

			PxU8* newData = new PxU8[m_Capacity];
			PX_ASSERT(newData!=NULL);

			if(newData)
			{
				memcpy(newData, m_Data, m_Size);
				delete[] m_Data;
			}
			m_Data = newData;
		}
		memcpy(m_Data+m_Size, src, size);
		m_Size += size;
		return size;
	}



	MemoryInputData::MemoryInputData(PxU8* data, PxU32 length) : m_Size(length),
		m_Data(data),
		m_Pos(0)
	{
	}

	PxU32 MemoryInputData::read(void* dest, PxU32 count)
	{
		PxU32 length = PxMin<PxU32>(count, m_Size-m_Pos);
		memcpy(dest, m_Data+m_Pos, length);
		m_Pos += length;
		return length;
	}

	PxU32 MemoryInputData::getLength() const
	{
		return m_Size;
	}

	void MemoryInputData::seek(PxU32 offset)
	{
		m_Pos = PxMin<PxU32>(m_Size, offset);
	}

	PxU32 MemoryInputData::tell() const
	{
		return m_Pos;
	}
}
#endif