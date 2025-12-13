// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:map.entity_data;

export import :map.component_data;
export import :map.output;

export {
	namespace pragma::asset {
		class WorldData;
		class DLLNETWORK EntityData : public std::enable_shared_from_this<EntityData> {
		  public:
			enum class Flags : uint64_t { None = 0u, ClientsideOnly = 1u };

			static std::shared_ptr<EntityData> Create();

			bool IsWorld() const;
			bool IsSkybox() const;
			bool IsClientSideOnly() const;
			void SetClassName(const std::string &className);
			void SetLeafData(uint32_t firstLeaf, uint32_t numLeaves);
			void SetKeyValue(const std::string &key, const std::string &value);
			void AddOutput(const Output &output);

			uint32_t GetMapIndex() const;
			const std::string &GetClassName() const;
			Flags GetFlags() const;
			void SetFlags(Flags flags);
			std::shared_ptr<ComponentData> AddComponent(const std::string &name);
			const std::unordered_map<std::string, std::shared_ptr<ComponentData>> &GetComponents() const;
			std::unordered_map<std::string, std::shared_ptr<ComponentData>> &GetComponents();
			const std::unordered_map<std::string, std::string> &GetKeyValues() const;
			std::unordered_map<std::string, std::string> &GetKeyValues();
			std::optional<std::string> GetKeyValue(const std::string &key) const;
			std::string GetKeyValue(const std::string &key, const std::string &def) const;
			const std::vector<Output> &GetOutputs() const;
			std::vector<Output> &GetOutputs();
			const std::vector<uint16_t> &GetLeaves() const;
			std::vector<uint16_t> &GetLeaves();
			void GetLeafData(uint32_t &outFirstLeaf, uint32_t &outNumLeaves) const;

			const std::optional<math::ScaledTransform> &GetPose() const;
			math::ScaledTransform GetEffectivePose() const;
			void SetPose(const math::ScaledTransform &pose);
			void ClearPose();
		  private:
			friend WorldData;
			EntityData() = default;
			std::string m_className = "entity";
			std::unordered_map<std::string, std::shared_ptr<ComponentData>> m_components;
			std::unordered_map<std::string, std::string> m_keyValues;
			std::vector<Output> m_outputs;
			uint32_t m_mapIndex = 0u;
			std::optional<math::ScaledTransform> m_pose {};
			std::vector<uint16_t> m_leaves = {};
			Flags m_flags = Flags::None;

			uint32_t m_firstLeaf = 0u;
			uint32_t m_numLeaves = 0u;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::EntityData &entityData);
	REGISTER_ENUM_FLAGS(pragma::asset::EntityData::Flags)
};
