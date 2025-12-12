// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.model_manager;

export import :model.model;

export namespace pragma::asset {
	class DLLNETWORK ModelProcessor : public pragma::util::FileAssetProcessor {
	  public:
		ModelProcessor(pragma::util::AssetFormatLoader &loader, std::unique_ptr<pragma::util::IAssetFormatHandler> &&handler);
		virtual bool Load() override;
		virtual bool Finalize() override;

		std::shared_ptr<pragma::asset::Model> model = nullptr;
		std::string identifier;
		std::string formatExtension;
	};
	class DLLNETWORK ModelLoader : public pragma::util::TAssetFormatLoader<ModelProcessor> {
	  public:
		ModelLoader(pragma::util::IAssetManager &assetManager) : pragma::util::TAssetFormatLoader<ModelProcessor> {assetManager, "model"} {}
	  protected:
		virtual std::unique_ptr<pragma::util::IAssetProcessor> CreateAssetProcessor(const std::string &identifier, const std::string &ext, std::unique_ptr<pragma::util::IAssetFormatHandler> &&formatHandler) override;
	};
	struct DLLNETWORK ModelLoadInfo : public pragma::util::AssetLoadInfo {
		ModelLoadInfo(pragma::util::AssetLoadFlags flags = pragma::util::AssetLoadFlags::None);
	};
	class DLLNETWORK IModelFormatHandler : public pragma::util::IAssetFormatHandler {
	  public:
		IModelFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool LoadData(ModelProcessor &processor, ModelLoadInfo &info);
		std::shared_ptr<pragma::asset::Model> model = nullptr;
	};
	class DLLNETWORK PmdlFormatHandler : public IModelFormatHandler {
	  public:
		PmdlFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool LoadData(ModelProcessor &processor, ModelLoadInfo &info) override;
	};
	class DLLNETWORK WmdFormatHandler : public IModelFormatHandler {
	  public:
		WmdFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool LoadData(ModelProcessor &processor, ModelLoadInfo &info) override;
	  private:
		void LoadBones(unsigned short version, unsigned int numBones, pragma::asset::Model &mdl);
		void LoadAttachments(pragma::asset::Model &mdl);
		void LoadObjectAttachments(pragma::asset::Model &mdl);
		void LoadHitboxes(uint16_t version, pragma::asset::Model &mdl);
		void LoadMeshes(unsigned short version, pragma::asset::Model &mdl, const std::function<std::shared_ptr<pragma::geometry::ModelMesh>()> &meshFactory, const std::function<std::shared_ptr<pragma::geometry::ModelSubMesh>()> &subMeshFactory);
		void LoadLODData(unsigned short version, pragma::asset::Model &mdl);
		void LoadBodygroups(pragma::asset::Model &mdl);
		void LoadJoints(pragma::asset::Model &mdl);
		void LoadSoftBodyData(pragma::asset::Model &mdl, pragma::physics::CollisionMesh &colMesh);
		void LoadCollisionMeshes(pragma::Game *game, unsigned short version, pragma::asset::Model &mdl, physics::SurfaceMaterial *smDefault = nullptr);
		void LoadBlendControllers(pragma::asset::Model &mdl);
		void LoadIKControllers(uint16_t version, pragma::asset::Model &mdl);
		void LoadAnimations(unsigned short version, pragma::asset::Model &mdl);
		void LoadChildBones(const pragma::animation::Skeleton &skeleton, std::shared_ptr<pragma::animation::Bone> bone);
		bool m_bStatic;
		pragma::Game *m_gameState = nullptr;
	};

	class DLLNETWORK SourceMdlFormatHandler : public pragma::util::IImportAssetFormatHandler {
	  public:
		SourceMdlFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK Source2VmdlFormatHandler : public pragma::util::IImportAssetFormatHandler {
	  public:
		Source2VmdlFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK NifFormatHandler : public pragma::util::IImportAssetFormatHandler {
	  public:
		NifFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK AssimpFormatHandler : public pragma::util::IImportAssetFormatHandler {
	  public:
		AssimpFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	  private:
	};

	class DLLNETWORK BlenderFormatHandler : public pragma::util::IImportAssetFormatHandler {
	  public:
		BlenderFormatHandler(pragma::util::IAssetManager &assetManager, std::string ext);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	  private:
		std::string m_ext;
	};

	class DLLNETWORK AssetManagerFormatHandler : public pragma::util::IImportAssetFormatHandler {
	  public:
		AssetManagerFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK PmxFormatHandler : public AssetManagerFormatHandler {
	  public:
		PmxFormatHandler(pragma::util::IAssetManager &assetManager);
		virtual bool Import(const std::string &outputPath, std::string &outFilePath) override;
	};

	class DLLNETWORK ModelManager : public pragma::util::TFileAssetManager<pragma::asset::Model, ModelLoadInfo> {
	  public:
		ModelManager(pragma::NetworkState &nw);
		virtual ~ModelManager() = default;

		using pragma::util::IAssetManager::FlagForRemoval;
		void FlagForRemoval(const pragma::asset::Model &mdl, bool flag = true);

		std::shared_ptr<pragma::asset::Model> CreateModel(const std::string &name = "", bool bAddReference = true, bool addToCache = false);
		//virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr);
		NetworkState &GetNetworkState() { return m_nw; }

		virtual std::shared_ptr<pragma::asset::Model> CreateModel(uint32_t numBones, const std::string &mdlName);
		virtual std::shared_ptr<pragma::geometry::ModelMesh> CreateMesh();
		virtual std::shared_ptr<pragma::geometry::ModelSubMesh> CreateSubMesh();
	  protected:
		virtual void InitializeProcessor(pragma::util::IAssetProcessor &processor) override;
		virtual pragma::util::AssetObject InitializeAsset(const pragma::util::Asset &asset, const pragma::util::AssetLoadJob &job) override;
		//bool PrecacheModel(const std::string &mdlName) const;
		//std::shared_ptr<Model> LoadModel(const std::string &cacheName,const std::shared_ptr<ufile::IFile> &file,const std::string &ext);

		NetworkState &m_nw;
		//virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const;
	};
};
