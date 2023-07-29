//*********************************************************
//
// Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//*********************************************************

#pragma once

// See the following for more information:
// https://github.com/NVIDIA/nsight-aftermath-samples/tree/master/VkHelloNsightAftermath
// https://developer.nvidia.com/nsight-aftermath

// #define PRAGMA_ENABLE_NSIGHT_AFTERMATH
#ifdef PRAGMA_ENABLE_NSIGHT_AFTERMATH

// #define NSIGHT_ENABLE_SHADER_DATABASE

#include "pragma/clientdefinitions.h"
#include <map>
#include <mutex>
#include <iomanip>
#include <string>
#include <sstream>
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

//*********************************************************
// Some std::to_string overloads for some Nsight Aftermath
// API types.
//

namespace std {
	template<typename T>
	inline std::string to_hex_string(T n)
	{
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << n;
		return stream.str();
	}

	inline std::string to_string(GFSDK_Aftermath_Result result) { return std::string("0x") + to_hex_string(static_cast<uint32_t>(result)); }

	inline std::string to_string(const GFSDK_Aftermath_ShaderDebugInfoIdentifier &identifier) { return to_hex_string(identifier.id[0]) + "-" + to_hex_string(identifier.id[1]); }

	inline std::string to_string(const GFSDK_Aftermath_ShaderBinaryHash &hash) { return to_hex_string(hash.hash); }
} // namespace std

inline std::string AftermathErrorMessage(GFSDK_Aftermath_Result result)
{
	switch(result) {
	case GFSDK_Aftermath_Result_FAIL_DriverVersionNotSupported:
		return "Unsupported driver version - requires an NVIDIA R495 display driver or newer.";
	default:
		return "Aftermath Error 0x" + std::to_hex_string(result);
	}
}

// Helper macro for checking Nsight Aftermath results and throwing exception
// in case of a failure.
#ifdef _WIN32
#define AFTERMATH_CHECK_ERROR(FC)                                                                                                                                                                                                                                                                \
	[&]() {                                                                                                                                                                                                                                                                                      \
		GFSDK_Aftermath_Result _result = FC;                                                                                                                                                                                                                                                     \
		if(!GFSDK_Aftermath_SUCCEED(_result)) {                                                                                                                                                                                                                                                  \
			MessageBoxA(0, AftermathErrorMessage(_result).c_str(), "Aftermath Error", MB_OK);                                                                                                                                                                                                    \
			exit(1);                                                                                                                                                                                                                                                                             \
		}                                                                                                                                                                                                                                                                                        \
	}()
#else
#define AFTERMATH_CHECK_ERROR(FC)                                                                                                                                                                                                                                                                \
	[&]() {                                                                                                                                                                                                                                                                                      \
		GFSDK_Aftermath_Result _result = FC;                                                                                                                                                                                                                                                     \
		if(!GFSDK_Aftermath_SUCCEED(_result)) {                                                                                                                                                                                                                                                  \
			printf("%s\n", AftermathErrorMessage(_result).c_str());                                                                                                                                                                                                                              \
			fflush(stdout);                                                                                                                                                                                                                                                                      \
			exit(1);                                                                                                                                                                                                                                                                             \
		}                                                                                                                                                                                                                                                                                        \
	}()
#endif

// Helper for comparing GFSDK_Aftermath_ShaderDebugInfoIdentifier.
inline bool operator<(const GFSDK_Aftermath_ShaderDebugInfoIdentifier &lhs, const GFSDK_Aftermath_ShaderDebugInfoIdentifier &rhs)
{
	if(lhs.id[0] == rhs.id[0]) {
		return lhs.id[1] < rhs.id[1];
	}
	return lhs.id[0] < rhs.id[0];
}

//*********************************************************
// Implements GPU crash dump tracking using the Nsight
// Aftermath API.
//
class GpuCrashTracker {
  public:
	// keep four frames worth of marker history
	const static unsigned int c_markerFrameHistory = 4;
	typedef std::array<std::map<uint64_t, std::string>, c_markerFrameHistory> MarkerMap;

	GpuCrashTracker(const MarkerMap &markerMap);
	~GpuCrashTracker();

	// Initialize the GPU crash dump tracker.
	void Initialize();
  private:
	//*********************************************************
	// Callback handlers for GPU crash dumps and related data.
	//

	// Handler for GPU crash dump callbacks.
	void OnCrashDump(const void *pGpuCrashDump, const uint32_t gpuCrashDumpSize);

	// Handler for shader debug information callbacks.
	void OnShaderDebugInfo(const void *pShaderDebugInfo, const uint32_t shaderDebugInfoSize);

	// Handler for GPU crash dump description callbacks.
	void OnDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription);

	// Handler for app-managed marker resolve callback
	void OnResolveMarker(const void *pMarker, void **resolvedMarkerData, uint32_t *markerSize);

	//*********************************************************
	// Helpers for writing a GPU crash dump and debug information
	// data to files.
	//

	// Helper for writing a GPU crash dump to a file.
	void WriteGpuCrashDumpToFile(const void *pGpuCrashDump, const uint32_t gpuCrashDumpSize);

	// Helper for writing shader debug information to a file
	void WriteShaderDebugInformationToFile(GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier, const void *pShaderDebugInfo, const uint32_t shaderDebugInfoSize);

	//*********************************************************
	// Helpers for decoding GPU crash dump to JSON.
	//

	// Handler for shader debug info lookup callbacks.
	void OnShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier &identifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const;

	// Handler for shader lookup callbacks.
	void OnShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash &shaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) const;

	// Handler for shader source debug info lookup callbacks.
	void OnShaderSourceDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugName &shaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary) const;

	//*********************************************************
	// Static callback wrappers.
	//

	// GPU crash dump callback.
	static void GpuCrashDumpCallback(const void *pGpuCrashDump, const uint32_t gpuCrashDumpSize, void *pUserData);

	// Shader debug information callback.
	static void ShaderDebugInfoCallback(const void *pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void *pUserData);

	// GPU crash dump description callback.
	static void CrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void *pUserData);

	// App-managed marker resolve callback
	static void ResolveMarkerCallback(const void *pMarker, void *pUserData, void **resolvedMarkerData, uint32_t *markerSize);

	// Shader debug information lookup callback.
	static void ShaderDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier *pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void *pUserData);

	// Shader lookup callback.
	static void ShaderLookupCallback(const GFSDK_Aftermath_ShaderBinaryHash *pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void *pUserData);

	// Shader source debug info lookup callback.
	static void ShaderSourceDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugName *pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void *pUserData);

	//*********************************************************
	// GPU crash tracker state.
	//

	// Is the GPU crash dump tracker initialized?
	bool m_initialized;

	// For thread-safe access of GPU crash tracker state.
	mutable std::mutex m_mutex;

	// List of Shader Debug Information by ShaderDebugInfoIdentifier.
	std::map<GFSDK_Aftermath_ShaderDebugInfoIdentifier, std::vector<uint8_t>> m_shaderDebugInfo;

	// The mock shader database.
#ifdef NSIGHT_ENABLE_SHADER_DATABASE
	ShaderDatabase m_shaderDatabase;
#endif

	// App-managed marker tracking
	const MarkerMap &m_markerMap;
};
#endif
