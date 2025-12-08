// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:model.animation.vertex_animation;

export import :game.coordinate_system;
export import :types;
export import pragma.udm;

export {
	namespace pragma {
		class DLLNETWORK MeshVertexFrame : public std::enable_shared_from_this<pragma::MeshVertexFrame> {
		  public:
			enum class Flags : uint8_t {
				None = 0u,
				HasDeltaValues = 1u,
				HasNormals = HasDeltaValues << 1u,

				Count = 2
			};

			MeshVertexFrame() = default;
			MeshVertexFrame(const pragma::MeshVertexFrame &other);

			const std::vector<std::array<uint16_t, 4>> &GetVertices() const;
			std::vector<std::array<uint16_t, 4>> &GetVertices();
			const std::vector<std::array<uint16_t, 4>> &GetNormals() const;
			std::vector<std::array<uint16_t, 4>> &GetNormals();
			void SetVertexCount(uint32_t count);
			uint32_t GetVertexCount() const;

			void SetVertexPosition(uint32_t vertId, const Vector3 &pos);
			void SetVertexPosition(uint32_t vertId, const std::array<uint16_t, 3> &pos);
			void SetVertexPosition(uint32_t vertId, const std::array<uint16_t, 4> &pos);
			bool GetVertexPosition(uint32_t vertId, Vector3 &pos) const;

			void SetVertexNormal(uint32_t vertId, const Vector3 &n);
			void SetVertexNormal(uint32_t vertId, const std::array<uint16_t, 3> &n);
			void SetVertexNormal(uint32_t vertId, const std::array<uint16_t, 4> &n);
			bool GetVertexNormal(uint32_t vertId, Vector3 &n) const;

			void SetDeltaValue(uint32_t vertId, float deltaValue);
			void SetDeltaValue(uint32_t vertId, uint16_t deltaValue);
			bool GetDeltaValue(uint32_t vertId, float &deltaValue) const;

			void SetFlags(Flags flags);
			void SetFlagEnabled(Flags flags, bool enabled = true);
			bool IsFlagEnabled(Flags flags) const;
			Flags GetFlags() const;

			void Rotate(const Quat &rot);
			void Scale(const Vector3 &scale);
			void Mirror(pragma::Axis axis);

			bool operator==(const pragma::MeshVertexFrame &other) const;
			bool operator!=(const pragma::MeshVertexFrame &other) const { return !operator==(other); }
		  private:
			// Each uint16_t is a half-float
			std::vector<std::array<uint16_t, 4>> m_vertices = {}; // Fourth component is wrinkle data
			std::vector<std::array<uint16_t, 4>> m_normals = {};  // Optional
			Flags m_flags = Flags::None;
		};
		using namespace umath::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::MeshVertexFrame::Flags)

	class DLLNETWORK MeshVertexAnimation : public std::enable_shared_from_this<MeshVertexAnimation> {
	  public:
		MeshVertexAnimation() = default;
		MeshVertexAnimation(const MeshVertexAnimation &other);

		const pragma::MeshVertexFrame *GetFrame(uint32_t frameId) const;
		pragma::MeshVertexFrame *GetFrame(uint32_t frameId);

		const std::vector<std::shared_ptr<pragma::MeshVertexFrame>> &GetFrames() const;
		std::vector<std::shared_ptr<pragma::MeshVertexFrame>> &GetFrames();

		std::shared_ptr<pragma::MeshVertexFrame> AddFrame();

		pragma::geometry::ModelMesh *GetMesh() const;
		pragma::geometry::ModelSubMesh *GetSubMesh() const;
		void SetMesh(pragma::geometry::ModelMesh &mesh, pragma::geometry::ModelSubMesh &subMesh);

		void Rotate(const Quat &rot);
		void Scale(const Vector3 &scale);
		void Mirror(pragma::Axis axis);

		bool operator==(const MeshVertexAnimation &other) const;
		bool operator!=(const MeshVertexAnimation &other) const { return !operator==(other); }
	  private:
		mutable std::weak_ptr<pragma::geometry::ModelMesh> m_wpMesh = {};
		mutable std::weak_ptr<pragma::geometry::ModelSubMesh> m_wpSubMesh = {};
		std::vector<std::shared_ptr<pragma::MeshVertexFrame>> m_frames;
	};

	class DLLNETWORK VertexAnimation : public std::enable_shared_from_this<VertexAnimation> {
	  public:
		static constexpr std::uint32_t layout_version = 1;

		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PMORPHANI_IDENTIFIER = "PMORPHANI";
		static std::shared_ptr<VertexAnimation> Create();
		static std::shared_ptr<VertexAnimation> Create(const VertexAnimation &other);
		static std::shared_ptr<VertexAnimation> Create(const std::string &name);
		static std::shared_ptr<VertexAnimation> Load(pragma::asset::Model &mdl, const udm::AssetData &data, std::string &outErr);

		virtual std::shared_ptr<VertexAnimation> Copy() const;
		virtual ~VertexAnimation() = default;

		std::shared_ptr<pragma::MeshVertexFrame> AddMeshFrame(pragma::geometry::ModelMesh &mesh, pragma::geometry::ModelSubMesh &subMesh);
		const std::vector<std::shared_ptr<MeshVertexAnimation>> &GetMeshAnimations() const;
		std::vector<std::shared_ptr<MeshVertexAnimation>> &GetMeshAnimations();

		bool GetMeshAnimationId(pragma::geometry::ModelSubMesh &subMesh, uint32_t &id) const;
		const MeshVertexAnimation *GetMeshAnimation(pragma::geometry::ModelSubMesh &subMesh) const;
		MeshVertexAnimation *GetMeshAnimation(pragma::geometry::ModelSubMesh &subMesh);

		const pragma::MeshVertexFrame *GetMeshFrame(pragma::geometry::ModelSubMesh &subMesh, uint32_t frameId) const;
		pragma::MeshVertexFrame *GetMeshFrame(pragma::geometry::ModelSubMesh &subMesh, uint32_t frameId);

		void SetName(const std::string &name);
		const std::string &GetName() const;

		void Rotate(const Quat &rot);
		void Scale(const Vector3 &scale);
		void Mirror(pragma::Axis axis);
		bool Save(pragma::asset::Model &mdl, udm::AssetDataArg outData, std::string &outErr);

		bool operator==(const VertexAnimation &other) const;
		bool operator!=(const VertexAnimation &other) const { return !operator==(other); }
	  protected:
		VertexAnimation() = default;
		VertexAnimation(const VertexAnimation &other);
		VertexAnimation(const std::string &name);
		bool LoadFromAssetData(pragma::asset::Model &mdl, const udm::AssetData &data, std::string &outErr);
		std::string m_name;
		std::vector<std::shared_ptr<MeshVertexAnimation>> m_meshAnims;
	};
};
