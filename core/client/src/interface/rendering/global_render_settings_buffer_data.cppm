// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

// #define PRAGMA_ENABLE_SHADER_DEBUG_PRINT

export module pragma.client:rendering.global_render_settings_buffer_data;

export import pragma.prosper;
export import pragma.udm;
export import std.compat;

export namespace pragma::rendering {
	// These have to match the enums in shaders/debug/debug_print.glsl
	enum class GlslType : uint32_t {
		Int32 = 0,
		UInt32,

		Float,
		Boolean,

		Vector2,
		Vector3,
		Vector4,
		Mat4,

		Vector2i,
		Vector3i,
		Vector4i,

		NotSet,

		Count,
		Last = Count - 1,
		Invalid = std::numeric_limits<uint32_t>::max()
	};
	constexpr udm::Type glsl_type_to_udm(GlslType type)
	{
		switch(type) {
		case GlslType::Int32:
			return udm::Type::Int32;
		case GlslType::UInt32:
			return udm::Type::UInt32;
		case GlslType::Float:
			return udm::Type::Float;
		case GlslType::Boolean:
			return udm::Type::Boolean;
		case GlslType::Vector2:
			return udm::Type::Vector2;
		case GlslType::Vector3:
			return udm::Type::Vector3;
		case GlslType::Vector4:
			return udm::Type::Vector4;
		case GlslType::Mat4:
			return udm::Type::Mat4;
		case GlslType::Vector2i:
			return udm::Type::Vector2i;
		case GlslType::Vector3i:
			return udm::Type::Vector3i;
		case GlslType::Vector4i:
			return udm::Type::Vector4i;
		default:
			return udm::Type::Invalid;
		}
	}

	template<typename T>
	concept is_glsl_type = std::is_same_v<T, float> || std::is_same_v<T, bool> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, Mat4> || std::is_same_v<T, Vector4> || std::is_same_v<T, Vector3> || std::is_same_v<T, Vector2> || std::is_same_v<T, Vector4i>
	  || std::is_same_v<T, Vector3i> || std::is_same_v<T, Vector2i>;
	struct GlobalRenderSettingsBufferData {
#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
#pragma pack(push, 1)
		struct DebugPrintData {
			uint8_t data[sizeof(Mat4)];
			GlslType type = GlslType::NotSet;
		};
#pragma pack(pop)
#endif

		GlobalRenderSettingsBufferData();
		std::shared_ptr<prosper::IBuffer> debugBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> timeBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> csmBuffer = nullptr;
#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
		std::shared_ptr<prosper::IBuffer> debugPrintBuffer = nullptr;
		void EvaluateDebugPrint();
		std::optional<std::string> GetDebugPrintString() const;
		void ResetDebugPrintData();
#endif
		std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroup = nullptr;
	};
};
