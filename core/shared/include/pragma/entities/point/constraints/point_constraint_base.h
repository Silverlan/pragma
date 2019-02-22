#ifndef __POINT_CONSTRAINT_BASE_H__
#define __POINT_CONSTRAINT_BASE_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/glmutil.h>
#include <pragma/physics/physapi.h>
#include "pragma/entities/components/basetoggle.h"
#include "pragma/physics/physconstraint.h"
#include <string>
#include <vector>

#define SF_CONSTRAINT_START_INACTIVE 1024
#define SF_CONSTRAINT_DISABLE_COLLISIONS 2048

class BaseEntity;
namespace pragma
{
	class DLLNETWORK BasePointConstraintComponent
		: public BaseEntityComponent
	{
	public:
		BasePointConstraintComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		std::vector<ConstraintHandle> &GetConstraints();
		virtual void OnRemove();
	protected:
		std::string m_kvSource;
		std::string m_kvTarget;
		CallbackHandle m_cbGameLoaded = {};
		Vector3 m_posTarget = {0.f,0.f,0.f};
		std::vector<ConstraintHandle> m_constraints;
		bool SetKeyValue(std::string key,std::string val);
		BaseEntity *GetSourceEntity();
		void GetTargetEntities(std::vector<BaseEntity*> &entsTgt);
		virtual void InitializeConstraint();
		virtual void InitializeConstraint(BaseEntity *src,BaseEntity *tgt);
		virtual void ClearConstraint();
		void OnTurnOn();
		void OnTurnOff();
	};
};

#endif
