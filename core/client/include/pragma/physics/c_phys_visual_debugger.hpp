#if 0
#ifndef __C_PHYS_VISUAL_DEBUGGER_HPP__
#define __C_PHYS_VISUAL_DEBUGGER_HPP__

#include "pragma/clientdefinitions.h"
#include <memory>

namespace prosper
{
	class Buffer;
	class PrimaryCommandBuffer;
};

namespace pragma
{
	class CCameraComponent;
};

class DLLCLIENT CPhysVisualDebuggerObject
{
public:
	std::shared_ptr<prosper::Buffer> lineBuffer = nullptr;
	std::shared_ptr<prosper::Buffer> colorBuffer = nullptr;
	void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam);
	void UpdateBuffer();
private:
	std::shared_ptr<prosper::Buffer> m_instanceBuffer = nullptr;
};

class DLLCLIENT CPhysVisualDebugger
{
public:
	CPhysVisualDebugger();
	void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam);
};

#endif

#endif
