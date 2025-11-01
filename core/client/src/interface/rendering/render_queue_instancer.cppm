// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.render_queue_instancer;

export import :rendering.render_queue;

export namespace pragma::rendering {
	class DLLCLIENT RenderQueueInstancer {
	  public:
		RenderQueueInstancer(pragma::rendering::RenderQueue &renderQueue);
		void Process();
	  private:
		util::Hash CalcNextEntityHash(uint32_t &outNumMeshes, EntityIndex &entIndex);
		void ProcessInstantiableList(uint32_t endIndex, uint32_t numMeshes, util::Hash hash);

		pragma::rendering::RenderQueue &m_renderQueue;
		uint32_t m_curIndex = 0;
		uint32_t m_instanceThreshold = 2;
		std::vector<EntityIndex> m_instantiableEntityList;
	};
};
