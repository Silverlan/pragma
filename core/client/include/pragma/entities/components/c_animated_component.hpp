/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_ANIMATED_COMPONENT_HPP__
#define __C_ANIMATED_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_animated_component.hpp>

struct Eyeball;
namespace pragma
{
	void initialize_articulated_buffers();
	void clear_articulated_buffers();
	const std::shared_ptr<prosper::IUniformResizableBuffer> &get_instance_bone_buffer();

	class DLLCLIENT CAnimatedComponent final
		: public BaseAnimatedComponent,
		public CBaseNetComponent
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			BoneBufferDirty = 1u
		};

		// static ComponentEventId EVENT_ON_SKELETON_UPDATED;
		// static ComponentEventId EVENT_ON_BONE_MATRICES_UPDATED;
		static ComponentEventId EVENT_ON_BONE_BUFFER_INITIALIZED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CAnimatedComponent(BaseEntity &ent) : BaseAnimatedComponent(ent) {}

		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		std::weak_ptr<prosper::IBuffer> GetBoneBuffer() const;
		const std::vector<Mat4> &GetBoneMatrices() const;
		std::vector<Mat4> &GetBoneMatrices();
		void UpdateBoneMatricesMT();
		void UpdateBoneBuffer(prosper::IPrimaryCommandBuffer &commandBuffer);
		void InitializeBoneBuffer();
		std::optional<Mat4> GetVertexTransformMatrix(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 *optOutNormalOffset=nullptr,float *optOutDelta=nullptr) const;
		virtual std::optional<Mat4> GetVertexTransformMatrix(const ModelSubMesh &subMesh,uint32_t vertexId) const override;

		uint32_t OnSkeletonUpdated();
		bool MaintainAnimations(double dt) override;

		void SetBoneBufferDirty();
	protected:
		virtual void OnModelChanged(const std::shared_ptr<Model> &mdl) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	private:
		std::shared_ptr<prosper::IBuffer> m_boneBuffer = nullptr;
		std::vector<Mat4> m_boneMatrices;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_boneDescSetGroup = nullptr;
		StateFlags m_stateFlags = StateFlags::BoneBufferDirty;
	};

	// Events

	struct DLLCLIENT CEOnSkeletonUpdated
		: public ComponentEvent
	{
		CEOnSkeletonUpdated(uint32_t &physRootBoneId);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		uint32_t &physRootBoneId;
	};

	struct DLLCLIENT CEOnBoneBufferInitialized
		: public ComponentEvent
	{
		CEOnBoneBufferInitialized(const std::shared_ptr<prosper::IBuffer> &buffer);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<prosper::IBuffer> buffer;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CAnimatedComponent::StateFlags)

#endif
