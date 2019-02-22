#ifndef __C_LIGHT_H__
#define __C_LIGHT_H__

// TODO: Remove this file
#if 0
#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include <mathutil/color.h>
#include <vector>
#include <memory>
#include "pragma/util/mvpbase.h"
#include "pragma/rendering/lighting/c_light_handle.hpp"
#include "pragma/rendering/lighting/shadows/c_shadow_type.hpp"
#include <sharedutils/property/util_property_color.hpp>

enum class LightType : uint32_t
{
	Invalid = 0,
	Directional = 1,
	Point = 2,
	Spot = 3
};

namespace prosper
{
	class Buffer;
	class UniformResizableBuffer;
};

class ShadowMap;
class CBaseEntity;
class CModelMesh;
class CModelSubMesh;
namespace pragma {class CParticleSystemComponent;};
class DLLCLIENT CLightBase
{
public:
	static const prosper::UniformResizableBuffer &GetGlobalRenderBuffer();
	static const prosper::UniformResizableBuffer &GetGlobalShadowBuffer();
	static CLightBase *GetLightByBufferIndex(uint32_t idx);
	static CLightBase *GetLightByShadowBufferIndex(uint32_t idx);
	static uint32_t GetMaxLightCount();
	static uint32_t GetMaxShadowCount();
	static uint32_t GetLightCount();
	static void InitializeBuffers();
	static void ClearBuffers();
	const std::shared_ptr<prosper::Buffer> &GetRenderBuffer() const;
	const std::shared_ptr<prosper::Buffer> &GetShadowBuffer() const;

	enum class RenderPass : uint8_t
	{
		Static = 0,
		Dynamic = 1
	};
	enum class Flags : uint32_t
	{
		None = 0,
		TurnedOn = 1,
		TypeSpot = TurnedOn<<1,
		TypePoint = TypeSpot<<1,
		TypeDirectional = TypePoint<<1
	};
	struct MeshInfo
	{
		MeshInfo(CModelSubMesh *mesh,uint32_t renderFlags=0);
		CModelSubMesh *mesh;
		uint32_t renderFlags;
	};
	struct EntityInfo
	{
		EntityInfo(CLightBase *light,BaseEntity *ent,uint32_t renderFlags=0);
		~EntityInfo();
		EntityInfo(const EntityInfo&)=delete;
		EntityInfo &operator=(const EntityInfo&)=delete;
		EntityHandle hEntity;
		CallbackHandle hCbMaterialsLoaded;
		uint32_t renderFlags;
		double tLastMoved;
		std::vector<MeshInfo> meshes;
	};
	struct ParticleInfo
	{
		ParticleInfo(pragma::CParticleSystemComponent &pt);
		ParticleInfo(const ParticleInfo&)=delete;
		ParticleInfo &operator=(const ParticleInfo&)=delete;
		util::WeakHandle<pragma::CParticleSystemComponent> hParticle;
	};
	struct ShadowInfoSet
	{
		std::vector<std::shared_ptr<EntityInfo>>::iterator FindEntity(CBaseEntity *ent);
		std::vector<std::shared_ptr<EntityInfo>> meshInfo;
	};
#pragma pack(push,1)
	struct BufferData
	{
		Vector4 position {}; // position.w = distance
		Vector4 color {};
		Vector4 direction {}; // direction.w is unused
		uint32_t shadowIndex = 0u;

		float cutoffOuter = 0.f;
		float cutoffInner = 0.f;
		float attenuation = 0.f;
		Flags flags = Flags::None;
		uint32_t shadowMapIndex = 0u;

		 // Alignment to vec4
		float dummy0 = 0.f;
		float dummy1 = 0.f;
	};
	struct ShadowBufferData
	{
		Mat4 depthVP = umat::identity();
		Mat4 view = umat::identity();
		Mat4 projection = umat::identity();
	};
#pragma pack(pop)
	/*class BufferUpdateInfo
	{
	private:
		std::function<void(const Vulkan::Context &context,const Vulkan::Buffer&)> m_fUpdate;
		Vulkan::SwapBufferObject *m_swapBuffer;
		uint32_t m_swapchainUpdateFlags; // 1 Bit per swapchain image
	public:
		BufferUpdateInfo(const Vulkan::SwapBufferObject *swapBuffer,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f);
		bool ExecSwapchainUpdate();
		bool IsComplete() const;
	};*/ // prosper TODO

	virtual void SetDirection(const Vector3 &dir);
	virtual const Vector3 &GetDirection() const;
	virtual void SetDistance(float dist);
	virtual float GetDistance() const;
	virtual void SetOuterCutoffAngle(float ang);
	virtual float GetOuterCutoffAngle() const;
	virtual void SetInnerCutoffAngle(float ang);
	virtual float GetInnerCutoffAngle() const;
protected:
	static std::shared_ptr<prosper::UniformResizableBuffer> s_instanceBuffer;
	static std::shared_ptr<prosper::UniformResizableBuffer> s_shadowBuffer;
	static std::vector<CLightBase*> s_bufferLights;
	static std::vector<CLightBase*> s_shadowBufferLights;
	static std::size_t s_lightCount;
	static std::size_t s_maxLightCount;
	static std::size_t s_maxShadowCount;
	void InitializeRenderBuffer();
	void InitializeShadowBuffer();
	void DestroyRenderBuffer();
	void DestroyShadowBuffer();

	BufferData m_bufferData {};
	std::unique_ptr<ShadowBufferData> m_shadowBufferData = nullptr;
	std::shared_ptr<prosper::Buffer> m_renderBuffer = nullptr;
	std::shared_ptr<prosper::Buffer> m_shadowBuffer = nullptr;
public:
	friend CLightObjectHandle;
	friend EntityInfo;
protected:
	enum class DataSlot : uint32_t
	{
		Flags,
		Position,
		Color,
		Distance,
		Direction,
		CutoffOuter,
		CutoffInner,
		ShadowIndex,
		ShadowMapIndex,
		// Light Buffer End

		ShadowStart,
		DepthMVP = ShadowStart,
		ViewMatrix,
		ProjectionMatrix
		// Shadow Buffer End
	};

	CLightBase(LightType type,const Vector3 &pos,const Color &col,ShadowType shadowType=ShadowType::Full);
	CLightBase(const CLightBase&)=delete;
	CLightBase &operator=(const CLightBase&)=delete;
	//std::vector<std::unique_ptr<BufferUpdateInfo>> m_bufferUpdateInfo; // Index corresponds to LIGHT_BUFFER_OFFSET_* ID // prosper TODO
	//virtual void ScheduleBufferUpdate(DataSlot offsetId,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f); // prosper TODO
	void ExecSwapchainUpdate();
	uint64_t m_lastThink = std::numeric_limits<uint64_t>::max();
	uint64_t m_lastShadowRendered = std::numeric_limits<uint64_t>::max();
	std::array<ShadowInfoSet,2> m_shadowInfoSets;
	std::vector<std::shared_ptr<ParticleInfo>> m_particleInfo;
	uint64_t m_nextDynamicUpdate;
	bool m_bUseDualTextureSet;

	// Entity Info
	ShadowType m_shadowTypes;
	CallbackHandle m_onEntitySpawned;
	CallbackHandle m_onModelChanged;
	bool m_bFullUpdateRequired;
	std::function<bool(const CBaseEntity&)> m_cbShouldRender;
	void ClearCache();
	virtual void UpdateEntity(CBaseEntity *ent);
	void UpdateAllEntities();
	virtual void UpdateParticleSystem(pragma::CParticleSystemComponent &pt);
	void UpdateAllParticleSystems();
	virtual void ScheduleRender()=0;
	void UpdateMeshes(CBaseEntity *ent,std::vector<MeshInfo> &meshes);
	//
	bool m_bStaticUpdateRequired;
	bool m_bTurnedOn = false;
	double m_tTurnedOff = 0.0;
	util::ColorProperty m_color {};
	util::FloatProperty m_innerCutoffAngle {};
	util::FloatProperty m_outerCutoffAngle {};
	std::unique_ptr<ShadowMap> m_shadow;
	CLightObjectHandle m_handle;
	bool ShouldCastShadows() const;
	void UpdateShadowTypes();
	void InitializeShadowMap(ShadowMap &sm);
	virtual void InitializeShadowMap();
	virtual void UpdateViewMatrices();
public:
	virtual ~CLightBase();
	virtual Mat4 &GetTransformationMatrix(unsigned int j)=0;
	virtual void TurnOn();
	virtual void TurnOff();
	void SetShadowType(ShadowType type);
	ShadowType GetShadowType() const;
	void Toggle();
	bool IsTurnedOn();
	virtual void Initialize();
	virtual bool ShouldUpdateRenderPass(RenderPass rp) const;
	void PostRenderShadow(); // Called once the light's shadows have been rendered
	void SetStaticResolved(bool b=true);
	void SetShouldPassCallback(const std::function<bool(const CBaseEntity&)> &callback);
	virtual bool ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags);
	virtual bool ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags);
	virtual bool ShouldPass(const Model &mdl,const CModelSubMesh &mesh);
	const Vector3 &GetPosition() const;
	Color &GetColor();
	virtual void SetPosition(const Vector3 &pos);
	void SetColor(const Color &color);
	CLightObjectHandle GetHandle();
	CLightObjectHandle *CreateHandle();
	void Remove();
	ShadowMap *GetShadowMap();
	virtual LightType GetType() const;
	virtual void Think();
	void UpdateCulledMeshes();
	std::vector<std::shared_ptr<EntityInfo>> &GetCulledMeshes(RenderPass rp);
	std::vector<std::shared_ptr<ParticleInfo>> &GetCulledParticleSystems();
	virtual bool ShouldRender();

	// Used for debug purposes only
	void UpdateBuffers();

	uint64_t GetLastTimeShadowRendered() const;
	void SetLastTimeShadowRendered(uint64_t t);

	void SetShadowMapIndex(uint32_t idx);
};
REGISTER_BASIC_BITWISE_OPERATORS(CLightBase::Flags);

class DLLCLIENT CLight
	: public CLightBase,public MVPBias<1>
{
protected:
	virtual void UpdateTransformationMatrix(unsigned int j) override;
public:
	CLight(LightType type,const Vector3 &pos,const Color &col,ShadowType shadowType=ShadowType::Full);
	virtual Mat4 &GetTransformationMatrix(unsigned int j) override;
	virtual void Initialize() override;
};

#endif
#endif
