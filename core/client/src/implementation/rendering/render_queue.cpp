// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.render_queue;
import :client_state;
import :engine;
import :entities.components.render;
import :game;

using namespace pragma::rendering;

SortingKey::SortingKey(material::MaterialIndex material, prosper::ShaderIndex shader, bool instantiable, bool translucentKey)
{
	if(translucentKey) {
		translucent.material = material;
		translucent.shader = shader;
		translucent.instantiable = instantiable;
	}
	else {
		opaque.material = material;
		opaque.shader = shader;
		opaque.instantiable = instantiable;
	}
}
void SortingKey::SetDistance(const Vector3 &origin, const CCameraComponent &cam)
{
	auto &p0 = origin;
	auto &p1 = cam.GetEntity().GetPosition();
	auto v = p1 - p0;
	// Double precision since we're working with the square of the far plane
	auto distSqr = std::max(math::pow2(static_cast<double>(v.x)) + math::pow2(static_cast<double>(v.y)) + math::pow2(static_cast<double>(v.z)), 0.0);
	SetDistance(distSqr, cam);
}

void SortingKey::SetDistance(double distSqr, const CCameraComponent &cam)
{
	auto nearZ = math::pow2(static_cast<double>(cam.GetNearZ()));
	auto farZ = math::pow2(static_cast<double>(cam.GetFarZ()));
	// Map the distance to [0,1] and invert (since we want objects that are furthest away to be rendered first)
	distSqr = 1.0 - std::clamp(distSqr / (farZ + nearZ), 0.0, 1.0);
	// Note: 16 bit precision is not enough, but 24 bit might be. For now we'll use a 32 bit integer,
	// since we have that much space available anyway
	translucent.distance = glm::floatBitsToUint(static_cast<float>(distSqr));
}

RenderQueueItem::RenderQueueItem(ecs::CBaseEntity &ent, RenderMeshIndex meshIdx, material::CMaterial &mat, prosper::PipelineID pipelineId, const TranslucencyPassInfo *optTranslucencyPassInfo)
    : material {mat.GetIndex()}, pipelineId {pipelineId}, entity {ent.GetLocalIndex()}, mesh {meshIdx}, translucentKey {optTranslucencyPassInfo ? true : false}
{
	instanceSetIndex = UNIQUE;
	auto &renderC = *ent.GetRenderComponent();
	auto instantiable = renderC.IsInstantiable();
	if(optTranslucencyPassInfo) {
		if(optTranslucencyPassInfo->distanceOverrideSqr)
			sortingKey.SetDistance(*optTranslucencyPassInfo->distanceOverrideSqr, optTranslucencyPassInfo->camera);
		else {
			// TODO: This isn't very efficient, find a better way to handle this!
			auto &renderMeshes = renderC.GetRenderMeshes();
			if(meshIdx < renderMeshes.size()) {
				auto &pose = ent.GetPose();
				auto pos = pose * renderMeshes[meshIdx]->GetCenter();
				sortingKey.SetDistance(pos, optTranslucencyPassInfo->camera);
			}
		}

		sortingKey.translucent.material = material;
		sortingKey.translucent.shader = pipelineId;
		sortingKey.translucent.instantiable = renderC.IsInstantiable();
	}
	else {
		sortingKey.opaque.distance = 0;
		sortingKey.opaque.material = material;
		sortingKey.opaque.shader = pipelineId;
		sortingKey.opaque.instantiable = renderC.IsInstantiable();
	}
}

pragma::material::CMaterial *RenderQueueItem::GetMaterial() const { return static_cast<material::CMaterial *>(get_client_state()->GetMaterialManager().GetAsset(material)->assetObject.get()); }
pragma::ecs::CBaseEntity *RenderQueueItem::GetEntity() const { return static_cast<ecs::CBaseEntity *>(get_cgame()->GetEntityByLocalIndex(entity)); }
pragma::geometry::CModelSubMesh *RenderQueueItem::GetMesh() const
{
	auto *ent = GetEntity();
	auto *renderC = ent ? ent->GetRenderComponent() : nullptr;
	if(!renderC)
		return nullptr;
	auto &renderMeshes = renderC->GetRenderMeshes();
	return mesh < renderMeshes.size() ? static_cast<geometry::CModelSubMesh *>(renderMeshes[mesh].get()) : nullptr;
}
prosper::ShaderGraphics *RenderQueueItem::GetShader(uint32_t &outPipelineIndex) const
{
	auto *shader = get_cengine()->GetRenderContext().GetShaderPipeline(pipelineId, outPipelineIndex);
	return shader && shader->IsGraphicsShader() ? static_cast<prosper::ShaderGraphics *>(shader) : nullptr;
}

std::shared_ptr<RenderQueue> RenderQueue::Create(std::string name) { return std::shared_ptr<RenderQueue> {new RenderQueue {std::move(name)}}; }

RenderQueue::RenderQueue(std::string name) : m_name {std::move(name)} {}

RenderQueue::~RenderQueue() {}

void RenderQueue::Reserve()
{
	if(queue.size() < queue.capacity())
		return;
	auto newCapacity = queue.size() * 1.1 + 100;
	queue.reserve(newCapacity);
	sortedItemIndices.reserve(newCapacity);
}
void RenderQueue::Clear()
{
	queue.clear();
	sortedItemIndices.clear();
}
void RenderQueue::Add(ecs::CBaseEntity &ent, RenderMeshIndex meshIdx, material::CMaterial &mat, prosper::PipelineID pipelineId, const CCameraComponent *optCam)
{
	if(optCam) {
		RenderQueueItem::TranslucencyPassInfo translucencyPassInfo {*optCam};
		Add({ent, meshIdx, mat, pipelineId, &translucencyPassInfo});
	}
	else
		Add({ent, meshIdx, mat, pipelineId});
}
void RenderQueue::Add(const RenderQueueItem &item)
{
	m_queueMutex.lock();
	Reserve();
	queue.push_back(item);
	sortedItemIndices.push_back({queue.size() - 1, item.sortingKey});
	m_queueMutex.unlock();
}
void RenderQueue::Add(const std::vector<RenderQueueItem> &items)
{
	m_queueMutex.lock();
	auto offset = queue.size();
	queue.resize(queue.size() + items.size());
	sortedItemIndices.resize(queue.size());
	for(auto i = decltype(items.size()) {0u}; i < items.size(); ++i) {
		queue[offset + i] = items[i];
		sortedItemIndices[offset + i] = {offset + i, items[i].sortingKey};
	}
	m_queueMutex.unlock();
}
void RenderQueue::Sort()
{
	std::sort(sortedItemIndices.begin(), sortedItemIndices.end(), [](const RenderQueueItemSortPair &a, const RenderQueueItemSortPair &b) {
		static_assert(sizeof(decltype(a.second)) == sizeof(uint64_t));
		return *reinterpret_cast<const uint64_t *>(&a.second) < *reinterpret_cast<const uint64_t *>(&b.second);
	});
}

void RenderQueue::Merge(const RenderQueue &other)
{
	queue.reserve(queue.size() + other.queue.size());
	sortedItemIndices.reserve(queue.size());
	for(auto i = decltype(other.queue.size()) {0u}; i < other.queue.size(); ++i) {
		auto &item = other.queue.at(i);
		queue.push_back(item);
		sortedItemIndices.push_back(other.sortedItemIndices.at(i));
		sortedItemIndices.back().first = queue.size() - 1;
	}
}

void RenderQueue::Lock()
{
	std::lock_guard<std::mutex> lock {m_threadWaitMutex};
	m_locked = true;
}
void RenderQueue::Unlock()
{
	std::lock_guard<std::mutex> lock {m_threadWaitMutex};
	m_locked = false;
	m_threadWaitCondition.notify_all();
}
bool RenderQueue::IsComplete() const { return !m_locked; }
void RenderQueue::WaitForCompletion(RenderPassStats *optStats) const
{
	std::chrono::steady_clock::time_point t;
	if(optStats)
		t = std::chrono::steady_clock::now();

	std::unique_lock<std::mutex> mlock(m_threadWaitMutex);
	m_threadWaitCondition.wait(mlock, [this]() -> bool { return !m_locked; });

	if(optStats)
		(*optStats)->AddTime(RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);
}

//////////////////////

RenderQueueBuilder::RenderQueueBuilder() { Exec(); }

RenderQueueBuilder::~RenderQueueBuilder()
{
	m_threadRunning = false;
	Flush();
	m_threadWaitCondition.notify_one();
	if(m_thread.joinable())
		m_thread.join();
}

void RenderQueueBuilder::Exec()
{
	m_threadRunning = true;
	m_thread = std::thread {[this]() {
		for(;;) {
			std::unique_lock<std::mutex> mlock {m_workMutex};
			m_threadWaitCondition.wait(mlock, [this]() -> bool { return !m_threadRunning || m_hasWork || (!m_hasWork && m_readyForCompletion); });

			if(m_renderQueueBuildQueue.empty()) {
				if(m_readyForCompletion) {
					// All queues have been built, it's time to finalize them
					m_readyForCompletion = false;
					while(!m_renderQueueCompleteQueue.empty()) {
						m_renderQueueCompleteQueue.front()();
						m_renderQueueCompleteQueue.pop();
					}
					continue;
				}
				m_hasWork = false;
				if(m_threadRunning == false)
					return;
				continue;
			}
			auto worker = std::move(m_renderQueueBuildQueue.front());
			m_renderQueueBuildQueue.pop();

			mlock.unlock();
			worker();
		}
	}};
	util::set_thread_name(m_thread, "render_queue_builder");
}

void RenderQueueBuilder::SetReadyForCompletion()
{
	std::scoped_lock lock {m_workMutex};
	m_readyForCompletion = true;
	m_threadWaitCondition.notify_one();
}

void RenderQueueBuilder::Append(const std::function<void()> &workerBuildQueue, const std::function<void()> &workerCompleteQueue)
{
	// if(m_readyForCompletion)
	// 	throw std::runtime_error{"Attempted to append render queue builder job after ready-for-completion flag has been set, this is not allowed!"};
	std::scoped_lock lock {m_workMutex};
	m_renderQueueBuildQueue.push(workerBuildQueue);
	if(workerCompleteQueue)
		m_renderQueueCompleteQueue.push(workerCompleteQueue);
	m_hasWork = true;
	m_threadWaitCondition.notify_one();
}

void RenderQueueBuilder::Flush()
{
	// TODO: Use a condition variable?
	while(m_hasWork)
		;
}

bool RenderQueueBuilder::HasWork() const { return m_hasWork; }
uint32_t RenderQueueBuilder::GetWorkQueueCount() const
{
	const_cast<RenderQueueBuilder *>(this)->m_workMutex.lock();
	auto numEls = m_renderQueueBuildQueue.size() + m_renderQueueCompleteQueue.size();
	const_cast<RenderQueueBuilder *>(this)->m_workMutex.unlock();
	return numEls;
}

void RenderQueueBuilder::BuildRenderQueues() {}
