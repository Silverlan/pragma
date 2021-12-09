/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_MODEL_MANAGER_HPP__
#define __PRAGMA_MODEL_MANAGER_HPP__

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
#include <sharedutils/asset_loader/file_asset_manager.hpp>
#include <sharedutils/asset_loader/asset_format_loader.hpp>
#include <sharedutils/asset_loader/file_asset_processor.hpp>
#include <unordered_set>

namespace pragma::asset
{
	class DLLNETWORK ModelProcessor
		: public util::FileAssetProcessor
	{
	public:
		ModelProcessor(util::AssetFormatLoader &loader,std::unique_ptr<util::IAssetFormatHandler> &&handler);
		virtual bool Load() override;
		virtual bool Finalize() override;

		std::shared_ptr<Model> model = nullptr;
		std::string identifier;
		std::string formatExtension;
	};
	class DLLNETWORK ModelLoader
		: public util::TAssetFormatLoader<ModelProcessor>
	{
	public:
		ModelLoader(util::IAssetManager &assetManager)
			: util::TAssetFormatLoader<ModelProcessor>{assetManager}
		{}
	protected:
		virtual std::unique_ptr<util::IAssetProcessor> CreateAssetProcessor(
			const std::string &identifier,const std::string &ext,std::unique_ptr<util::IAssetFormatHandler> &&formatHandler
		) override;
	};
	struct DLLNETWORK ModelLoadInfo
		: public util::AssetLoadInfo
	{
		ModelLoadInfo(util::AssetLoadFlags flags=util::AssetLoadFlags::None);
	};
	class DLLNETWORK ModelFormatHandler
		: public util::IAssetFormatHandler
	{
	public:
		ModelFormatHandler(util::IAssetManager &assetManager);
		bool LoadData(ModelProcessor &processor,ModelLoadInfo &info);
		std::shared_ptr<Model> model = nullptr;
	};
	class DLLNETWORK ModelManager
		: public util::TFileAssetManager<Model,ModelLoadInfo>
	{
	public:
		ModelManager(NetworkState &nw);
		virtual ~ModelManager()=default;

		using util::IAssetManager::FlagForRemoval;
		void FlagForRemoval(const Model &mdl,bool flag=true);

		std::shared_ptr<Model> CreateModel(const std::string &name="",bool bAddReference=true,bool addToCache=false);
		//virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr);
		NetworkState &GetNetworkState() {return m_nw;}

		virtual std::shared_ptr<Model> Load(const std::string &mdlName,std::unique_ptr<ufile::IFile> &&f,const std::string &ext,const std::function<std::shared_ptr<Model>(const std::string&)> &loadModel);
	protected:
		virtual void InitializeProcessor(util::IAssetProcessor &processor) override;
		virtual util::AssetObject InitializeAsset(const util::Asset &asset,const util::AssetLoadJob &job) override;
		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones,const std::string &mdlName);
		//bool PrecacheModel(const std::string &mdlName) const;
		//std::shared_ptr<Model> LoadModel(const std::string &cacheName,const std::shared_ptr<ufile::IFile> &file,const std::string &ext);

		NetworkState &m_nw;
		//virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const;
	};
};

#endif
