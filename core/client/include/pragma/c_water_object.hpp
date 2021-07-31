/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_WATER_OBJECT_HPP__
#define __C_WATER_OBJECT_HPP__

#include "pragma/clientdefinitions.h"
#include <memory>
#include <sharedutils/functioncallback.h>
#include <sharedutils/property/util_property.hpp>
#include <pragma/basewaterobject.hpp>

namespace pragma {class CSceneComponent;};
class DLLCLIENT CWaterObject
	: virtual public BaseWaterObject
{
public:
	CWaterObject()=default;
	virtual ~CWaterObject()=default;
	struct DLLCLIENT WaterScene
	{
		~WaterScene();
		util::TWeakSharedHandle<pragma::CSceneComponent> sceneReflection = {};
		std::shared_ptr<prosper::IBuffer> settingsBuffer = nullptr;

		// Fog
		std::shared_ptr<prosper::IBuffer> fogBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> fogDescSetGroup = nullptr;

		std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupTexEffects = nullptr;
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
	prosper::IDescriptorSet *GetEffectDescriptorSet() const;
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
