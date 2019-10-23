#ifndef __C_RENDER_COMPONENT_HPP__
#define __C_RENDER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/c_rendermode.h"
#include <sharedutils/property/util_property.hpp>
#include <pragma/math/sphere.h>
#include <pragma/entities/components/base_render_component.hpp>
#include <mathutil/uvec.h>

namespace prosper {class UniformResizableBuffer; class DescriptorSet;};
namespace pragma
{
	class CModelComponent;
	class DLLCLIENT CRenderComponent final
		: public BaseRenderComponent,
		public CBaseNetComponent
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			RenderBufferDirty = 1u
		};
		static ComponentEventId EVENT_ON_UPDATE_RENDER_DATA;
		static ComponentEventId EVENT_ON_RENDER_BUFFERS_INITIALIZED;
		static ComponentEventId EVENT_ON_RENDER_BOUNDS_CHANGED;
		static ComponentEventId EVENT_SHOULD_DRAW;
		static ComponentEventId EVENT_SHOULD_DRAW_SHADOW;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_MATRICES;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CRenderComponent(BaseEntity &ent);
		std::weak_ptr<prosper::Buffer> GetRenderBuffer() const;
		prosper::DescriptorSet *GetRenderDescriptorSet() const;

		static const std::vector<CRenderComponent*> &GetViewEntities();
		static const std::shared_ptr<prosper::UniformResizableBuffer> &GetInstanceBuffer();
		static void InitializeBuffers();
		static void ClearBuffers();

		virtual void Initialize() override;
		virtual ~CRenderComponent() override;
		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();

		unsigned long long &GetLastRenderFrame();
		void SetLastRenderFrame(unsigned long long &t);

		void GetRenderBounds(Vector3 *min,Vector3 *max) const;
		virtual void SetRenderBounds(Vector3 min,Vector3 max);
		Sphere GetRenderSphereBounds() const;
		void GetRotatedRenderBounds(Vector3 *min,Vector3 *max);

		void SetRenderMode(RenderMode mode);
		RenderMode GetRenderMode() const;
		const util::PEnumProperty<RenderMode> &GetRenderModeProperty() const;

		Mat4 &GetModelMatrix();
		Mat4 &GetTranslationMatrix();
		Mat4 &GetRotationMatrix();
		Mat4 &GetTransformationMatrix();

		virtual void ReceiveData(NetPacket &packet) override;

		virtual void UpdateRenderData(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,bool bForceBufferUpdate=false);

		virtual void Render(RenderMode renderMode);
		virtual void PostRender(RenderMode renderMode);
		virtual bool Render(pragma::ShaderTextured3DBase *shader,Material *mat,CModelSubMesh *mesh); // Return true to override default rendering
		virtual void PreRender();

		virtual bool ShouldDraw(const Vector3 &camOrigin) const;
		virtual bool ShouldDrawShadow(const Vector3 &camOrigin) const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;
		void UpdateRenderBounds();

		util::WeakHandle<CModelComponent> &GetModelComponent() const;

		void SetRenderBufferDirty();
	protected:
		void UpdateRenderBuffer() const;
		virtual void UpdateMatrices();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void ClearRenderObjects();
		static bool RenderCallback(RenderObject *o,CBaseEntity *ent,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase *shader,Material *mat);
		virtual bool RenderCallback(RenderObject *o,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase *shader,Material *mat);
		virtual void UpdateRenderMeshes();

		void ClearRenderBuffers();
		void InitializeRenderBuffers();
		void UpdateBoneBuffer();

		Mat4 m_matModel = umat::identity();
		Mat4 m_matRotation = umat::identity();
		Mat4 m_matTranslation = umat::identity();
		Mat4 m_matTransformation = umat::identity();
		util::PEnumProperty<RenderMode> m_renderMode = nullptr;
		mutable util::WeakHandle<CModelComponent> m_mdlComponent = {}; // Used for quick access to avoid having to do a lookup on the entity's components

		Vector3 m_renderMin = {};
		Vector3 m_renderMax = {};
		Vector3 m_renderMinRot = {};
		Vector3 m_renderMaxRot = {};
		Sphere m_renderSphere = {};

		StateFlags m_stateFlags = StateFlags::RenderBufferDirty;
		unsigned long long m_lastRender = 0ull;
		std::unordered_map<unsigned int,RenderInstance*> m_renderInstances;
		std::unique_ptr<SortedRenderMeshContainer> m_renderMeshContainer = nullptr;
		static std::vector<CRenderComponent*> s_viewEntities;
	private:
		std::shared_ptr<prosper::Buffer> m_renderBuffer = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_renderDescSetGroup = nullptr;
	};

	// Events

	struct DLLCLIENT CEShouldDraw
		: public ComponentEvent
	{
		enum class ShouldDraw : uint8_t
		{
			Undefined = 0u,
			Yes,
			No
		};
		CEShouldDraw(const Vector3 &camOrigin);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		const Vector3 &camOrigin;
		ShouldDraw shouldDraw = ShouldDraw::Undefined;
	};

	struct DLLCLIENT CEOnUpdateRenderMatrices
		: public ComponentEvent
	{
		CEOnUpdateRenderMatrices(Mat4 &translation,Mat4 &rotation,Mat4 &transformation);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		Mat4 &translation;
		Mat4 &rotation;
		Mat4 &transformation;
	};

	struct DLLCLIENT CEOnUpdateRenderData
		: public ComponentEvent
	{
		CEOnUpdateRenderData(const std::shared_ptr<prosper::PrimaryCommandBuffer> &commandBuffer,bool bufferUpdateRequired);
		virtual void PushArguments(lua_State *l) override;
		const bool bufferUpdateRequired;
		std::shared_ptr<prosper::PrimaryCommandBuffer> commandBuffer;
	};

	struct DLLCLIENT CEOnRenderBoundsChanged
		: public ComponentEvent
	{
		CEOnRenderBoundsChanged(const Vector3 &min,const Vector3 &max,const Sphere &sphere);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &min;
		const Vector3 &max;
		const Sphere &sphere;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CRenderComponent::StateFlags)

#endif
