// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.model_manager;

export import :model.model;

export namespace pragma::asset {
	class DLLNETWORK ModelProcessor : public util::FileAssetProcessor {
	  public:
		ModelProcessor(util::AssetFormatLoader &loader, std::unique_ptr<util::IAssetFormatHandler> &&handler);
		virtual bool Load() override;
		virtual bool Finalize() override;

		std::shared_ptr<Model> model = nullptr;
		std::string identifier;
		std::string formatExtension;
	};
	class DLLNETWORK ModelLoader : public util::TAssetFormatLoader<ModelProcessor> {
	  public:
		ModelLoader(util::IAssetManager &assetManager) : TAssetFormatLoader<ModelProcessor> {assetManager, "model"} {}
	  protected:
		virtual std::unique_ptr<util::IAssetProcessor> CreateAssetProcessor(const std::string &identifier, const std::string &ext, std::unique_ptr<util::IAssetFormatHandler> &&formatHandler) override;
	};
	struct DLLNETWORK ModelLoadInfo : public util::AssetLoadInfo {
		ModelLoadInfo(util::AssetLoadFlags flags = util::AssetLoadFlags::None);
	};
	class DLLNETWORK IModelFormatHandler : public util::IAssetFormatHandler {
	  public:
		IModelFormatHandler(util::IAssetManager &assetManager);
		virtual bool LoadData(ModelProcessor &processor, ModelLoadInfo &info);
		std::shared_ptr<Model> model = nullptr;
	};
	class DLLNETWORK PmdlFormatHandler : public IModelFormatHandler {
	  public:
		PmdlFormatHandler(util::IAssetManager &assetManager);
		virtual bool LoadData(ModelProcessor &processor, ModelLoadInfo &info) override;
	};
	class DLLNETWORK WmdFormatHandler : public IModelFormatHandler {
	  public:
		WmdFormatHandler(util::IAssetManager &assetManager);
		virtual bool LoadData(ModelProcessor &processor, ModelLoadInfo &info) override;
	  private:
		void LoadBones(unsigned short version, unsigned int numBones, Model &mdl);
		void LoadAttachments(Model &mdl);
		void LoadObjectAttachments(Model &mdl);
		void LoadHitboxes(uint16_t version, Model &mdl);
		void LoadMeshes(unsigned short version, Model &mdl, const std::function<std::shared_ptr<geometry::ModelMesh>()> &meshFactory, const std::function<std::shared_ptr<geometry::ModelSubMesh>()> &subMeshFactory);
		void LoadLODData(unsigned short version, Model &mdl);
		void LoadBodygroups(Model &mdl);
		void LoadJoints(Model &mdl);
		void LoadSoftBodyData(Model &mdl, physics::CollisionMesh &colMesh);
		void LoadCollisionMeshes(Game *game, unsigned short version, Model &mdl, physics::SurfaceMaterial *smDefault = nullptr);
		void LoadBlendControllers(Model &mdl);
		void LoadIKControllers(uint16_t version, Model &mdl);
		void LoadAnimations(unsigned short version, Model &mdl);
		void LoadChildBones(const animation::Skeleton &skeleton, std::shared_ptr<animation::Bone> bone);
		bool m_bStatic;
		Game *m_gameState = nullptr;
	};

	class DLLNETWORK SourceMdlFormatHandler : public util::IImportAssetFormatHandler {
	  public:
		SourceMdlFormatHandler(util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK Source2VmdlFormatHandler : public util::IImportAssetFormatHandler {
	  public:
		Source2VmdlFormatHandler(util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK NifFormatHandler : public util::IImportAssetFormatHandler {
	  public:
		NifFormatHandler(util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK AssimpFormatHandler : public util::IImportAssetFormatHandler {
	  public:
		AssimpFormatHandler(util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	  private:
	};

	class DLLNETWORK BlenderFormatHandler : public util::IImportAssetFormatHandler {
	  public:
		BlenderFormatHandler(util::IAssetManager &assetManager, std::string ext);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	  private:
		std::string m_ext;
	};

	class DLLNETWORK AssetManagerFormatHandler : public util::IImportAssetFormatHandler {
	  public:
		AssetManagerFormatHandler(util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK PmxFormatHandler : public AssetManagerFormatHandler {
	  public:
		PmxFormatHandler(util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK ModelManager : public util::TFileAssetManager<Model, ModelLoadInfo> {
	  public:
		ModelManager(NetworkState &nw);
		virtual ~ModelManager() = default;

		using IAssetManager::FlagForRemoval;
		void FlagForRemoval(const Model &mdl, bool flag = true);

		std::shared_ptr<Model> CreateModel(const std::string &name = "", bool bAddReference = true, bool addToCache = false);
		//virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr);
		NetworkState &GetNetworkState() { return m_nw; }

		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones, const std::string &mdlName);
		virtual std::shared_ptr<geometry::ModelMesh> CreateMesh();
		virtual std::shared_ptr<geometry::ModelSubMesh> CreateSubMesh();
	  protected:
		virtual void InitializeProcessor(util::IAssetProcessor &processor) override;
		virtual util::AssetObject InitializeAsset(const util::Asset &asset, const util::AssetLoadJob &job) override;
		//bool PrecacheModel(const std::string &mdlName) const;
		//std::shared_ptr<Model> LoadModel(const std::string &cacheName,const std::shared_ptr<ufile::IFile> &file,const std::string &ext);

		NetworkState &m_nw;
		//virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const;
	};
};
