#ifndef __C_POINT_RENDERTARGET_H__
#define __C_POINT_RENDERTARGET_H__
#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseentity_handle.h>
#include "pragma/entities/point/point_rendertarget.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <texturemanager/texture.h>

struct Camera;
namespace pragma
{
	class DLLCLIENT CRenderTargetComponent final
		: public BasePointRenderTargetComponent,
		public CBaseNetComponent
	{
	public:
		CRenderTargetComponent(BaseEntity &ent) : BasePointRenderTargetComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		void SetRenderSize(Vector2 &size);
		void SetRenderSize(float w,float h);
		Vector2 GetRenderSize();
		void GetRenderSize(float *w,float *h);

		void SetRenderMaterial(Material *mat);
		Material *GetRenderMaterial();
		void SetRenderMaterial(std::string mat);

		void SetRefreshRate(float f);
		float GetRefreshRate();

		void SetRenderFOV(float fov);
		float GetRenderFOV();

		void SetRenderDepth(unsigned int depth);
		unsigned int GetRenderDepth();

		void Spawn();
		unsigned int GetTextureBuffer();
		void Render(RenderMode renderMode);

		std::unique_ptr<Texture> m_texture = nullptr;
		Material *m_matRender = nullptr;
		float m_refreshRate = 0.f;
		double m_tLastRefresh;
		unsigned int m_curDepth = 0u;
		std::unique_ptr<Camera> m_cam = nullptr;
	};
};

class DLLCLIENT CPointRenderTarget
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif