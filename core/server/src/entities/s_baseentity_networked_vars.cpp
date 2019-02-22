#include "stdafx_server.h"
#include "pragma/entities/s_baseentity.h"

// COMPONENT TODO
/*
uint32_t SBaseEntity::RegisterNetworkedVariableName(const std::string &name)
{
	uint32_t id;
	if(BaseEntity::RegisterNetworkedVariableName(name,id) == true) // New variable
	{
		NetPacket p;
		p->WriteString(name);
		p->Write<uint32_t>(id);
		SendNetEventTCP(umath::to_integral(NetEvent::RegisterNetworkedVar),p);
	}
	return id;
}

template<typename T>
	bool SBaseEntity::SetNetworkedVariable(const std::string &name,NetworkedVariable::Type type,T val,const std::function<bool()> &baseSetVar,bool bTCP)
{
	if(baseSetVar() == false)
		return false;
	auto id = RegisterNetworkedVariableName(name);
	NetPacket p;
	p->Write(id);
	p->Write(type);
	p->Write(val);
	if(bTCP == true)
		SendNetEventTCP(umath::to_integral(NetEvent::SetNetworkedVar),p);
	else
		SendNetEventUDP(umath::to_integral(NetEvent::SetNetworkedVar),p);
	return true;
}

bool SBaseEntity::SetNetworkedBool(const std::string &name,bool v,bool bTCP) {return SetNetworkedVariable<bool>(name,NetworkedVariable::Type::Bool,v,[&]() -> bool {return BaseEntity::SetNetworkedBool(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedChar(const std::string &name,char v,bool bTCP) {return SetNetworkedVariable<char>(name,NetworkedVariable::Type::Char,v,[&]() -> bool {return BaseEntity::SetNetworkedChar(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedDouble(const std::string &name,double v,bool bTCP) {return SetNetworkedVariable<double>(name,NetworkedVariable::Type::Double,v,[&]() -> bool {return BaseEntity::SetNetworkedDouble(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedFloat(const std::string &name,float v,bool bTCP) {return SetNetworkedVariable<float>(name,NetworkedVariable::Type::Float,v,[&]() -> bool {return BaseEntity::SetNetworkedFloat(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedInt8(const std::string &name,int8_t v,bool bTCP) {return SetNetworkedVariable<int8_t>(name,NetworkedVariable::Type::Int8,v,[&]() -> bool {return BaseEntity::SetNetworkedInt8(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedInt16(const std::string &name,int16_t v,bool bTCP) {return SetNetworkedVariable<int16_t>(name,NetworkedVariable::Type::Int16,v,[&]() -> bool {return BaseEntity::SetNetworkedInt16(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedInt32(const std::string &name,int32_t v,bool bTCP) {return SetNetworkedVariable<int32_t>(name,NetworkedVariable::Type::Int32,v,[&]() -> bool {return BaseEntity::SetNetworkedInt32(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedInt64(const std::string &name,int64_t v,bool bTCP) {return SetNetworkedVariable<int64_t>(name,NetworkedVariable::Type::Int64,v,[&]() -> bool {return BaseEntity::SetNetworkedInt64(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedLongDouble(const std::string &name,long double v,bool bTCP) {return SetNetworkedVariable<long double>(name,NetworkedVariable::Type::LongDouble,v,[&]() -> bool {return BaseEntity::SetNetworkedLongDouble(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedString(const std::string &name,const std::string &v,bool bTCP) {return SetNetworkedVariable<std::string>(name,NetworkedVariable::Type::String,v,[&]() -> bool {return BaseEntity::SetNetworkedString(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedUInt8(const std::string &name,uint8_t v,bool bTCP) {return SetNetworkedVariable<uint8_t>(name,NetworkedVariable::Type::UInt8,v,[&]() -> bool {return BaseEntity::SetNetworkedUInt8(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedUInt16(const std::string &name,uint16_t v,bool bTCP) {return SetNetworkedVariable<uint16_t>(name,NetworkedVariable::Type::UInt16,v,[&]() -> bool {return BaseEntity::SetNetworkedUInt16(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedUInt32(const std::string &name,uint32_t v,bool bTCP) {return SetNetworkedVariable<uint32_t>(name,NetworkedVariable::Type::UInt32,v,[&]() -> bool {return BaseEntity::SetNetworkedUInt32(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedUInt64(const std::string &name,uint64_t v,bool bTCP) {return SetNetworkedVariable<uint64_t>(name,NetworkedVariable::Type::UInt64,v,[&]() -> bool {return BaseEntity::SetNetworkedUInt64(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedAngles(const std::string &name,const EulerAngles &v,bool bTCP) {return SetNetworkedVariable<EulerAngles>(name,NetworkedVariable::Type::Angles,v,[&]() -> bool {return BaseEntity::SetNetworkedAngles(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedColor(const std::string &name,const Color &v,bool bTCP) {return SetNetworkedVariable<Color>(name,NetworkedVariable::Type::Color,v,[&]() -> bool {return BaseEntity::SetNetworkedColor(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedVector(const std::string &name,const Vector3 &v,bool bTCP) {return SetNetworkedVariable<Vector3>(name,NetworkedVariable::Type::Vector,v,[&]() -> bool {return BaseEntity::SetNetworkedVector(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedVector2(const std::string &name,const Vector2 &v,bool bTCP) {return SetNetworkedVariable<Vector2>(name,NetworkedVariable::Type::Vector2,v,[&]() -> bool {return BaseEntity::SetNetworkedVector2(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedVector4(const std::string &name,const Vector4 &v,bool bTCP) {return SetNetworkedVariable<Vector4>(name,NetworkedVariable::Type::Vector4,v,[&]() -> bool {return BaseEntity::SetNetworkedVector4(name,v);},bTCP);}
bool SBaseEntity::SetNetworkedEntity(const std::string &name,const BaseEntity *ent,bool bTCP)
{
	if(BaseEntity::SetNetworkedEntity(name,ent) == false)
		return false;
	auto id = RegisterNetworkedVariableName(name);
	NetPacket p;
	p->Write(id);
	p->Write(NetworkedVariable::Type::Entity);
	nwm::write_entity(p,ent);
	if(bTCP == true)
		SendNetEventTCP(umath::to_integral(NetEvent::SetNetworkedVar),p);
	else
		SendNetEventUDP(umath::to_integral(NetEvent::SetNetworkedVar),p);
	return true;
}

bool SBaseEntity::SetNetworkedBool(const std::string &name,bool v) {return SetNetworkedBool(name,v,false);}
bool SBaseEntity::SetNetworkedChar(const std::string &name,char v) {return SetNetworkedChar(name,v,false);}
bool SBaseEntity::SetNetworkedDouble(const std::string &name,double v) {return SetNetworkedDouble(name,v,false);}
bool SBaseEntity::SetNetworkedFloat(const std::string &name,float v) {return SetNetworkedFloat(name,v,false);}
bool SBaseEntity::SetNetworkedInt8(const std::string &name,int8_t v) {return SetNetworkedInt8(name,v,false);}
bool SBaseEntity::SetNetworkedInt16(const std::string &name,int16_t v) {return SetNetworkedInt16(name,v,false);}
bool SBaseEntity::SetNetworkedInt32(const std::string &name,int32_t v) {return SetNetworkedInt32(name,v,false);}
bool SBaseEntity::SetNetworkedInt64(const std::string &name,int64_t v) {return SetNetworkedInt64(name,v,false);}
bool SBaseEntity::SetNetworkedLongDouble(const std::string &name,long double v) {return SetNetworkedLongDouble(name,v,false);}
bool SBaseEntity::SetNetworkedString(const std::string &name,const std::string &v) {return SetNetworkedString(name,v,false);}
bool SBaseEntity::SetNetworkedUInt8(const std::string &name,uint8_t v) {return SetNetworkedUInt8(name,v,false);}
bool SBaseEntity::SetNetworkedUInt16(const std::string &name,uint16_t v) {return SetNetworkedUInt16(name,v,false);}
bool SBaseEntity::SetNetworkedUInt32(const std::string &name,uint32_t v) {return SetNetworkedUInt32(name,v,false);}
bool SBaseEntity::SetNetworkedUInt64(const std::string &name,uint64_t v) {return SetNetworkedUInt64(name,v,false);}
bool SBaseEntity::SetNetworkedAngles(const std::string &name,const EulerAngles &v) {return SetNetworkedAngles(name,v,false);}
bool SBaseEntity::SetNetworkedColor(const std::string &name,const Color &v) {return SetNetworkedColor(name,v,false);}
bool SBaseEntity::SetNetworkedVector(const std::string &name,const Vector3 &v) {return SetNetworkedVector(name,v,false);}
bool SBaseEntity::SetNetworkedVector2(const std::string &name,const Vector2 &v) {return SetNetworkedVector2(name,v,false);}
bool SBaseEntity::SetNetworkedVector4(const std::string &name,const Vector4 &v) {return SetNetworkedVector4(name,v,false);}
bool SBaseEntity::SetNetworkedEntity(const std::string &name,const BaseEntity *ent) {return SetNetworkedEntity(name,ent,false);}
*/
