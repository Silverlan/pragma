/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_MODEL_COMPONENT_HPP__
#define __BASE_MODEL_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

class Model;
enum class Activity : uint16_t;
namespace pragma
{
	struct DLLNETWORK CEOnBodyGroupChanged
		: public ComponentEvent
	{
		CEOnBodyGroupChanged(uint32_t groupId,uint32_t typeId);
		virtual void PushArguments(lua_State *l) override;
		uint32_t groupId;
		uint32_t typeId;
	};
	struct DLLNETWORK CEOnSkinChanged
		: public ComponentEvent
	{
		CEOnSkinChanged(uint32_t skinId);
		virtual void PushArguments(lua_State *l) override;
		uint32_t skinId;
	};
	struct DLLNETWORK CEOnModelChanged
		: public ComponentEvent
	{
		CEOnModelChanged(const std::shared_ptr<Model> &model);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<Model> model;
	};
	class DLLNETWORK BaseModelComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_MODEL_CHANGED;
		static ComponentEventId EVENT_ON_MODEL_MATERIALS_LOADED;
		static ComponentEventId EVENT_ON_SKIN_CHANGED;
		static ComponentEventId EVENT_ON_BODY_GROUP_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		int LookupBlendController(const std::string &controller) const;
		Int32 LookupBone(const std::string &name) const;
		int LookupAnimation(const std::string &name) const;
		int LookupAttachment(const std::string &name) const;
		void GetAnimations(Activity activity,std::vector<unsigned int> &animations) const;
		unsigned char GetAnimationActivityWeight(unsigned int animation) const;
		Activity GetAnimationActivity(unsigned int animation) const;
		float GetAnimationDuration(unsigned int animation) const;
		void OnModelMaterialsLoaded();
		bool HasModelMaterialsLoaded() const;
		void SetModel(const std::string &mdl);
		virtual void SetModel(const std::shared_ptr<Model> &mdl);
		const std::shared_ptr<Model> &GetModel() const;
		std::string GetModelName() const;
		bool HasModel() const;
		unsigned int GetSkin() const;
		const std::shared_ptr<util::UInt32Property> &GetSkinProperty() const;
		virtual void SetSkin(unsigned int skin);
		const std::vector<uint32_t> &GetBodyGroups() const;
		UInt32 GetBodyGroup(unsigned int groupId) const;
		void SetBodyGroup(const std::string &name,UInt32 id);
		virtual bool SetBodyGroup(UInt32 groupId,UInt32 id);

		virtual void SetMaxDrawDistance(float maxDist);
		float GetMaxDrawDistance() const;

		bool GetAttachment(unsigned int attID,Vector3 *pos,EulerAngles *angles) const;
		bool GetAttachment(const std::string &name,Vector3 *pos,EulerAngles *angles) const;
		bool GetAttachment(unsigned int attID,Vector3 *pos,Quat *rot) const;
		bool GetAttachment(const std::string &name,Vector3 *pos,Quat *rot) const;

		uint32_t GetFlexControllerCount() const;
		bool LookupFlexController(const std::string &name,uint32_t &flexId) const;

		virtual void Save(DataStream &ds) override;
		using BaseEntityComponent::Load;

		uint32_t GetHitboxCount() const;
		bool GetHitboxBounds(uint32_t boneId,Vector3 &min,Vector3 &max,Vector3 &origin,Quat &rot) const;
	protected:
		BaseModelComponent(BaseEntity &ent);
		virtual void OnModelChanged(const std::shared_ptr<Model> &model);
		virtual void Load(DataStream &ds,uint32_t version) override;
		std::shared_ptr<Model> m_model = nullptr;

		std::vector<unsigned int> m_bodyGroups;
		std::unique_ptr<std::string> m_modelName = nullptr;
		std::shared_ptr<util::UInt32Property> m_skin = nullptr;
		pragma::NetEventId m_netEvSetBodyGroup = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvMaxDrawDist = pragma::INVALID_NET_EVENT;
		CallbackHandle m_onModelMaterialsLoaded = {};
		float m_maxDrawDistance = 0.f;

		std::string m_kvModel = "";
		uint32_t m_kvSkin = std::numeric_limits<uint32_t>::max();

		bool m_bMaterialsLoaded = true;
	};
};

#endif
