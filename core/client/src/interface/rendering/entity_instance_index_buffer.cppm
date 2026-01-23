// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.entity_instance_index_buffer;

export import :entities.components.enums;
export import :rendering.render_queue;

export namespace pragma::rendering {
	constexpr auto SINGLE_INSTANCE_RENDER_BUFFER_INDEX = std::numeric_limits<RenderBufferIndex>::max();
	class DLLCLIENT EntityInstanceIndexBuffer {
	  public:
		EntityInstanceIndexBuffer();
		~EntityInstanceIndexBuffer();
		std::shared_ptr<prosper::IBuffer> AddInstanceList(const RenderQueue &renderQueue, std::vector<RenderBufferIndex> &&instanceList, util::Hash hash);
		const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetBuffer() const { return m_buffer; }
		const std::shared_ptr<prosper::IBuffer> &GetZeroIndexBuffer() const { return m_zeroIndexBuffer; }
		bool BufferUpdateRequred() const { return !m_bufferUpdateQueue.empty(); }

		void UpdateBufferData(const RenderQueue &renderQueue); // Executed from main thread
		void UpdateAndClearUnusedBuffers();                    // Executed from render queue thread
	  private:
		prosper::FrameIndex GetCurrentFrameIndex() const;

		struct BufferInfo {
			std::shared_ptr<prosper::IBuffer> buffer;
			prosper::FrameIndex lastUse;
		};
		std::shared_ptr<prosper::IDynamicResizableBuffer> m_buffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_zeroIndexBuffer = nullptr;
		std::unordered_map<util::Hash, BufferInfo> m_cachedBuffers;
		prosper::FrameIndex m_lastFrameCheck = std::numeric_limits<prosper::FrameIndex>::max();

		struct BufferUpdateData {
			std::shared_ptr<prosper::IBuffer> buffer;
			std::vector<RenderBufferIndex> data;
		};
		std::mutex m_bufferUpdateQueueMutex;
		std::unordered_map<const RenderQueue *, std::queue<BufferUpdateData>> m_bufferUpdateQueue;
	};
};
