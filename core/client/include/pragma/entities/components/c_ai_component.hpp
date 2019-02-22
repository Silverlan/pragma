#ifndef __C_AI_COMPONENT_HPP__
#define __C_AI_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_ai_component.hpp>
#include <pragma/util/util_handled.hpp>

class CBaseEntity;
namespace pragma
{
	class DLLCLIENT CAIComponent final
		: public BaseAIComponent,
		public CBaseSnapshotComponent
	{
	private:
		static std::vector<CAIComponent*> s_npcs;
	protected:
		Vector3 OnCalcMovementDirection(const Vector3 &forward,const Vector3 &right) const;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	public:
		static unsigned int GetNPCCount();
		static const std::vector<CAIComponent*> &GetAll();
		CAIComponent(BaseEntity &ent);
		virtual ~CAIComponent() override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual bool ShouldTransmitNetData() const override {return false;}
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
		virtual bool ShouldTransmitSnapshotData() const override {return true;}
	};
};

#endif
