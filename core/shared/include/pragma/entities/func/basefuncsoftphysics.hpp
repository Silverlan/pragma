#ifndef __BASEFUNCSOFTPHYSICS_HPP__
#define __BASEFUNCSOFTPHYSICS_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"

namespace pragma
{
	class DLLNETWORK BaseFuncSoftPhysicsComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		PhysSoftBodyInfo *GetSoftBodyInfo();
	protected:
		bool SetKeyValue(std::string key,std::string val);
		PhysSoftBodyInfo m_softBodyInfo = {};
	};
};

#endif
