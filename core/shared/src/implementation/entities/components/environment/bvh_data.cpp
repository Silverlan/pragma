// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <bvh/v2/default_builder.h>
#include <bvh/v2/reinsertion_optimizer.h>
#include <bvh/v2/stack.h>

module pragma.shared;

import :entities.components.bvh_data;

static_assert(sizeof(Vector3) == sizeof(pragma::bvh::Vec));

const pragma::bvh::Vec &pragma::bvh::to_bvh_vector(const Vector3 &v) { return reinterpret_cast<const Vec &>(v); }
const Vector3 &pragma::bvh::from_bvh_vector(const Vec &v) { return reinterpret_cast<const Vector3 &>(v); }
bool pragma::bvh::is_mesh_bvh_compatible(const geometry::ModelSubMesh &mesh) { return mesh.GetGeometryType() == geometry::ModelSubMesh::GeometryType::Triangles; }

pragma::bvh::Primitive pragma::bvh::create_triangle(const Vector3 &a, const Vector3 &b, const Vector3 &c) { return Primitive {to_bvh_vector(a), to_bvh_vector(b), to_bvh_vector(c)}; }

std::vector<pragma::bvh::MeshRange> &pragma::bvh::get_bvh_mesh_ranges(MeshBvhTree &bvhData) { return bvhData.meshRanges; }

static void get_bvh_bounds(const pragma::bvh::BBox &bb, Vector3 &outMin, Vector3 &outMax)
{
	constexpr auto epsilon = 0.001f;
	outMin = Vector3 {pragma::math::min(bb.min.values[0], bb.max.values[0]) - epsilon, pragma::math::min(bb.min.values[1], bb.max.values[1]) - epsilon, pragma::math::min(bb.min.values[2], bb.max.values[2]) - epsilon};
	outMax = Vector3 {pragma::math::max(bb.min.values[0], bb.max.values[0]) + epsilon, pragma::math::max(bb.min.values[1], bb.max.values[1]) + epsilon, pragma::math::max(bb.min.values[2], bb.max.values[2]) + epsilon};
}
std::tuple<bool, bool, bool> pragma::bvh::test_node_aabb_intersection(const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const Node &left, const Node &right)
{
	bool hit_left, hit_right;
	{
		Vector3 v0_l, v1_l;
		get_bvh_bounds(left.get_bbox(), v0_l, v1_l);
		hit_left = testAabb(v0_l, v1_l);
	}

	{
		Vector3 v0_r, v1_r;
		get_bvh_bounds(right.get_bbox(), v0_r, v1_r);
		hit_right = testAabb(v0_r, v1_r);
	}
	return std::make_tuple(hit_left, hit_right, false);
}
bool pragma::bvh::test_bvh_intersection(const MeshBvhTree &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const Primitive &)> &testTri, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	auto &bvh = bvhData.bvh;
	auto &node = bvh.nodes[nodeIdx];
	constexpr size_t stack_size = 64;
	::bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
	auto hasAnyHit = false;

	auto isPrimitiveIntersectionInfo = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(PrimitiveIntersectionInfo);
	auto isMeshIntersectionInfo = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(MeshIntersectionInfo);

	std::unique_ptr<IntersectionCache> intersectionCache {};
	if(isPrimitiveIntersectionInfo || isMeshIntersectionInfo)
		intersectionCache = std::make_unique<IntersectionCache>();

	auto traverse = [&]<bool ReturnOnFirstHit>() {
		bvh.traverse_top_down<ReturnOnFirstHit>(
		  bvh.get_root().index, stack,
		  [outIntersectionInfo, isPrimitiveIntersectionInfo, isMeshIntersectionInfo, &intersectionCache, &bvh, &bvhData, &testTri, &hasAnyHit](size_t begin, size_t end) {
			  auto hasHit = false;
			  for(auto i = begin; i < end; ++i) {
				  auto primIdx = should_permute ? bvh.prim_ids[i] : i;

				  if(intersectionCache) {
					  auto skip = false;
					  auto idx = primIdx * 3;
					  for(auto &meshRange : intersectionCache->meshRanges) {
						  if(idx >= meshRange.start && idx < meshRange.end) {
							  skip = true;
							  break;
						  }
					  }
					  if(skip)
						  continue;
				  }

				  auto &prim = bvhData.primitives[primIdx];
				  auto &p1 = prim.p1;
				  auto &p2 = prim.p2;
				  auto res = testTri(prim);
				  if(res) {
					  hasHit = true;
					  hasAnyHit = true;
					  if(!intersectionCache)
						  return true;
					  auto addPrim = true;
					  if(intersectionCache) {
						  auto *meshRange = bvhData.FindMeshRange(primIdx);
						  if(meshRange) {
							  intersectionCache->meshRanges.push_back({*meshRange});

							  auto hash = pragma::util::hash_combine<uint64_t>(pragma::util::hash_combine<uint64_t>(0, reinterpret_cast<uint64_t>(meshRange->mesh.get())), reinterpret_cast<uint64_t>(meshRange->entity));
							  auto it = intersectionCache->meshes.find(hash);
							  if(it != intersectionCache->meshes.end())
								  addPrim = false;
							  else {
								  intersectionCache->meshes.insert(hash);
								  if(isMeshIntersectionInfo)
									  static_cast<MeshIntersectionInfo *>(outIntersectionInfo)->meshInfos.push_back({meshRange->mesh.get(), meshRange->entity});
							  }
						  }
					  }
					  if(addPrim && isPrimitiveIntersectionInfo) {
						  auto *primIntersectionInfo = static_cast<PrimitiveIntersectionInfo *>(outIntersectionInfo);
						  if(primIntersectionInfo->primitives.size() == primIntersectionInfo->primitives.capacity())
							  primIntersectionInfo->primitives.reserve(primIntersectionInfo->primitives.size() * 1.75);
						  primIntersectionInfo->primitives.push_back(primIdx);
					  }
				  }
			  }

			  if(intersectionCache)
				  intersectionCache->meshes.clear();
			  return hasHit;
		  },
		  [&testAabb](const Node &left, const Node &right) { return test_node_aabb_intersection(testAabb, left, right); });
	};
	if(!outIntersectionInfo)
		traverse.template operator()<true>();
	else
		traverse.template operator()<false>();
	return hasAnyHit;
}
bool pragma::bvh::test_bvh_intersection_with_obb(const MeshBvhTree &bvhData, const Vector3 &origin, const Quat &rot, const Vector3 &min, const Vector3 &max, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	auto planes = math::geometry::get_obb_planes(origin, rot, min, max);
	return test_bvh_intersection(
	  bvhData, [&origin, &rot, &min, &max, &planes](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return math::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, planes.begin(), planes.end()) != math::intersection::Intersect::Outside; },
	  [&min, &max, &origin, &rot](
	    const Primitive &prim) -> bool { return math::intersection::obb_triangle(min, max, origin, rot, *reinterpret_cast<const Vector3 *>(&prim.p0.values), *reinterpret_cast<const Vector3 *>(&prim.p1.values), *reinterpret_cast<const Vector3 *>(&prim.p2.values)); },
	  nodeIdx, outIntersectionInfo);
}
bool pragma::bvh::test_bvh_intersection_with_aabb(const MeshBvhTree &bvhData, const Vector3 &min, const Vector3 &max, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&min, &max](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return math::intersection::aabb_aabb(min, max, aabbMin, aabbMax) != math::intersection::Intersect::Outside; },
	  [&min, &max](const Primitive &prim) -> bool { return math::intersection::aabb_triangle(min, max, *reinterpret_cast<const Vector3 *>(&prim.p0.values), *reinterpret_cast<const Vector3 *>(&prim.p1.values), *reinterpret_cast<const Vector3 *>(&prim.p2.values)); }, nodeIdx,
	  outIntersectionInfo);
}
bool pragma::bvh::test_bvh_intersection_with_kdop(const MeshBvhTree &bvhData, const std::vector<math::Plane> &kdop, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&kdop](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return math::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, kdop.begin(), kdop.end()) != math::intersection::Intersect::Outside; },
	  [&kdop](const Primitive &prim) -> bool {
		  // TODO: Use pragma::math::intersection::triangle_in_plane_mesh() once it's implemented

		  // Use AABB approximation for intersection check
		  Vector3 v0, v1;
		  get_bvh_bounds(prim.get_bbox(), v0, v1);
		  return math::intersection::aabb_in_plane_mesh(v0, v1, kdop.begin(), kdop.end()) != math::intersection::Intersect::Outside;
	  },
	  nodeIdx, outIntersectionInfo);
}

static std::unique_ptr<bvh::v2::ThreadPool> g_threadPool {};
static size_t g_bvhCount = 0;
pragma::bvh::BvhTree::BvhTree()
{
	if(g_bvhCount++ == 0)
		g_threadPool = std::make_unique<::bvh::v2::ThreadPool>();
}
pragma::bvh::BvhTree::~BvhTree()
{
	if(--g_bvhCount == 0)
		g_threadPool = nullptr;
}
bvh::v2::ThreadPool &pragma::bvh::BvhTree::GetThreadPool() { return *g_threadPool; }

void pragma::bvh::BvhTree::InitializeBvh()
{
	auto config = InitializeExecutor();
	if(!DoInitializeBvh(*executor, config))
		executor = {};
}

template<typename T>
static std::unique_ptr<pragma::bvh::Executor> make_executor()
{
	std::unique_ptr<pragma::bvh::Executor> executor;
	if constexpr(std::is_same_v<T, bvh::v2::ParallelExecutor>)
		executor = std::make_unique<T>(*g_threadPool);
	else
		executor = std::make_unique<T>();
	return executor;
}
bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config pragma::bvh::BvhTree::InitializeExecutor()
{
	executor = make_executor<Executor>();
	::bvh::v2::DefaultBuilder<Node>::Config config;
	config.quality = ::bvh::v2::DefaultBuilder<Node>::Quality::High;
	return config;
}

void pragma::bvh::MeshBvhTree::Refit()
{
	bvh.refit([this](Node &node) {
		auto begin = node.index.first_id();
		auto end = begin + node.index.prim_count();
		for(size_t i = begin; i < end; ++i) {
			size_t j = bvh.prim_ids[i];

			auto &prim = primitives[j];
			auto bbox = prim.get_bbox();
			node.set_bbox(bbox);
		}
	});
}

void pragma::bvh::MeshBvhTree::Update()
{
	if(!dirty)
		return;
	dirty = false;
	Refit();
}

void pragma::bvh::MeshBvhTree::Deserialize(const std::vector<uint8_t> &data, std::vector<Primitive> &&primitives)
{
	InitializeExecutor();
	struct BinaryStream : public ::bvh::v2::InputStream {
		BinaryStream(const std::vector<uint8_t> &data) : m_data {data} {}
	  protected:
		virtual size_t read_raw(void *data, size_t sz) override
		{
			if(m_readPos >= m_data.size())
				return 0;
			auto remaining = m_data.size() - m_readPos;
			auto szRead = math::min(remaining, sz);
			memcpy(data, m_data.data() + m_readPos, szRead);
			m_readPos += szRead;
			return szRead;
		}
		const std::vector<uint8_t> &m_data;
		size_t m_readPos = 0;
	};

	BinaryStream binStream {data};
	bvh = Bvh::deserialize(binStream);
	this->primitives = std::move(primitives);
	InitializePrecomputedTris();
}

const pragma::bvh::MeshRange *pragma::bvh::MeshBvhTree::FindMeshRange(size_t primIdx) const
{
	MeshRange search {};
	search.start = primIdx * 3;
	auto it = std::upper_bound(meshRanges.begin(), meshRanges.end(), search);
	if(it == meshRanges.begin())
		return nullptr;
	--it;
	return &*it;
}

void pragma::bvh::MeshBvhTree::InitializePrecomputedTris()
{
	auto numTris = primitives.size();
	if(numTris == 0)
		return;
	precomputed_tris.resize(numTris);
	executor->for_each(0, numTris, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			auto j = should_permute ? bvh.prim_ids[i] : i;
			precomputed_tris[i] = primitives[j];
		}
	});
}

bool pragma::bvh::MeshBvhTree::DoInitializeBvh(Executor &executor, ::bvh::v2::DefaultBuilder<Node>::Config &config)
{
	auto numTris = primitives.size();
	if(numTris == 0)
		return false;
	std::vector<BBox> bboxes {numTris};
	std::vector<Vec> centers {numTris};
	executor.for_each(0, numTris, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			bboxes[i] = primitives[i].get_bbox();
			centers[i] = primitives[i].get_center();
		}
	});

	bvh = ::bvh::v2::DefaultBuilder<Node>::build(bboxes, centers, config);
	//bvh = ::bvh::v2::DefaultBuilder<Node>::build(GetThreadPool(), bboxes, centers, config);
	InitializePrecomputedTris();
	dirty = false;
	return true;
}

bool pragma::bvh::MeshBvhTree::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitData &outHitData)
{
	Update();

	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	auto &prim_id = outHitData.primitiveIndex = invalid_id;
	auto &u = outHitData.u;
	auto &v = outHitData.v;
	::bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
	auto ray = get_ray(origin, dir, minDist, maxDist);
	bvh.intersect<false, use_robust_traversal>(ray, bvh.get_root().index, stack, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			size_t j = bvh.prim_ids[i];
			auto &prim = primitives[j];
			PrecomputedTri tri {prim.p0, prim.p1, prim.p2};
			if(auto hit = tri.intersect(ray)) {
				prim_id = j;
				float d;
				std::tie(d, u, v) = *hit;
				ray.tmax = d;
			}
		}
		return prim_id != invalid_id;
	});
	auto distDiff = maxDist - minDist;
	if(distDiff > 0.0001f)
		outHitData.t = (ray.tmax - minDist) / distDiff;
	else
		outHitData.t = 0.f;
	return prim_id != invalid_id;
}

pragma::bvh::Ray pragma::bvh::get_ray(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) { return Ray {to_bvh_vector(origin), to_bvh_vector(dir), minDist, maxDist}; }

std::unordered_map<std::string, std::shared_ptr<pragma::geometry::ModelSubMesh>> pragma::bvh::get_uuid_mesh_map(asset::Model &mdl)
{
	std::unordered_map<std::string, std::shared_ptr<geometry::ModelSubMesh>> mdlMeshes;
	for(auto &mg : mdl.GetMeshGroups()) {
		for(auto &m : mg->GetMeshes()) {
			for(auto &sm : m->GetSubMeshes()) {
				if(!is_mesh_bvh_compatible(*sm))
					continue;
				mdlMeshes[util::uuid_to_string(sm->GetUuid())] = sm;
			}
		}
	}
	return mdlMeshes;
}

void pragma::bvh::debug::print_bvh_tree(Bvh &bvh)
{
	std::stringstream ss;
	std::function<void(const Node &, std::string)> printStack = nullptr;
	printStack = [&printStack, &ss, &bvh](const Node &node, std::string t) {
		auto bbox = node.get_bbox();
		auto min = from_bvh_vector(bbox.min);
		auto max = from_bvh_vector(bbox.max);
		auto isLeaf = node.index.prim_count() > 0;
		ss << t;
		if(isLeaf)
			ss << "Leaf";
		else
			ss << "Node";
		ss << "[" << min.x << "," << min.y << "," << min.z << "][" << max.x << "," << max.y << "," << max.z << "]\n";
		if(isLeaf)
			return;
		printStack(bvh.nodes[node.index.first_id()], t + "\t");
		printStack(bvh.nodes[node.index.first_id() + 1], t + "\t");
	};
	printStack(bvh.get_root(), "");
	Con::COUT << "BVH Tree:" << ss.str() << Con::endl;
}

void pragma::bvh::debug::draw_bvh_tree(const Game &game, Bvh &bvh, const math::ScaledTransform &pose, float duration)
{
	constexpr size_t stack_size = 64;
	::bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
	draw_node(game, bvh.get_root(), pose, DEFAULT_NODE_COLOR, duration);
	auto start = bvh.get_root().index;
	stack.push(start);
restart:
	while(!stack.is_empty()) {
		auto top = stack.pop();
		while(top.prim_count() == 0) {
			auto &left = bvh.nodes[top.first_id()];
			auto &right = bvh.nodes[top.first_id() + 1];

			draw_node(game, left, pose, DEFAULT_NODE_COLOR, duration);
			draw_node(game, right, pose, DEFAULT_NODE_COLOR, duration);

			if(true) {
				auto near_index = left.index;
				if(true)
					stack.push(right.index);
				top = near_index;
			}
		}
	}
}
void pragma::bvh::debug::draw_node(const Game &game, const BBox &bbox, const math::ScaledTransform &pose, const Color &col, float duration)
{
	auto vstart = from_bvh_vector(bbox.min);
	auto vend = from_bvh_vector(bbox.max);
	const_cast<Game &>(game).DrawBox(pose.GetOrigin(), vstart, vend, pose.GetRotation(), colors::White, col, duration);
}
void pragma::bvh::debug::draw_node(const Game &game, const Node &node, const math::ScaledTransform &pose, const Color &col, float duration)
{
	auto bbox = node.get_bbox();
	draw_node(game, bbox, pose, col, duration);
}

std::unique_ptr<pragma::bvh::MeshBvhTree> pragma::bvh::create_bvh_data(std::vector<Primitive> &&triangles)
{
	auto bvhData = std::make_unique<MeshBvhTree>();
	bvhData->primitives = std::move(triangles);
	bvhData->InitializeBvh();
	return bvhData;
}
