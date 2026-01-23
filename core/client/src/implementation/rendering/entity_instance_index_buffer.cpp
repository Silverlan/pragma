// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.entity_instance_index_buffer;
import :engine;

using namespace pragma;

rendering::EntityInstanceIndexBuffer::EntityInstanceIndexBuffer()
{
	prosper::util::BufferCreateInfo bufCreateInfo = {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostAccessable | prosper::MemoryFeatureFlags::HostCoherent;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	bufCreateInfo.size = math::to_integral(GameLimits::MaxEntityInstanceCount) * sizeof(RenderBufferIndex);
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;

	m_buffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(bufCreateInfo, bufCreateInfo.size);
	m_buffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);

	auto id = SINGLE_INSTANCE_RENDER_BUFFER_INDEX;
	m_zeroIndexBuffer = m_buffer->AllocateBuffer(sizeof(RenderBufferIndex), &id);
}
rendering::EntityInstanceIndexBuffer::~EntityInstanceIndexBuffer()
{
	m_zeroIndexBuffer = nullptr;
	m_cachedBuffers.clear();
	m_buffer = nullptr;
}

prosper::FrameIndex rendering::EntityInstanceIndexBuffer::GetCurrentFrameIndex() const { return get_cengine()->GetRenderContext().GetLastFrameId(); }

void rendering::EntityInstanceIndexBuffer::UpdateAndClearUnusedBuffers()
{
	auto frameIndex = GetCurrentFrameIndex();
	if(frameIndex == m_lastFrameCheck)
		return;
	m_lastFrameCheck = frameIndex;
	for(auto it = m_cachedBuffers.begin(); it != m_cachedBuffers.end();) {
		auto &pair = *it;
		if(pair.second.lastUse >= (frameIndex - 1)) {
			// Buffer was in use either this frame or last frame; keep it alive
			++it;
			continue;
		}
		// Buffer is no longer in use
		it = m_cachedBuffers.erase(it);
	}
}

void rendering::EntityInstanceIndexBuffer::UpdateBufferData(const RenderQueue &renderQueue)
{
	m_bufferUpdateQueueMutex.lock();
	auto it = m_bufferUpdateQueue.find(&renderQueue);
	if(it == m_bufferUpdateQueue.end()) {
		m_bufferUpdateQueueMutex.unlock();
		return;
	}
	auto bufferUpdateQueue = std::move(it->second);
	m_bufferUpdateQueue.erase(it);
	m_bufferUpdateQueueMutex.unlock();

	while(bufferUpdateQueue.empty() == false) {
		auto updateData = std::move(bufferUpdateQueue.front());
		bufferUpdateQueue.pop();
		updateData.buffer->Write(0, updateData.data.size() * sizeof(updateData.data.front()), updateData.data.data());
	}
	// TODO: Enable instancing for shadows, etc.
	// TODO: Call from main thread before rendering -> After world geometry? -> WARNING: Some render queues might still run -> mutex lock?
	// TOOD: actually write data to buffers
}

std::shared_ptr<prosper::IBuffer> rendering::EntityInstanceIndexBuffer::AddInstanceList(const RenderQueue &renderQueue, std::vector<RenderBufferIndex> &&instanceList, util::Hash hash)
{
	auto itCache = m_cachedBuffers.find(hash);
	if(itCache != m_cachedBuffers.end()) {
		itCache->second.lastUse = GetCurrentFrameIndex();
		return itCache->second.buffer;
	}

	// Note: This is getting called from a separate thread, which means:
	// 1) We must ensure that the buffer doesn't get re-allocated (since this would invoke gl-calls)
	// 2) We mustn't write data to the buffer here (this would also invoke gl-calls)
	// The actual buffer updates will be performed on the main thread
	auto buf = m_buffer->AllocateBuffer(instanceList.size() * sizeof(instanceList.front()), sizeof(uint32_t) * 4 /* uvec4 alignment */, nullptr, false /* don't re-allocate buffer */);
	assert(buf != nullptr);
	if(buf == nullptr)
		return nullptr;
	// Buffers are cached for temporal frame coherence (objects on screen in the next frame are likely to be the same as in this frame).
	// The buffer is global, so this also works for special cases like VR, if the objects for both eyes are the same
	m_cachedBuffers[hash] = {buf, GetCurrentFrameIndex()};
	m_bufferUpdateQueueMutex.lock();
	auto it = m_bufferUpdateQueue.find(&renderQueue);
	if(it == m_bufferUpdateQueue.end())
		it = m_bufferUpdateQueue.insert(std::make_pair(&renderQueue, std::queue<BufferUpdateData> {})).first;
	it->second.push({buf, std::move(instanceList)});
	m_bufferUpdateQueueMutex.unlock();
	return buf;
}
