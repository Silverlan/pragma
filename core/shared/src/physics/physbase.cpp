#include "stdafx_shared.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physbase.h"

DEFINE_BASE_HANDLE(DLLNETWORK,PhysBase,Phys);

PhysBase::PhysBase(PhysEnv *env)
	: m_physEnv(env)
{
	m_baseHandle = new PhysHandle(this);
}

PhysBase::~PhysBase()
{
	m_baseHandle->Invalidate();
	delete m_baseHandle;
}

PhysHandle *PhysBase::CreateBaseHandle() {return m_baseHandle->Copy();}
PhysHandle PhysBase::GetBaseHandle() {return *m_baseHandle;}