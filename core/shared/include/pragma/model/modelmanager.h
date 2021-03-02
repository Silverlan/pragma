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
#include <unordered_set>

namespace pragma::asset
{
	class DLLNETWORK ModelManager
	{
	public:
		static std::string GetNormalizedModelName(const std::string &mdlName);

		ModelManager(NetworkState &nw);
		virtual ~ModelManager()=default;

		uint32_t ClearUnused();
		uint32_t ClearFlagged();
		uint32_t Clear();
		void FlagForRemoval(Model &mdl);
		void FlagAllForRemoval();

		const std::unordered_map<std::string,std::shared_ptr<Model>> &GetCache() const;
		const Model *FindCachedModel(const std::string &mdlName) const;
		Model *FindCachedModel(const std::string &mdlName);
		std::shared_ptr<Model> CreateModel(const std::string &name="",bool bAddReference=true,bool addToCache=false);
		virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr);
	protected:
		static std::string GetCacheName(const std::string &mdlName);
		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones,const std::string &mdlName);

		NetworkState &m_nw;
		virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const;
	private:
		std::shared_ptr<Model> FindModel(const std::string &mdlName);

		std::unordered_map<std::string,std::shared_ptr<Model>> m_cache;
		std::unordered_set<std::string> m_flaggedForDeletion;
	};
};

#endif
