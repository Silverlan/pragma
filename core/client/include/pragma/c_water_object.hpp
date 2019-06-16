#ifndef __C_WATER_OBJECT_HPP__
#define __C_WATER_OBJECT_HPP__

#include "pragma/clientdefinitions.h"
#include <memory>
#include <sharedutils/functioncallback.h>
#include <sharedutils/property/util_property.hpp>
#include <pragma/basewaterobject.hpp>

class Scene;
class DLLCLIENT CWaterObject
	: virtual public BaseWaterObject
{
public:
	CWaterObject()=default;
	virtual ~CWaterObject()=default;
	struct DLLCLIENT WaterScene
	{
		~WaterScene();
		std::shared_ptr<Scene> sceneReflection = nullptr;
		std::shared_ptr<prosper::Buffer> settingsBuffer = nullptr;

		// Fog
		std::shared_ptr<prosper::Buffer> fogBuffer = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> fogDescSetGroup = nullptr;

		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupTexEffects = nullptr;
		CallbackHandle hRenderScene = {};

		CallbackHandle hRender = {};
		CallbackHandle hPostProcessing = {};

		std::shared_ptr<prosper::Texture> texScene = nullptr;
		std::shared_ptr<prosper::Texture> texSceneDepth = nullptr;

		util::PFloatProperty waterScale = nullptr;
		util::PFloatProperty waveStrength = nullptr;
		util::PFloatProperty waveSpeed = nullptr;
		util::PFloatProperty reflectiveIntensity = nullptr;
	};
	const WaterScene &GetWaterScene() const;
	Anvil::DescriptorSet *GetEffectDescriptorSet() const;
	virtual const Vector3 &GetPosition() const=0;
	virtual const Quat &GetOrientation() const=0;
	virtual CMaterial *GetWaterMaterial() const=0;
	void InitializeWaterScene(const Vector3 &refPos,const Vector3 &planeNormal,const Vector3 &waterAabbMin,const Vector3 &waterAabbMax);
	void InitializeWaterScene(const WaterScene &scene);
	bool IsWaterSceneValid() const;
protected:
	std::unique_ptr<WaterScene> m_waterScene = nullptr;
	uint8_t m_reflectionRendered = 0;
	std::pair<Vector3,Vector3> m_waterAabbBounds = {};
};

#endif
