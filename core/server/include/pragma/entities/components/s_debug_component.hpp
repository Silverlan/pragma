#ifndef __S_DEBUG_COMPONENT_HPP__
#define __S_DEBUG_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_debug_component.hpp>

namespace pragma
{
	class DLLSERVER SDebugTextComponent final
		: public BaseDebugTextComponent,
		public SBaseNetComponent
	{
	public:
		SDebugTextComponent(BaseEntity &ent) : BaseDebugTextComponent(ent) {}
		virtual void SetText(const std::string &text) override;
		virtual void SetSize(float size) override;
		
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};

	////////////////

	class DLLSERVER SBaseDebugOutlineComponent
		: public SBaseNetComponent
	{
	public:
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		SBaseDebugOutlineComponent()=default;
	};

	////////////////
	
	class DLLSERVER SDebugPointComponent final
		: public BaseDebugPointComponent,
		public SBaseNetComponent
	{
	public:
		SDebugPointComponent(BaseEntity &ent) : BaseDebugPointComponent(ent) {}
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};

	////////////////

	class DLLSERVER SDebugLineComponent final
		: public BaseDebugLineComponent,
		public SBaseNetComponent
	{
	public:
		SDebugLineComponent(BaseEntity &ent) : BaseDebugLineComponent(ent) {}
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
	
	////////////////

	class DLLSERVER SDebugBoxComponent final
		: public BaseDebugBoxComponent,
		public SBaseDebugOutlineComponent
	{
	public:
		SDebugBoxComponent(BaseEntity &ent) : BaseDebugBoxComponent(ent) {}
		virtual void SetBounds(const Vector3 &min,const Vector3 &max) override;
		
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};

	////////////////
	
	class DLLSERVER SDebugSphereComponent final
		: public BaseDebugSphereComponent,
		public SBaseDebugOutlineComponent
	{
	public:
		SDebugSphereComponent(BaseEntity &ent) : BaseDebugSphereComponent(ent) {}
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};

	////////////////
	
	class DLLSERVER SDebugConeComponent final
		: public BaseDebugConeComponent,
		public SBaseDebugOutlineComponent
	{
	public:
		SDebugConeComponent(BaseEntity &ent) : BaseDebugConeComponent(ent) {}
		virtual void SetConeAngle(float angle) override;
		virtual void SetStartRadius(float radius) override;
		
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};

	////////////////
	
	class DLLSERVER SDebugCylinderComponent final
		: public BaseDebugCylinderComponent,
		public SBaseDebugOutlineComponent
	{
	public:
		SDebugCylinderComponent(BaseEntity &ent) : BaseDebugCylinderComponent(ent) {}
		virtual void SetLength(float length) override;
		
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};

	////////////////
	
	class DLLSERVER SDebugPlaneComponent final
		: public BaseDebugPlaneComponent
	{
	public:
		SDebugPlaneComponent(BaseEntity &ent) : BaseDebugPlaneComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
