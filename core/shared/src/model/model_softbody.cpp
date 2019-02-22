#include "stdafx_shared.h"
#include "pragma/model/model.h"
#include "pragma/physics/physsoftbodyinfo.hpp"
/*
void Model::SetUseSoftBodyPhysics(bool b)
{
	if(b == false)
	{
		m_softBodyInfo = nullptr;
		return;
	}
	m_softBodyInfo = std::make_shared<PhysSoftBodyInfo>();
}
bool Model::GetUseSoftBodyPhysics() const {return (m_softBodyInfo != nullptr) ? true : false;}
const PhysSoftBodyInfo *Model::GetSoftBodyInfo() const {return const_cast<Model*>(this)->GetSoftBodyInfo();}
PhysSoftBodyInfo *Model::GetSoftBodyInfo() {return m_softBodyInfo.get();}
*/