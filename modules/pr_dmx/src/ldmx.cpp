#include "ldmx.hpp"
#include <util_dmx.hpp>
#include <luainterface.hpp>
#include <luasystem.h>
#include <pragma/lua/libraries/lfile.h>
#include <mathutil/color.h>
#include <sstream>
#include <unordered_map>
#include <sharedutils/util.h>

#pragma optimize("",off)
static void print_attribute(std::stringstream &ss,::dmx::Attribute &attr,const std::string &t,std::unordered_map<::dmx::Element*,bool> &iteratedEls,std::unordered_map<::dmx::Attribute*,bool> &iteratedAttrs);
static void print_element(std::stringstream &ss,::dmx::Element &e,const std::string &t,std::unordered_map<::dmx::Element*,bool> &iteratedEls,std::unordered_map<::dmx::Attribute*,bool> &iteratedAttrs)
{
	auto it = iteratedEls.find(&e);
	if(it != iteratedEls.end())
		return;
	iteratedEls[&e] = true;

	auto &attrs = e.attributes;
	ss<<t<<e.name<<" ("<<e.type<<", "<<attrs.size()<<" attributes)";
	for(auto &pair : attrs)
	{
		ss<<"\n\t"<<t<<pair.first;
		print_attribute(ss,*pair.second,t +"\t\t",iteratedEls,iteratedAttrs);
	}
}

static void print_attribute(std::stringstream &ss,::dmx::Attribute &attr,const std::string &t,std::unordered_map<::dmx::Element*,bool> &iteratedEls,std::unordered_map<::dmx::Attribute*,bool> &iteratedAttrs)
{
	auto it = iteratedAttrs.find(&attr);
	if(it != iteratedAttrs.end())
		return;
	iteratedAttrs[&attr] = true;

	ss<<" ("<<::dmx::type_to_string(attr.type)<<"):\n";
	if(attr.data == nullptr)
	{
		ss<<t<<"NULL";
		return;
	}
	switch(attr.type)
	{
		case ::dmx::AttrType::Element:
		{
			auto &wpElement = *static_cast<std::weak_ptr<::dmx::Element>*>(attr.data.get());
			if(wpElement.expired())
				return;
			print_element(ss,*wpElement.lock(),t +"\t",iteratedEls,iteratedAttrs);
			break;
		}
		case ::dmx::AttrType::String:
			ss<<t<<*static_cast<std::string*>(attr.data.get());
			break;
		case ::dmx::AttrType::Int:
			ss<<t<<*static_cast<int32_t*>(attr.data.get());
			break;
		case ::dmx::AttrType::Float:
			ss<<t<<*static_cast<float*>(attr.data.get());
			break;
		case ::dmx::AttrType::Bool:
			ss<<t<<*static_cast<bool*>(attr.data.get());
			break;
		case ::dmx::AttrType::Vector2:
			ss<<t<<*static_cast<Vector2*>(attr.data.get());
			break;
		case ::dmx::AttrType::Vector3:
			ss<<t<<*static_cast<Vector3*>(attr.data.get());
			break;
		case ::dmx::AttrType::Angle:
			ss<<t<<*static_cast<EulerAngles*>(attr.data.get());
			break;
		case ::dmx::AttrType::Vector4:
			ss<<t<<*static_cast<Vector4*>(attr.data.get());
			break;
		case ::dmx::AttrType::Quaternion:
			ss<<t<<*static_cast<Quat*>(attr.data.get());
			break;
		case ::dmx::AttrType::Matrix:
			ss<<t<<*static_cast<Mat4*>(attr.data.get());
			break;
		case ::dmx::AttrType::Color:
		{
			auto &c = *static_cast<std::array<uint8_t,4>*>(attr.data.get());
			ss<<t<<+c.at(0)<<" "<<+c.at(1)<<" "<<+c.at(2)<<" "<<+c.at(3);
			break;
		}
		case ::dmx::AttrType::Time:
			ss<<t<<*static_cast<float*>(attr.data.get());
			break;
		case ::dmx::AttrType::Binary:
		{
			auto &data = *static_cast<std::vector<uint8_t>*>(attr.data.get());
			auto len = data.size();
			ss<<t<<"Binary ("<<len<<" bytes)";
			break;
		}
		default:
		{
			if(attr.type >= ::dmx::AttrType::ArrayFirst && attr.type <= ::dmx::AttrType::ArrayLast)
			{
				auto &vdata = *static_cast<std::vector<std::shared_ptr<::dmx::Attribute>>*>(attr.data.get());
				ss<<t<<"Array ["<<::dmx::type_to_string(attr.type)<<"]["<<vdata.size()<<" elements]";
				//ss<<t<<"Array of type "<<::dmx::type_to_string(::dmx::get_single_type(attr.type))<<" ["<<vdata.size()<<" elements]";
				//for(auto &subAttr : vdata)
				//	print_attribute(ss,*subAttr,t +"\t",iteratedEls,iteratedAttrs);
			}
			break;
		}
	}
}

static bool push_attribute_value(lua_State *l,::dmx::Attribute &attr)
{
	if(attr.data == nullptr)
		return false;
	switch(attr.type)
	{
		case ::dmx::AttrType::Element:
		{
			auto &wpElement = *static_cast<std::weak_ptr<::dmx::Element>*>(attr.data.get());
			if(wpElement.expired())
				return false;
			Lua::Push<std::shared_ptr<::dmx::Element>>(l,wpElement.lock());
			break;
		}
		case ::dmx::AttrType::String:
			Lua::PushString(l,*static_cast<std::string*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Int:
			Lua::PushInt(l,*static_cast<int32_t*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Float:
			Lua::PushNumber(l,*static_cast<float*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Bool:
			Lua::PushBool(l,*static_cast<bool*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Vector2:
			Lua::Push<Vector2>(l,*static_cast<Vector2*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Vector3:
			Lua::Push<Vector3>(l,*static_cast<Vector3*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Angle:
			Lua::Push<EulerAngles>(l,*static_cast<EulerAngles*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Vector4:
			Lua::Push<Vector4>(l,*static_cast<Vector4*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Quaternion:
			Lua::Push<Quat>(l,*static_cast<Quat*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Matrix:
			Lua::Push<Mat4>(l,*static_cast<Mat4*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Color:
		{
			auto &c = *static_cast<std::array<uint8_t,4>*>(attr.data.get());
			Lua::Push<Color>(l,Color(c.at(0),c.at(1),c.at(2),c.at(3)));
			break;
		}
		case ::dmx::AttrType::Time:
			Lua::PushNumber(l,*static_cast<float*>(attr.data.get()));
			break;
		case ::dmx::AttrType::Binary:
		{
			auto &data = *static_cast<std::vector<uint8_t>*>(attr.data.get());
			auto len = data.size();
			DataStream ds(len);
			ds->Write(data.data(),data.size());
			Lua::Push<DataStream>(l,ds);
			break;
		}
		default:
		{
			if(attr.type >= ::dmx::AttrType::ArrayFirst && attr.type <= ::dmx::AttrType::ArrayLast)
			{
				auto &vdata = *static_cast<std::vector<std::shared_ptr<::dmx::Attribute>>*>(attr.data.get());
				auto t = Lua::CreateTable(l);
				auto idx = 1u;
				for(auto &subAttr : vdata)
				{
					Lua::PushInt(l,idx++);
					Lua::Push<std::shared_ptr<::dmx::Attribute>>(l,subAttr);
					Lua::SetTableValue(l,t);
				}
			}
			break;
		}
	}
	return true;
}

void Lua::dmx::register_lua_library(Lua::Interface &l)
{
	Lua::RegisterLibrary(l.GetState(),"dmx",{
		{"load",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto &f = *Lua::CheckFile(l,1);
			auto hFile = f.GetHandle();

			std::shared_ptr<::dmx::FileData> fd = nullptr;
			try
			{
				fd = ::dmx::FileData::Load(hFile);
			}
			catch(const std::runtime_error &e)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,e.what());
				return 2;
			}
			catch(const std::logic_error &e)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,e.what());
				return 2;
			}
			if(fd == nullptr)
			{
				Lua::PushBool(l,false);
				return 1;
			}
			Lua::Push<decltype(fd)>(l,fd);
			return 1;
		})},
		{"type_to_string",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto type = Lua::CheckInt(l,1);
			Lua::PushString(l,::dmx::type_to_string(static_cast<::dmx::AttrType>(type)));
			return 1;
		})},
	});

	auto &modDMX = l.RegisterLibrary("dmx");
	auto classDefData = luabind::class_<::dmx::FileData>("Data");
	classDefData.def("__tostring",static_cast<void(*)(lua_State*,::dmx::FileData&)>([](lua_State *l,::dmx::FileData &data) {
		std::stringstream ss;
		data.DebugPrint(ss);
		Lua::PushString(l,ss.str());
	}));
	classDefData.def("GetElements",static_cast<void(*)(lua_State*,::dmx::FileData&)>([](lua_State *l,::dmx::FileData &data) {
		auto &elements = data.GetElements();
		auto t = Lua::CreateTable(l);
		auto elIdx = 1u;
		for(auto &el : elements)
		{
			Lua::PushInt(l,elIdx++);
			Lua::Push<std::shared_ptr<::dmx::Element>>(l,el);
			Lua::SetTableValue(l,t);
		}
	}));
	classDefData.def("GetRootAttribute",static_cast<void(*)(lua_State*,::dmx::FileData&)>([](lua_State *l,::dmx::FileData &data) {
		auto &attr = data.GetRootAttribute();
		Lua::Push<std::shared_ptr<::dmx::Attribute>>(l,attr);
	}));
	modDMX[classDefData];

	auto classDefElement = luabind::class_<::dmx::Element>("Element");
	classDefElement.def("__tostring",static_cast<void(*)(lua_State*,::dmx::Element&)>([](lua_State *l,::dmx::Element &el) {
		std::stringstream ss;
		el.DebugPrint(ss);
		Lua::PushString(l,ss.str());
	}));
	classDefElement.def("Get",static_cast<void(*)(lua_State*,::dmx::Element&,const std::string&)>([](lua_State *l,::dmx::Element &el,const std::string &name) {
		auto child = el.Get(name);
		if(child == nullptr)
			return;
		Lua::Push(l,child);
	}));
	classDefElement.def("GetAttr",static_cast<void(*)(lua_State*,::dmx::Element&,const std::string&)>([](lua_State *l,::dmx::Element &el,const std::string &name) {
		auto attr = el.GetAttr(name);
		if(attr == nullptr)
			return;
		Lua::Push(l,attr);
	}));
	classDefElement.def("GetAttrV",static_cast<void(*)(lua_State*,::dmx::Element&,const std::string&)>([](lua_State *l,::dmx::Element &el,const std::string &name) {
		auto attr = el.GetAttr(name);
		if(attr == nullptr)
			return;
		push_attribute_value(l,*attr);
	}));
	classDefElement.def("GetName",static_cast<void(*)(lua_State*,::dmx::Element&)>([](lua_State *l,::dmx::Element &el) {
		Lua::PushString(l,el.name);
	}));
	classDefElement.def("GetType",static_cast<void(*)(lua_State*,::dmx::Element&)>([](lua_State *l,::dmx::Element &el) {
		Lua::PushString(l,el.type);
	}));
	classDefElement.def("GetAttributes",static_cast<void(*)(lua_State*,::dmx::Element&)>([](lua_State *l,::dmx::Element &el) {
		auto t = Lua::CreateTable(l);
		auto attrId = 1u;
		for(auto &pair : el.attributes)
		{
			auto &attr = pair.second;
			Lua::PushString(l,pair.first);
			Lua::Push<std::shared_ptr<::dmx::Attribute>>(l,attr);
			Lua::SetTableValue(l,t);
		}
	}));
	classDefElement.def("GetAttribute",static_cast<void(*)(lua_State*,::dmx::Element&,const std::string&)>([](lua_State *l,::dmx::Element &el,const std::string &id) {
		auto it = el.attributes.find(id);
		if(it == el.attributes.end())
			return;
		Lua::Push<std::shared_ptr<::dmx::Attribute>>(l,it->second);
	}));
	classDefElement.def("GetAttributeValue",static_cast<void(*)(lua_State*,::dmx::Element&,const std::string&)>([](lua_State *l,::dmx::Element &el,const std::string &id) {
		auto it = el.attributes.find(id);
		if(it == el.attributes.end() || push_attribute_value(l,*it->second) == false)
			return;
	}));
	modDMX[classDefElement];

	auto classDefAttribute = luabind::class_<::dmx::Attribute>("Attribute");
	classDefAttribute.add_static_constant("TYPE_NONE",umath::to_integral(::dmx::AttrType::None));
	classDefAttribute.add_static_constant("TYPE_ELEMENT",umath::to_integral(::dmx::AttrType::Element));
	classDefAttribute.add_static_constant("TYPE_INT",umath::to_integral(::dmx::AttrType::Int));
	classDefAttribute.add_static_constant("TYPE_FLOAT",umath::to_integral(::dmx::AttrType::Float));
	classDefAttribute.add_static_constant("TYPE_BOOL",umath::to_integral(::dmx::AttrType::Bool));
	classDefAttribute.add_static_constant("TYPE_STRING",umath::to_integral(::dmx::AttrType::String));
	classDefAttribute.add_static_constant("TYPE_BINARY",umath::to_integral(::dmx::AttrType::Binary));
	classDefAttribute.add_static_constant("TYPE_TIME",umath::to_integral(::dmx::AttrType::Time));
	classDefAttribute.add_static_constant("TYPE_OBJECTID",umath::to_integral(::dmx::AttrType::ObjectId));
	classDefAttribute.add_static_constant("TYPE_COLOR",umath::to_integral(::dmx::AttrType::Color));
	classDefAttribute.add_static_constant("TYPE_VECTOR2",umath::to_integral(::dmx::AttrType::Vector2));
	classDefAttribute.add_static_constant("TYPE_VECTOR3",umath::to_integral(::dmx::AttrType::Vector3));
	classDefAttribute.add_static_constant("TYPE_VECTOR4",umath::to_integral(::dmx::AttrType::Vector4));
	classDefAttribute.add_static_constant("TYPE_ANGLE",umath::to_integral(::dmx::AttrType::Angle));
	classDefAttribute.add_static_constant("TYPE_QUATERNION",umath::to_integral(::dmx::AttrType::Quaternion));
	classDefAttribute.add_static_constant("TYPE_MATRIX",umath::to_integral(::dmx::AttrType::Matrix));
	classDefAttribute.add_static_constant("TYPE_UINT64",umath::to_integral(::dmx::AttrType::UInt64));
	classDefAttribute.add_static_constant("TYPE_UINT8",umath::to_integral(::dmx::AttrType::UInt8));

	classDefAttribute.add_static_constant("TYPE_ELEMENT_ARRAY",umath::to_integral(::dmx::AttrType::ElementArray));
	classDefAttribute.add_static_constant("TYPE_INT_ARRAY",umath::to_integral(::dmx::AttrType::IntArray));
	classDefAttribute.add_static_constant("TYPE_FLOAT_ARRAY",umath::to_integral(::dmx::AttrType::FloatArray));
	classDefAttribute.add_static_constant("TYPE_BOOL_ARRAY",umath::to_integral(::dmx::AttrType::BoolArray));
	classDefAttribute.add_static_constant("TYPE_STRING_ARRAY",umath::to_integral(::dmx::AttrType::StringArray));
	classDefAttribute.add_static_constant("TYPE_BINARY_ARRAY",umath::to_integral(::dmx::AttrType::BinaryArray));
	classDefAttribute.add_static_constant("TYPE_TIME_ARRAY",umath::to_integral(::dmx::AttrType::TimeArray));
	classDefAttribute.add_static_constant("TYPE_OBJECTID_ARRAY",umath::to_integral(::dmx::AttrType::ObjectIdArray));
	classDefAttribute.add_static_constant("TYPE_COLOR_ARRAY",umath::to_integral(::dmx::AttrType::ColorArray));
	classDefAttribute.add_static_constant("TYPE_VECTOR2_ARRAY",umath::to_integral(::dmx::AttrType::Vector2Array));
	classDefAttribute.add_static_constant("TYPE_VECTOR3_ARRAY",umath::to_integral(::dmx::AttrType::Vector3Array));
	classDefAttribute.add_static_constant("TYPE_VECTOR4_ARRAY",umath::to_integral(::dmx::AttrType::Vector4Array));
	classDefAttribute.add_static_constant("TYPE_ANGLE_ARRAY",umath::to_integral(::dmx::AttrType::AngleArray));
	classDefAttribute.add_static_constant("TYPE_QUATERNION_ARRAY",umath::to_integral(::dmx::AttrType::QuaternionArray));
	classDefAttribute.add_static_constant("TYPE_MATRIX_ARRAY",umath::to_integral(::dmx::AttrType::MatrixArray));

	classDefAttribute.add_static_constant("TYPE_INVALID",umath::to_integral(::dmx::AttrType::Invalid));

	classDefAttribute.def("__tostring",static_cast<void(*)(lua_State*,::dmx::Attribute&)>([](lua_State *l,::dmx::Attribute &attr) {
		std::stringstream ss;
		attr.DebugPrint(ss);
		Lua::PushString(l,ss.str());
	}));
	classDefAttribute.def("GetType",static_cast<void(*)(lua_State*,::dmx::Attribute&)>([](lua_State *l,::dmx::Attribute &attr) {
		Lua::PushInt(l,umath::to_integral(attr.type));
	}));
	classDefAttribute.def("Get",static_cast<void(*)(lua_State*,::dmx::Attribute&,const std::string&)>([](lua_State *l,::dmx::Attribute &el,const std::string &name) {
		auto child = el.Get(name);
		if(child == nullptr)
			return;
		Lua::Push(l,child);
	}));
	classDefAttribute.def("IsValid",static_cast<void(*)(lua_State*,::dmx::Attribute&)>([](lua_State *l,::dmx::Attribute &attr) {
		Lua::PushBool(l,attr.type != ::dmx::AttrType::Invalid);
	}));
	classDefAttribute.def("GetValue",static_cast<void(*)(lua_State*,::dmx::Attribute&)>([](lua_State *l,::dmx::Attribute &attr) {
		push_attribute_value(l,attr);
	}));
	classDefAttribute.def("GetValueAsString",static_cast<void(*)(lua_State*,::dmx::Attribute&)>([](lua_State *l,::dmx::Attribute &attr) {
		if(attr.data == nullptr)
		{
			Lua::PushString(l,"");
			return;
		}
		std::stringstream ss;
		switch(attr.type)
		{
			case ::dmx::AttrType::Element:
			{
				ss<<"Element";
				auto &wpElement = *static_cast<std::weak_ptr<::dmx::Element>*>(attr.data.get());
				if(wpElement.expired())
					ss<<" [NULL]";
				else
				{
					auto el = wpElement.lock();
					ss<<" ["<<el->name<<"]["<<el->type<<"]";
				}
				break;
			}
			case ::dmx::AttrType::String:
				ss<<*static_cast<std::string*>(attr.data.get());
				break;
			case ::dmx::AttrType::Int:
				ss<<*static_cast<int32_t*>(attr.data.get());
				break;
			case ::dmx::AttrType::Float:
				ss<<*static_cast<float*>(attr.data.get());
				break;
			case ::dmx::AttrType::Bool:
				ss<<*static_cast<bool*>(attr.data.get());
				break;
			case ::dmx::AttrType::Vector2:
				ss<<*static_cast<Vector2*>(attr.data.get());
				break;
			case ::dmx::AttrType::Vector3:
				ss<<*static_cast<Vector3*>(attr.data.get());
				break;
			case ::dmx::AttrType::Angle:
				ss<<*static_cast<EulerAngles*>(attr.data.get());
				break;
			case ::dmx::AttrType::Vector4:
				ss<<*static_cast<Vector4*>(attr.data.get());
				break;
			case ::dmx::AttrType::Quaternion:
				ss<<*static_cast<Quat*>(attr.data.get());
				break;
			case ::dmx::AttrType::Matrix:
				ss<<*static_cast<Mat4*>(attr.data.get());
				break;
			case ::dmx::AttrType::Color:
			{
				auto &c = *static_cast<std::array<uint8_t,4>*>(attr.data.get());
				ss<<c.at(0)<<" "<<c.at(1)<<" "<<c.at(2)<<" "<<c.at(3);
				break;
			}
			case ::dmx::AttrType::Time:
				ss<<*static_cast<float*>(attr.data.get());
				break;
			case ::dmx::AttrType::Binary:
			{
				auto &data = *static_cast<std::vector<uint8_t>*>(attr.data.get());
				auto len = data.size();
				ss<<"Binary ["<<util::get_pretty_bytes(len)<<"]";
				break;
			}
			default:
			{
				if(attr.type >= ::dmx::AttrType::ArrayFirst && attr.type <= ::dmx::AttrType::ArrayLast)
				{
					auto &vdata = *static_cast<std::vector<std::shared_ptr<::dmx::Attribute>>*>(attr.data.get());
					ss<<"Array ["<<::dmx::type_to_string(attr.type)<<"]["<<vdata.size()<<" elements]";
				}
				break;
			}
		}
		Lua::PushString(l,ss.str());
	}));
	modDMX[classDefAttribute];
}
#pragma optimize("",on)
