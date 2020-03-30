#include "stdafx_client.h"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lentity.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <sharedutils/netpacket.hpp>
#include <networkmanager/interface/nwm_manager.hpp>

#include <misc/image_create_info.h>
#include <wgui/types/wirect.h>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
static Anvil::QueueFamilyFlags queue_family_flags_to_anvil_queue_family(prosper::util::QueueFamilyFlags flags)
{
	Anvil::QueueFamilyFlags queueFamilies {};
	if((flags &prosper::util::QueueFamilyFlags::Graphics) != prosper::util::QueueFamilyFlags::None)
		queueFamilies = queueFamilies | Anvil::QueueFamilyFlagBits::GRAPHICS_BIT;
	if((flags &prosper::util::QueueFamilyFlags::Compute) != prosper::util::QueueFamilyFlags::None)
		queueFamilies = queueFamilies | Anvil::QueueFamilyFlagBits::COMPUTE_BIT;
	if((flags &prosper::util::QueueFamilyFlags::DMA) != prosper::util::QueueFamilyFlags::None)
		queueFamilies = queueFamilies | Anvil::QueueFamilyFlagBits::DMA_BIT;
	return queueFamilies;
}
static void test()
{
	auto &dev = c_engine->GetDevice();
	uint32_t width = 512;
	uint32_t height = 512;
	auto sharingMode = Anvil::SharingMode::EXCLUSIVE;
	auto queueFamilies = queue_family_flags_to_anvil_queue_family(prosper::util::QueueFamilyFlags::Graphics);

	std::vector<uint8_t> data;
	data.resize(width *height *sizeof(uint8_t) *4);
	for(auto i=0;i<data.size();i+=4)
	{
		data.at(i) = 255;
		data.at(i +1) = 0;
		data.at(i +2) = 0;
		data.at(i +3) = 255;
	}

	Anvil::ImageCreateFlags flags {};
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = data.size();
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	//static auto buf = prosper::util::create_buffer(dev,createInfo,data.data());

	auto maxTotalSize = createInfo.size *100;
	static auto testBuf = prosper::util::create_dynamic_resizable_buffer(*c_engine,createInfo,maxTotalSize);
	testBuf->SetPermanentlyMapped(true);
	testBuf->AllocateBuffer(100);
	auto buf = testBuf->AllocateBuffer(createInfo.size,data.data());
	buf->Write(0,data.data());

#if 0
	DLLPROSPER std::shared_ptr<DynamicResizableBuffer> create_dynamic_resizable_buffer(
		Context &context,BufferCreateInfo createInfo,
		uint64_t maxTotalSize,float clampSizeToAvailableGPUMemoryPercentage=1.f,const void *data=nullptr
	);
#endif

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	imgCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	static auto img = prosper::util::create_image(dev,imgCreateInfo,buf);


	prosper::util::TextureCreateInfo texCreateInfo {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	static auto tex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);
	
	auto *el = WGUI::GetInstance().Create<WITexturedRect>();
	el->SetTexture(*tex);
	el->SetSize(512,512);
	el->SetZPos(100000);
#if 0
	if((createInfo.flags &prosper::util::ImageCreateInfo::Flags::Sparse) != prosper::util::ImageCreateInfo::Flags::None)
	{
		if((createInfo.flags &prosper::util::ImageCreateInfo::Flags::SparseAliasedResidency) != prosper::util::ImageCreateInfo::Flags::None)
			imageCreateFlags |= Anvil::ImageCreateFlagBits::SPARSE_ALIASED_BIT | Anvil::ImageCreateFlagBits::SPARSE_RESIDENCY_BIT;
		;
		return prosper::Image::Create(prosper::Context::GetContext(dev),Anvil::Image::create(
			Anvil::ImageCreateInfo::create_no_alloc(
				&dev,createInfo.type,createInfo.format,
				createInfo.tiling,createInfo.usage,
				createInfo.width,createInfo.height,1u,layers,
				createInfo.samples,queueFamilies,
				sharingMode,bUseFullMipmapChain,imageCreateFlags
			)
		));
	}

	return prosper::Image::Create(prosper::Context::GetContext(dev),Anvil::Image::create(
		Anvil::ImageCreateInfo::create_alloc(
			&dev,createInfo.type,createInfo.format,
			createInfo.tiling,createInfo.usage,
			createInfo.width,createInfo.height,1u,layers,
			createInfo.samples,queueFamilies,
			sharingMode,bUseFullMipmapChain,memoryFeatureFlags,imageCreateFlags,
			postCreateLayout,data
		)
	));
#endif
}

void Lua::Entity::Client::register_class(luabind::class_<EntityHandle> &classDef)
{
	::Lua::Entity::register_class(classDef);
	classDef.def("IsClientsideOnly",&IsClientsideOnly);
	classDef.def("GetClientIndex",&GetClientIndex);
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t protocol,unsigned int,const NetPacket&)>(&SendNetEvent));
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t protocol,unsigned int)>(&SendNetEvent));
}

void Lua::Entity::Client::IsClientsideOnly(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,static_cast<CBaseEntity*>(hEnt.get())->IsClientsideOnly());

	test();
}

void Lua::Entity::Client::GetClientIndex(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushInt(l,static_cast<CBaseEntity*>(hEnt.get())->GetClientIndex());
}
void Lua::Entity::Client::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,const NetPacket &packet)
{
	LUA_CHECK_ENTITY(l,hEnt);
	switch(static_cast<nwm::Protocol>(protocol))
	{
		case nwm::Protocol::TCP:
			static_cast<CBaseEntity*>(hEnt.get())->SendNetEventTCP(eventId,const_cast<NetPacket&>(packet));
			break;
		case nwm::Protocol::UDP:
			static_cast<CBaseEntity*>(hEnt.get())->SendNetEventUDP(eventId,const_cast<NetPacket&>(packet));
			break;
	}
}
void Lua::Entity::Client::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId) {SendNetEvent(l,hEnt,protocol,eventId,{});}
#pragma optimize("",on)
