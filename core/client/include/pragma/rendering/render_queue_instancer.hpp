/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __RENDER_QUEUE_INSTANCER_HPP__
#define __RENDER_QUEUE_INSTANCER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/render_queue.hpp"
#include <sharedutils/util_hash.hpp>

using EntityIndex = uint32_t;
namespace pragma::rendering {
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

#endif
