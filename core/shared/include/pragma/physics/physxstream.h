#ifndef __PHYSXSTREAM_H__
#define __PHYSXSTREAM_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/physics/physxapi.h"
#include "pragma/networkdefinitions.h"

namespace physx
{
	class DLLNETWORK MemoryOutputStream : public physx::PxOutputStream
	{
	public:
		MemoryOutputStream();
		virtual ~MemoryOutputStream() override;
		physx::PxU32 write(const void* src, physx::PxU32 count);
		physx::PxU32 getSize() const {return m_Size;}
		physx::PxU8* getData() const {return m_Data;}
	private:
		physx::PxU8* m_Data;
		physx::PxU32 m_Size;
		physx::PxU32 m_Capacity;
	};

	class DLLNETWORK MemoryInputData: public physx::PxInputData
	{
	public:
		MemoryInputData(physx::PxU8* data, physx::PxU32 length);
		physx::PxU32 read(void* dest, physx::PxU32 count);
		physx::PxU32 getLength() const;
		void seek(physx::PxU32 pos);
		physx::PxU32 tell() const;
	private:
		physx::PxU32 m_Size;
		const physx::PxU8* m_Data;
		physx::PxU32 m_Pos;
	};
}

#endif

#endif