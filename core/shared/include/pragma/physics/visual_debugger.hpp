#ifndef __C_PHYSDEBUG_H__
#define __C_PHYSDEBUG_H__

#include <pragma/networkdefinitions.h>
#include <pragma/physics/physapi.h>
#include <vector>
#include <mathutil/glmutil.h>
#include <mathutil/color.h>
#include <memory>
#include <sharedutils/functioncallback.h>
#include <sharedutils/util_shared_handle.hpp>

namespace pragma {class CCameraComponent;};
namespace pragma::physics
{
	class IBase;
	class ICollisionObject;
	class IConstraint;
	class DLLNETWORK VisualDebugObject
	{
	public:
		VisualDebugObject(pragma::physics::ICollisionObject &o);
		VisualDebugObject(pragma::physics::IConstraint &c);
		~VisualDebugObject();
		util::TWeakSharedHandle<pragma::physics::IBase> hObject = {};
		bool constraint = false;
		std::vector<Vector3> lines;
		std::vector<Vector4> colors;
		bool dynamic = false;
		//std::shared_ptr<prosper::Buffer> lineBuffer = nullptr;
		//std::shared_ptr<prosper::Buffer> colorBuffer = nullptr;
		//void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam);
		void UpdateBuffer();
	private:
		VisualDebugObject();
		uint32_t m_vertexCount = 0u;
		//std::shared_ptr<prosper::Buffer> m_instanceBuffer = nullptr;
	};

	class DLLNETWORK IVisualDebugger
	{
	public:
		enum class DebugMode : uint32_t
		{
			None = 0,
			Wireframe,
			Constraints,
			Normals,
			All
		};
		/* // Bullet: 
		auto mode = btIDebugDraw::DBG_DrawAabb |
			btIDebugDraw::DBG_DrawConstraintLimits |
			btIDebugDraw::DBG_DrawConstraints |
			btIDebugDraw::DBG_DrawContactPoints |
			btIDebugDraw::DBG_DrawFeaturesText |
			btIDebugDraw::DBG_DrawFrames |
			btIDebugDraw::DBG_DrawNormals |
			btIDebugDraw::DBG_DrawText |
			btIDebugDraw::DBG_DrawWireframe |
			btIDebugDraw::DBG_EnableCCD;
		if(val == 2)
			mode = btIDebugDraw::DBG_DrawWireframe;
		else if(val == 3)
			mode = btIDebugDraw::DBG_DrawConstraints;
		else if(val == 4)
			mode = btIDebugDraw::DBG_DrawNormals;
			*/
		IVisualDebugger();
		virtual ~IVisualDebugger();
		//void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam);
		IVisualDebugger(IVisualDebugger&)=delete;
		IVisualDebugger &operator=(const IVisualDebugger&)=delete;

		void DrawLine(const Vector3 &from,const Vector3 &to,const Color &color);
		void DrawLine(const Vector3 &from,const Vector3 &to,const Color &fromColor,const Color &toColor);
		void DrawContactPoint(const Vector3 &PointOnB,const Vector3 &normalOnB,float distance,int lifeTime,const Color &color);
		void ReportErrorWarning(const std::string &str);
		void Draw3DText(const Vector3 &location,const std::string &str);

		void SetDebugMode(DebugMode debugMode);
	protected:
		void Draw();
		void Clear();
	private:
		std::vector<std::unique_ptr<VisualDebugObject>> m_objects;
		VisualDebugObject *m_drawObject;
		CallbackHandle m_cbCollisionObject;
		CallbackHandle m_cbConstraint;
		CallbackHandle m_cbController;
		DebugMode m_debugMode = DebugMode::None;
		void Draw(VisualDebugObject *o);

		void Draw(pragma::physics::IBase *b);
	};
};

#endif
