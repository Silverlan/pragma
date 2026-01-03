// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.model_class;

export import :model.mesh;

export namespace pragma::asset {
	class DLLCLIENT CModel : public Model {
	  public:
		CModel(NetworkState *nw, uint32_t numBones, const std::string &name = "");
		void AddMesh(const std::string &meshGroup, const std::shared_ptr<geometry::CModelMesh> &mesh);
		virtual void PrecacheTextureGroup(uint32_t i) override;
		virtual void PrecacheTexture(uint32_t texId, bool bReload = false) override;
		virtual void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData) override;

		virtual std::shared_ptr<geometry::ModelMesh> CreateMesh() const override;
		virtual std::shared_ptr<geometry::ModelSubMesh> CreateSubMesh() const override;

		const std::shared_ptr<prosper::IBuffer> &GetVertexAnimationBuffer() const;
		bool GetVertexAnimationBufferFrameOffset(uint32_t vaIdx, geometry::CModelSubMesh &subMesh, uint32_t frameId, uint64_t &offset) const;
	  protected:
		virtual void AddMesh(const std::string &meshGroup, const std::shared_ptr<geometry::ModelMesh> &mesh) override;
		virtual void OnMaterialMissing(const std::string &matName) override;

		std::shared_ptr<prosper::IBuffer> m_vertexAnimationBuffer = nullptr;
		std::vector<std::vector<std::vector<uint32_t>>> m_frameIndices = {};
		void UpdateVertexAnimationBuffer();
	};
}
