#ifndef __BASEENTITY_LUAOBJECT_H__
#define __BASEENTITY_LUAOBJECT_H__

#include "pragma/entities/baseentity.h"

template<class THandle>
	void BaseEntity::InitializeHandle()
{
	m_handle = new THandle(new PtrEntity(this));
}

#endif