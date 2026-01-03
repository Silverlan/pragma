// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :model.model_class;
import :model.model_manager;
import :client_state;

std::shared_ptr<pragma::asset::Model> pragma::asset::CModelManager::CreateModel(uint32_t numBones, const std::string &mdlName) { return Model::Create<CModel>(&m_nw, numBones, mdlName); }
