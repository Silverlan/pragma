// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :util.baking;
import :engine;
import :entities.components;
import :util.image;

// #define DEBUG_BAKE_LIGHT_WEIGHTS

static double calc_light_luminance(const pragma::util::baking::LightSource &light, const Vector3 &pos, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2)
{
	Vector3 planeNormal;
	float planeDist;
	uvec::calc_plane(p0, p1, p2, planeNormal, planeDist);
	auto side = pragma::math::geometry::get_side_of_point_to_plane(planeNormal, planeDist, light.position);
	if(side == pragma::math::geometry::PlaneSide::Back)
		return 0.0;
	Candela intensity;
	auto lightIntensity = light.intensity;
#ifdef DEBUG_BAKE_LIGHT_WEIGHTS
	lightIntensity = 1.f;
#endif
	switch(light.type) {
	case pragma::util::baking::LightSource::Type::Spot:
		intensity = pragma::BaseEnvLightSpotComponent::CalcIntensityAtPoint(light.position, lightIntensity, light.direction, light.outerConeAngle, light.innerConeAngle, pos);
		break;
	case pragma::util::baking::LightSource::Type::Point:
		intensity = pragma::BaseEnvLightPointComponent::CalcIntensityAtPoint(light.position, lightIntensity, pos);
		break;
	case pragma::util::baking::LightSource::Type::Directional:
		// TODO: Use as default direction
		intensity = pragma::BaseEnvLightDirectionalComponent::CalcIntensityAtPoint(lightIntensity, pos) * 0.025f;
		break;
	default:
		intensity = 0.f;
		break;
	}
#ifdef DEBUG_BAKE_LIGHT_WEIGHTS
	return intensity;
#else
	return ulighting::srgb_to_luminance(light.color * intensity);
#endif
}
static Vector3 calc_light_direction_to_point(const pragma::util::baking::LightSource &light, const Vector3 &pos)
{
	switch(light.type) {
	case pragma::util::baking::LightSource::Type::Spot:
	case pragma::util::baking::LightSource::Type::Point:
		{
			auto n = pos - light.position;
			auto l = uvec::length(n);
			if(l > 0.001)
				n /= l;
			else
				n = uvec::PRM_UP;
			return n;
		}
	case pragma::util::baking::LightSource::Type::Directional:
		return light.direction;
	}
	return uvec::PRM_UP;
}

static std::vector<float> calc_light_weights(const std::vector<pragma::util::baking::LightSource> &lights, const Vector3 &pos, const Vector3 &p0, const Vector3 &p1, const Vector3 &p2)
{
	std::vector<float> weights;
	weights.resize(lights.size());
	for(uint32_t idx = 0; auto &l : lights) {
		auto weight = calc_light_luminance(l, pos, p0, p1, p2);
		weights[idx] = weight;
		++idx;
	}
	return weights;
}

static Vector3 calc_dominant_light_direction(const std::vector<pragma::util::baking::LightSource> &lights, const std::vector<float> &weights, const Vector3 &pos)
{
	Vector3 n {};
	auto totalWeight = 0.f;
	for(uint32_t i = 0; auto &l : lights) {
#ifdef DEBUG_BAKE_LIGHT_WEIGHTS
		n += Vector3 {weights[i], weights[i], weights[i]};
#else
		auto dir = calc_light_direction_to_point(l, pos);
		dir *= weights[i];
		totalWeight += weights[i];
		n += dir;
#endif
		++i;
	}
	uvec::normalize(n, uvec::PRM_UP);
	return n;
}

static Vector3 triangle_point(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, float u, float v)
{
	float t = 1.0f - u - v;
	return (u * v0 + v * v1 + t * v2);
}
static Vector3 triangle_normal(const Vector3 &n0, const Vector3 &n1, const Vector3 &n2, float u, float v)
{
	float t = 1.0f - u - v;
	auto n = (u * n0 + v * n1 + t * n2);
	uvec::normalize(&n);
	return n;
}
static void generate_sh_normals(const std::vector<pragma::util::baking::BakePixel> &bps, const std::vector<pragma::util::baking::LightSource> &lights, const std::vector<std::shared_ptr<pragma::geometry::ModelSubMesh>> meshes, Vector3 *outNormals)
{
	auto process = [&bps, &meshes, &outNormals, &lights](uint32_t start, uint32_t end) {
		for(auto idx = start; idx < end; ++idx) {
			auto &bp = bps[idx];
			if(bp.objectId < 0 || bp.objectId >= meshes.size()) {
				outNormals[idx] = uvec::PRM_UP;
				continue;
			}
			auto &mesh = meshes[bp.objectId];
			if(bp.primitiveId < 0 || bp.primitiveId >= mesh->GetTriangleCount()) {
				outNormals[idx] = uvec::PRM_UP;
				continue;
			}
			auto triOffset = bp.primitiveId * 3;
			std::array<uint32_t, 3> indices = {*mesh->GetIndex(triOffset), *mesh->GetIndex(triOffset + 1), *mesh->GetIndex(triOffset + 2)};

			Vector2 uv {bp.uv[0], bp.uv[1]};
			auto dudx = bp.du_dx;
			auto dudy = bp.du_dy;
			auto dvdx = bp.dv_dx;
			auto dvdy = bp.dv_dy;

			auto p0 = mesh->GetVertexPosition(indices[0]);
			auto p1 = mesh->GetVertexPosition(indices[1]);
			auto p2 = mesh->GetVertexPosition(indices[2]);
			auto p = triangle_point(p0, p1, p2, uv.x, uv.y);
			// auto n = triangle_normal(mesh->GetVertexNormal(indices[0]), mesh->GetVertexNormal(indices[1]), mesh->GetVertexNormal(indices[2]), uv.x, uv.y);
			auto weights = calc_light_weights(lights, p, p0, p1, p2);
			auto dir = calc_dominant_light_direction(lights, weights, p);
			outNormals[idx] = dir;
		}
	};
	pragma::ThreadPool pool {10, "directional_lightmap"};
	pool.BatchProcess(bps.size(), 1'024, [process](uint32_t start, uint32_t end) -> pragma::ThreadPool::ResultHandler {
		process(start, end);
		return {};
	});
	pool.WaitForCompletion();
}

static std::shared_ptr<pragma::image::ImageBuffer> generate_sh_normal_map(const std::vector<pragma::util::baking::BakePixel> &bps, const std::vector<pragma::util::baking::LightSource> &lights, const std::vector<std::shared_ptr<pragma::geometry::ModelSubMesh>> meshes, uint32_t width,
  uint32_t height)
{
	auto imgBuf = pragma::image::ImageBuffer::Create(width, height, pragma::image::Format::RGB32);
	generate_sh_normals(bps, lights, meshes, static_cast<Vector3 *>(imgBuf->GetData()));
	/*auto f = fs::open_file("test_nm.png",fs::FileMode::Write | fs::FileMode::Binary);
	if(f)
	{
		auto tmp = imgBuf->Copy();
		fs::File fp {f};
		image::save_image(fp,*tmp,image::ImageFormat::PNG);
	}
	f = nullptr;*/

	auto *n = static_cast<Vector3 *>(imgBuf->GetData());
	auto numPixels = imgBuf->GetPixelCount();
	for(auto i = decltype(numPixels) {0u}; i < numPixels; ++i) {
		*n = (*n + 1.f) / 2.f;
		++n;
	}

	std::vector<uint8_t> mask_buffer {};
	mask_buffer.resize(numPixels);
	constexpr auto margin = 16u;
	pragma::util::baking::fill_bake_mask(bps, numPixels, reinterpret_cast<char *>(mask_buffer.data()));
	pragma::image::bake_margin(*imgBuf, mask_buffer, margin);
	return imgBuf;
}

pragma::util::ParallelJob<std::shared_ptr<pragma::image::ImageBuffer>> pragma::util::baking::bake_directional_lightmap_atlas(const std::vector<CLightComponent *> &lights, const std::vector<geometry::ModelSubMesh *> meshes, const std::vector<ecs::BaseEntity *> &entities, uint32_t width,
  uint32_t height, rendering::LightmapDataCache *optLightmapDataCache)
{
	class LightmapBakeJob : public ParallelWorker<std::shared_ptr<image::ImageBuffer>> {
	  public:
		LightmapBakeJob(uint32_t width, uint32_t height, std::vector<LightSource> &&lights, std::vector<std::shared_ptr<geometry::ModelSubMesh>> &&meshes, std::vector<std::string> &&meshEntityUuids, std::vector<math::ScaledTransform> &&meshEntityPoses,
		  const std::shared_ptr<rendering::LightmapDataCache> &lmdCache)
		    : m_width {width}, m_height {height}, m_lightData {std::move(lights)}, m_meshes {std::move(meshes)}, m_meshEntityUuids {std::move(meshEntityUuids)}, m_lightmapDataCache {lmdCache}, m_meshEntityPoses {std::move(meshEntityPoses)}
		{
			AddThread([this]() {
				Run();
				//m_imgBuffer = image::load_image(f,pixelFormat);

				//SetStatus(pragma::util::JobStatus::Failed,"Unable to open image!");
				//UpdateProgress(1.f);

				UpdateProgress(1.f);
			});
		}

		virtual std::shared_ptr<image::ImageBuffer> GetResult() override { return m_imgBuffer; }
	  private:
		void Run()
		{
			// Transform meshes to world space
			auto n = m_meshes.size();
			for(auto i = decltype(n) {0}; i < n; ++i) {
				auto &mesh = m_meshes.at(i);
				auto &pose = m_meshEntityPoses.at(i);
				mesh->Transform(pose);
			}

			std::vector<BakePixel> bps;
			bps.resize(m_width * m_height, BakePixel {});

			BakeDataView bd;
			bd.bakePixels = bps.data();

			auto &zspan = bd.span;
			zspan.rectx = m_width;
			zspan.recty = m_height;

			zspan.span1.resize(zspan.recty);
			zspan.span2.resize(zspan.recty);

			for(uint32_t idx = 0; auto &subMesh : m_meshes) {
				if(subMesh->GetGeometryType() != geometry::ModelSubMesh::GeometryType::Triangles) {
					++idx;
					continue;
				}
				const std::vector<Vector2> *uvs = nullptr;
				if(m_lightmapDataCache)
					uvs = m_lightmapDataCache->FindLightmapUvs(uuid_string_to_bytes(m_meshEntityUuids[idx]), subMesh->GetUuid());
				else
					uvs = subMesh->GetUVSet("lightmap");
				if(!uvs) {
					++idx;
					continue;
				}
				MeshInterface meshInterface;
				meshInterface.getUv = [uvs](uint32_t idx) -> Uv {
					auto &uv = (*uvs)[idx];
					return {uv.x, uv.y};
				};
				meshInterface.getTriangle = [subMesh](uint32_t idx) -> Triangle {
					idx *= 3;
					return {*subMesh->GetIndex(idx), *subMesh->GetIndex(idx + 1), *subMesh->GetIndex(idx + 2)};
				};
				prepare_bake_pixel_data(bd, idx, meshInterface, subMesh->GetTriangleCount(), m_width, m_height);
				++idx;
			}
			m_imgBuffer = generate_sh_normal_map(bps, m_lightData, m_meshes, m_width, m_height);
		}
		uint32_t m_width;
		uint32_t m_height;
		std::vector<LightSource> m_lightData;
		std::vector<std::shared_ptr<geometry::ModelSubMesh>> m_meshes;
		std::vector<std::string> m_meshEntityUuids;
		std::vector<math::ScaledTransform> m_meshEntityPoses;
		std::shared_ptr<rendering::LightmapDataCache> m_lightmapDataCache;
		std::shared_ptr<image::ImageBuffer> m_imgBuffer;
	};

	std::vector<LightSource> lightData;
	lightData.reserve(lights.size());
	for(auto *l : lights) {
		lightData.push_back({});
		auto &ld = lightData.back();
		ld.position = l->GetEntity().GetPosition();
		ld.direction = l->GetEntity().GetForward();
		auto col = l->GetEntity().GetColor();
		ld.color = col.has_value() ? col->ToVector3() : colors::White.ToVector3();
		ld.intensity = l->GetLightIntensityCandela();

		auto spotC = l->GetEntity().GetComponent<CLightSpotComponent>();
		if(spotC.valid()) {
			ld.type = LightSource::Type::Spot;
			ld.innerConeAngle = spotC->GetInnerConeAngle();
			ld.outerConeAngle = spotC->GetOuterConeAngle();
		}
		else if(l->GetEntity().HasComponent<CLightPointComponent>())
			ld.type = LightSource::Type::Point;
		else if(l->GetEntity().HasComponent<CLightDirectionalComponent>())
			ld.type = LightSource::Type::Directional;
		else
			lightData.erase(lightData.end() - 1); // Invalid light source or unknown type
	}

	auto pLmdc = optLightmapDataCache ? optLightmapDataCache->shared_from_this() : nullptr;
	std::vector<std::shared_ptr<geometry::ModelSubMesh>> meshCopies;
	meshCopies.reserve(meshes.size());
	for(auto *mesh : meshes) {
		meshCopies.push_back(mesh->Copy(true));
		meshCopies.back()->SetUuid(mesh->GetUuid());
	}
	std::vector<math::ScaledTransform> entityPoses;
	std::vector<std::string> entityUuids;
	entityPoses.reserve(entities.size());
	entityUuids.reserve(entities.size());
	for(auto *ent : entities) {
		entityUuids.push_back(uuid_to_string(ent->GetUuid()));
		entityPoses.push_back(ent->GetPose());
	}
	return pragma::util::create_parallel_job<LightmapBakeJob>(width, height, std::move(lightData), std::move(meshCopies), std::move(entityUuids), std::move(entityPoses), pLmdc);
}
