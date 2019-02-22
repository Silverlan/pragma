#ifndef __C_PHYSDEBUG_H__
#define __C_PHYSDEBUG_H__

#include "pragma/clientdefinitions.h"
#include <pragma/networkdefinitions.h>
#include <pragma/physics/physapi.h>
#include <vector>
#include <mathutil/glmutil.h>
#include <memory>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <sharedutils/functioncallback.h>

class PhysCollisionObject;
class PhysConstraint;
class PhysBase;
class PhysHandle;
class DLLCLIENT WVBtIDebugObject final
{
public:
	WVBtIDebugObject(PhysCollisionObject *o);
	WVBtIDebugObject(PhysConstraint *c);
	~WVBtIDebugObject();
	PhysHandle hObject = {};
	bool constraint = false;
	std::vector<Vector3> lines;
	std::vector<Vector4> colors;
	bool dynamic = false;
	std::shared_ptr<prosper::Buffer> lineBuffer = nullptr;
	std::shared_ptr<prosper::Buffer> colorBuffer = nullptr;
	void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Camera &cam);
	void UpdateBuffer();
private:
	WVBtIDebugObject();
	uint32_t m_vertexCount = 0u;
	std::shared_ptr<prosper::Buffer> m_instanceBuffer = nullptr;
};

class DLLCLIENT WVBtIDebugDraw
	: public btIDebugDraw
{
private:
	int m_debugMode;
	std::vector<std::unique_ptr<WVBtIDebugObject>> m_objects;
	WVBtIDebugObject *m_drawObject;
	CallbackHandle m_cbCollisionObject;
	CallbackHandle m_cbConstraint;
	CallbackHandle m_cbController;
	void Draw(WVBtIDebugObject *o);

	void Draw(PhysBase *b);
	void Draw();
	void Clear();
public:
	WVBtIDebugDraw();
	virtual ~WVBtIDebugDraw() override;
	void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Camera &cam);
	virtual void drawLine(const btVector3 &from,const btVector3 &to,const btVector3 &color) override;
	virtual void drawLine(const btVector3 &from,const btVector3 &to,const btVector3 &fromColor,const btVector3 &toColor) override;
	virtual void drawContactPoint(const btVector3 &PointOnB,const btVector3 &normalOnB,btScalar distance,int lifeTime,const btVector3 &color) override;
	virtual void reportErrorWarning(const char *warningString) override;
	virtual void draw3dText(const btVector3 &location,const char *textString) override;
	virtual void setDebugMode(int debugMode) override;
	virtual int getDebugMode() const override;
	WVBtIDebugDraw(WVBtIDebugDraw&)=delete;
	WVBtIDebugDraw &operator=(const WVBtIDebugDraw&)=delete;
};

#endif