#ifndef __C_MODEL_COMPONENT_HPP__
#define __C_MODEL_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_model_component.hpp>

namespace pragma
{
	class DLLCLIENT CModelComponent final
		: public BaseModelComponent,
		public CBaseNetComponent
	{
	public:
		static ComponentEventId EVENT_ON_UPDATE_LOD;
		static ComponentEventId EVENT_ON_UPDATE_LOD_BY_POS;

		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CModelComponent(BaseEntity &ent) : BaseModelComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void Initialize() override;

		bool IsWeighted() const;

		virtual void UpdateLOD(uint32_t lod);
		uint8_t GetLOD();
		virtual void UpdateLOD(const Vector3 &posCam);
		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes() const;
		using BaseModelComponent::SetBodyGroup;
		using BaseModelComponent::SetModel;
		virtual void SetModel(const std::string &mdl) override;
		virtual bool SetBodyGroup(uint32_t groupId,uint32_t id) override;

		// Only use if LOD is handled externally!
		void SetLOD(uint8_t lod);
	protected:
		virtual void OnModelChanged(const std::shared_ptr<Model> &model) override;

		uint8_t m_lod = 0u; // Current level of detail
		std::vector<std::shared_ptr<ModelMesh>> m_lodMeshes;
	};

	// Events

	struct DLLCLIENT CEOnUpdateLOD
		: public ComponentEvent
	{
		CEOnUpdateLOD(uint32_t lod);
		virtual void PushArguments(lua_State *l) override;
		uint32_t lod;
	};

	struct DLLCLIENT CEOnUpdateLODByPos
		: public ComponentEvent
	{
		CEOnUpdateLODByPos(const Vector3 &posCam);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &posCam;
	};
};

#endif
