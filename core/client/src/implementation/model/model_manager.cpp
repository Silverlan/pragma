// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/debug/intel_vtune.hpp>

module pragma.client.model;

import :model_manager;

import pragma.client.client_state;

std::shared_ptr<Model> pragma::asset::CModelManager::CreateModel(uint32_t numBones, const std::string &mdlName) { return Model::Create<CModel>(&m_nw, numBones, mdlName); }
