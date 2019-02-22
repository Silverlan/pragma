#include "stdafx_shared.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/point_joint_base.h"
#include <algorithm>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseworld.h"
BasePointJoint::BasePointJoint()
{}

void BasePointJoint::SetKeyValue(std::string key,std::string val)
{
	std::transform(key.begin(),key.end(),key.begin(),::tolower);
	if(key == "source")
	{
		m_kvSource = val;
		std::transform(m_kvSource.begin(),m_kvSource.end(),m_kvSource.begin(),::tolower);
	}
	else if(key == "target")
	{
		m_kvTarget = val;
		std::transform(m_kvTarget.begin(),m_kvTarget.end(),m_kvTarget.begin(),::tolower);
	}
	else if(key == "target_origin")
		 StringToVector(val,m_posTarget);
}

void BasePointJoint::GetTargetEntities(std::vector<BaseEntity*> &entsSrc,std::vector<BaseEntity*> &entsTgt)
{
	BaseEntity &ent = GetEntity();
	NetworkState *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	if(m_kvSource.empty())
		entsSrc.push_back(dynamic_cast<BaseEntity*>(game->GetWorld()));
	else
	{
		game->FindEntitiesByClass(m_kvSource.c_str(),&entsSrc);
		game->FindEntitiesByName(m_kvSource.c_str(),&entsSrc);
	}

	if(m_kvTarget.empty())
		entsSrc.push_back(dynamic_cast<BaseEntity*>(game->GetWorld()));
	else
	{
		game->FindEntitiesByClass(m_kvTarget.c_str(),&entsTgt);
		game->FindEntitiesByName(m_kvTarget.c_str(),&entsTgt);
	}
}
#endif