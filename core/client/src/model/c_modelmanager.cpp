/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/debug/intel_vtune.hpp>

std::shared_ptr<Model> pragma::asset::CModelManager::CreateModel(uint32_t numBones, const std::string &mdlName) { return Model::Create<CModel>(&m_nw, numBones, mdlName); }
