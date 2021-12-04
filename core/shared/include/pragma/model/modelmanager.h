/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __MODELMANAGER_H__
#define __MODELMANAGER_H__

#include <unordered_map>
#include <vector>
#include <string>
#include "pragma/networkdefinitions.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/file_formats/wmd.h"
#include <sharedutils/util_string.h>
#include "pragma/entities/baseworld.h"
#include <sharedutils/util_file.h>
#include <sharedutils/asset_loader/asset_manager.hpp>
#include <unordered_set>

namespace pragma::asset
{
	struct DLLNETWORK ModelAsset
		: public util::IAsset
	{
	public:
		ModelAsset(const std::shared_ptr<Model> &model)
			: model{model}
		{}
		virtual bool IsInUse() const override;
		std::shared_ptr<Model> model;
	};
	class DLLNETWORK ModelManager
		: public util::IAssetManager
	{
	public:
		using AssetType = ModelAsset;
		ModelManager(NetworkState &nw);
		virtual ~ModelManager()=default;

		using util::IAssetManager::FlagForRemoval;
		void FlagForRemoval(const Model &mdl,bool flag=true);

		std::shared_ptr<Model> CreateModel(const std::string &name="",bool bAddReference=true,bool addToCache=false);
		virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr);
	protected:
		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones,const std::string &mdlName);

		NetworkState &m_nw;
		virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const;
	};
};

#endif
