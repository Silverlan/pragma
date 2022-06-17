/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/util/util_baking.hpp"
#include "pragma/util/util_image.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/util/util_thread_pool.hpp"
#include "pragma/entities/components/lightmap_data_cache.hpp"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_image.hpp>
#include <buffers/prosper_buffer.hpp>
#include <mathutil/umath_lighting.hpp>
#include <pragma/entities/environment/lights/c_env_light.h>
#include <sharedutils/util_baking.hpp>
#include <util_image.hpp>
#include <fsys/ifile.hpp>

extern DLLCLIENT CEngine *c_engine;
#pragma optimize("",off)
struct LightSource
{
	enum class Type : uint8_t
	{
		Point = 0,
		Spot,
		Directional
	};
	Vector3 position;
	Vector3 direction;
	umath::Degree innerConeAngle;
	umath::Degree outerConeAngle;
	Candela intensity;
	float radius;
	Vector3 color;
	Type type;
};

static double calc_light_luminance(const LightSource &light,const Vector3 &pos)
{
	Candela intensity;
	switch(light.type)
	{
	case LightSource::Type::Spot:
		intensity = ::pragma::BaseEnvLightSpotComponent::CalcIntensityAtPoint(
			light.position,light.radius,light.intensity,light.direction,
			light.outerConeAngle,light.innerConeAngle,pos
		);
		break;
	case LightSource::Type::Point:
		intensity = ::pragma::BaseEnvLightPointComponent::CalcIntensityAtPoint(
			light.position,light.radius,light.intensity,pos
		);
		break;
	case LightSource::Type::Directional:
		intensity = ::pragma::BaseEnvLightDirectionalComponent::CalcIntensityAtPoint(
			light.intensity,pos
		);
		break;
	}
	return ulighting::srgb_to_luminance(light.color *intensity);
}
static Vector3 calc_light_direction_to_point(const LightSource &light,const Vector3 &pos)
{
	switch(light.type)
	{
	case LightSource::Type::Spot:
	case LightSource::Type::Point:
	{
		auto n = pos -light.position;
		auto l = uvec::length(n);
		if(l > 0.001)
			n /= l;
		else
			n = uvec::UP;
		return n;
	}
	case LightSource::Type::Directional:
		return light.direction;
	}
	return uvec::UP;
}

static std::vector<float> calc_light_weights(const std::vector<LightSource> &lights,const Vector3 &pos)
{
	std::vector<float> weights;
	weights.resize(lights.size());
	float weightSum = 0.f;
	for(uint32_t idx = 0; auto &l : lights)
	{
		auto weight = calc_light_luminance(l,pos);
		weights[idx] = weight;
		weightSum += weight;
		++idx;
	}
	if(weightSum > 0.f)
	{
		for(auto &w : weights)
			w /= weightSum;
	}
	return weights;
}

static Vector3 calc_dominant_light_direction(const std::vector<LightSource> &lights,const std::vector<float> &weights,const Vector3 &pos)
{
	Vector3 n {};
	for(uint32_t i=0;auto &l : lights)
	{
		auto dir = calc_light_direction_to_point(l,pos);
		n += dir *weights[i];
		++i;
	}
	auto l = uvec::length(n);
	if(l > 0.0001)
		n /= l;
	return n;
}

static Vector3 triangle_point_normal(const Vector3 &v0,const Vector3 &v1,const Vector3 &v2, float u, float v)
{
	float t = 1.0f -u -v;
	return (u *v0 +v *v1 +t *v2);
}
static void generate_sh_normals(
	const std::vector<util::baking::BakePixel> &bps,
	const std::vector<LightSource> &lights,
	const std::vector<std::shared_ptr<ModelSubMesh>> meshes,
	Vector3 *outNormals
)
{
	auto process = [&bps,&meshes,&outNormals,&lights](uint32_t start,uint32_t end) {
		for(auto idx=start;idx<end;++idx)
		{
			auto &bp = bps[idx];
			if(bp.objectId < 0 || bp.objectId >= meshes.size())
			{
				outNormals[idx] = uvec::UP;
				continue;
			}
			auto &mesh = meshes[bp.objectId];
			if(bp.primitiveId < 0 || bp.primitiveId >= mesh->GetTriangleCount())
			{
				outNormals[idx] = uvec::UP;
				continue;
			}
			auto triOffset = bp.primitiveId *3;
			std::array<uint32_t,3> indices = {
				*mesh->GetIndex(triOffset),
				*mesh->GetIndex(triOffset +1),
				*mesh->GetIndex(triOffset +2)
			};

			Vector2 uv {bp.uv[0],bp.uv[1]};
			auto dudx = bp.du_dx;
			auto dudy = bp.du_dy;
			auto dvdx = bp.dv_dx;
			auto dvdy = bp.dv_dy;

			auto p = triangle_point_normal(
				mesh->GetVertexPosition(indices[0]),
				mesh->GetVertexPosition(indices[1]),
				mesh->GetVertexPosition(indices[2]),
				uv.x,uv.y
			);

			auto weights = calc_light_weights(lights,p);
			auto dir = calc_dominant_light_direction(lights,weights,p);
			outNormals[idx] = dir;
		}
	};
	pragma::ThreadPool pool {10,"directional_lightmap"};
	pool.BatchProcess(bps.size(),1'024,[process](uint32_t start,uint32_t end) -> pragma::ThreadPool::ResultHandler {
		process(start,end);
		return {};
	});
	pool.WaitForCompletion();
}

static std::shared_ptr<uimg::ImageBuffer> generate_sh_normal_map(
	const std::vector<util::baking::BakePixel> &bps,
	const std::vector<LightSource> &lights,
	const std::vector<std::shared_ptr<ModelSubMesh>> meshes,
	uint32_t width,uint32_t height
)
{
	auto imgBuf = uimg::ImageBuffer::Create(width,height,uimg::Format::RGB32);
	generate_sh_normals(bps,lights,meshes,static_cast<Vector3*>(imgBuf->GetData()));

	auto *n = static_cast<Vector3*>(imgBuf->GetData());
	auto numPixels = imgBuf->GetPixelCount();
	for(auto i=decltype(numPixels){0u};i<numPixels;++i)
	{
		*n = (*n +1.f) /2.f;
		++n;
	}

	std::vector<uint8_t> mask_buffer {};
	mask_buffer.resize(numPixels);
	constexpr auto margin = 16u;
	util::baking::fill_bake_mask(bps, numPixels, reinterpret_cast<char*>(mask_buffer.data()));
	uimg::bake_margin(*imgBuf, mask_buffer, margin);
	return imgBuf;
}

#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/components/c_radius_component.hpp"

/*using PRenderer = std::shared_ptr<Renderer>;
class RenderWorker
	: public util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>>
{
public:
	friend Renderer;
	RenderWorker(Renderer &renderer);
	using util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>>::Cancel;
	virtual void Wait() override;
	virtual std::shared_ptr<uimg::ImageBuffer> GetResult() override;

	using util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>>::SetResultMessage;
	using util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>>::AddThread;
	using util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>>::UpdateProgress;
private:
	virtual void DoCancel(const std::string &resultMsg,std::optional<int32_t> resultCode) override;
	PRenderer m_renderer = nullptr;
	template<typename TJob,typename... TARGS>
		friend util::ParallelJob<typename TJob::RESULT_TYPE> util::create_parallel_job(TARGS&& ...args);
};*/

util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> util::baking::bake_directional_lightmap_atlas(
	const std::vector<::pragma::CLightComponent*> &lights,
	const std::vector<ModelSubMesh*> meshes,
	const std::vector<std::string> &entityUuids,
	uint32_t width,uint32_t height,
	::pragma::LightmapDataCache *optLightmapDataCache
)
{
	class LightmapBakeJob
		: public util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>>
	{
	public:
		LightmapBakeJob(
			uint32_t width,uint32_t height,std::vector<LightSource> &&lights,std::vector<std::shared_ptr<ModelSubMesh>> &&meshes,
			std::vector<std::string> &&meshEntityUuids,const std::shared_ptr<::pragma::LightmapDataCache> &lmdCache
		)
			: m_width{width},m_height{height},m_lightData{std::move(lights)},m_meshes{std::move(meshes)},
			m_meshEntityUuids{std::move(meshEntityUuids)},m_lightmapDataCache{lmdCache}
		{
			AddThread([this]() {
				Run();
				//m_imgBuffer = uimg::load_image(f,pixelFormat);
				
				//SetStatus(util::JobStatus::Failed,"Unable to open image!");
				//UpdateProgress(1.f);
				
				UpdateProgress(1.f);
			});
		}

		virtual std::shared_ptr<uimg::ImageBuffer> GetResult() override {return m_imgBuffer;}
	private:
		void Run()
		{
			std::vector<util::baking::BakePixel> bps;
			bps.resize(m_width *m_height,util::baking::BakePixel{});

			util::baking::BakeDataView bd;
			bd.bakePixels = bps.data();

			auto &zspan = bd.span;
			zspan.rectx = m_width;
			zspan.recty = m_height;

			zspan.span1.resize(zspan.recty);
			zspan.span2.resize(zspan.recty);

			for(uint32_t idx=0;auto &subMesh : m_meshes)
			{
				if(subMesh->GetGeometryType() != ModelSubMesh::GeometryType::Triangles)
					continue;
				const std::vector<Vector2> *uvs = nullptr;
				if(m_lightmapDataCache)
					uvs = m_lightmapDataCache->FindLightmapUvs(util::uuid_string_to_bytes(m_meshEntityUuids[idx]),subMesh->GetUuid());
				else
					uvs = subMesh->GetUVSet("lightmap");
				if(!uvs)
					continue;
				util::baking::MeshInterface meshInterface;
				meshInterface.getUv = [uvs](uint32_t idx) -> util::baking::Uv {
					auto &uv = (*uvs)[idx];
					return {uv.x,uv.y};
				};
				meshInterface.getTriangle = [subMesh](uint32_t idx) -> util::baking::Triangle {
					idx *= 3;
					return {
						*subMesh->GetIndex(idx),
						*subMesh->GetIndex(idx +1),
						*subMesh->GetIndex(idx +2)
					};
				};
				util::baking::prepare_bake_pixel_data(
					bd,idx,meshInterface,subMesh->GetTriangleCount(),m_width,m_height
				);
				++idx;
			}
			m_imgBuffer = generate_sh_normal_map(bps,m_lightData,m_meshes,m_width,m_height);
		}
		uint32_t m_width;
		uint32_t m_height;
		std::vector<LightSource> m_lightData;
		std::vector<std::shared_ptr<ModelSubMesh>> m_meshes;
		std::vector<std::string> m_meshEntityUuids;
		std::shared_ptr<::pragma::LightmapDataCache> m_lightmapDataCache;
		std::shared_ptr<uimg::ImageBuffer> m_imgBuffer;
	};

	std::vector<LightSource> lightData;
	lightData.reserve(lights.size());
	for(auto *l : lights)
	{
		lightData.push_back({});
		auto &ld = lightData.back();
		ld.position = l->GetEntity().GetPosition();
		ld.direction = l->GetEntity().GetForward();
		auto col = l->GetEntity().GetColor();
		ld.color = col.has_value() ? col->ToVector3() : Color::White.ToVector3();
		ld.intensity = l->GetLightIntensityCandela();

		auto radiusC = l->GetEntity().GetComponent<::pragma::CRadiusComponent>();
		ld.radius = radiusC.valid() ? radiusC->GetRadius() : 0.f;

		auto spotC = l->GetEntity().GetComponent<::pragma::CLightSpotComponent>();
		if(spotC.valid())
		{
			ld.type = LightSource::Type::Spot;
			ld.innerConeAngle = spotC->GetInnerConeAngle();
			ld.outerConeAngle = spotC->GetOuterConeAngle();
		}
		else if(l->GetEntity().HasComponent<::pragma::CLightPointComponent>())
			ld.type = LightSource::Type::Point;
		else if(l->GetEntity().HasComponent<::pragma::CLightDirectionalComponent>())
			ld.type = LightSource::Type::Directional;
		else
			lightData.erase(lightData.end() -1); // Invalid light source or unknown type
	}

	auto pLmdc = optLightmapDataCache ? optLightmapDataCache->shared_from_this() : nullptr;
	std::vector<std::shared_ptr<ModelSubMesh>> meshCopies;
	meshCopies.reserve(meshes.size());
	for(auto *mesh : meshes)
	{
		meshCopies.push_back(mesh->Copy(true));
		meshCopies.back()->SetUuid(mesh->GetUuid());
	}
	auto eu = entityUuids;
	return util::create_parallel_job<LightmapBakeJob>(
		width,height,std::move(lightData),std::move(meshCopies),
		std::move(eu),pLmdc
	);
}
#pragma optimize("",on)
