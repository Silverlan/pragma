--- @meta
--- 
--- @class net
net = {}

--- 
--- @param protocol enum pragma::networking::Protocol
--- @param identifier string
--- @param packet net.Packet
function net.broadcast(protocol, identifier, packet) end

--- 
--- @param protocol enum pragma::networking::Protocol
--- @param identifier string
--- @param packet net.Packet
--- @param recipient ents.PlayerComponent
--- @overload fun(protocol: enum pragma::networking::Protocol, identifier: string, packet: net.Packet, recipients: net.RecipientFilter): 
--- @overload fun(protocol: enum pragma::networking::Protocol, identifier: string, packet: net.Packet, recipients: table): 
--- @overload fun(protocol: enum nwm::Protocol, identifier: string, packet: net.Packet): 
function net.send(protocol, identifier, packet, recipient) end

--- 
--- @param name string
--- @return int ret0
function net.register_event(name) end

--- 
--- @param name string
--- @param function unknown
function net.receive(name, function_) end

--- 
--- @param identifier string
--- @return bool ret0
function net.register(identifier) end


--- 
--- @class net.RecipientFilter
--- @overload fun():net.RecipientFilter
net.RecipientFilter = {}

--- 
--- @param hPl ents.PlayerComponent
function net.RecipientFilter:RemoveRecipient(hPl) end

--- 
--- @param hPl ents.PlayerComponent
--- @return bool ret0
function net.RecipientFilter:HasRecipient(hPl) end

--- 
function net.RecipientFilter:GetFilterType() end

--- 
--- @param filterType int
function net.RecipientFilter:SetFilterType(filterType) end

--- 
--- @return table ret0
function net.RecipientFilter:GetRecipients() end

--- 
--- @param hPl ents.PlayerComponent
function net.RecipientFilter:AddRecipient(hPl) end


--- @enum Type
net.RecipientFilter = {
	TYPE_EXCLUDE = 1,
	TYPE_INCLUDE = 0,
}

--- 
--- @class net.ClientRecipientFilter
net.ClientRecipientFilter = {}

--- 
function net.ClientRecipientFilter:GetRecipients() end


--- 
--- @class net.Packet
--- @overload fun():net.Packet
net.Packet = {}

--- 
function net.Packet:ReadQuaternion() end

--- 
--- @param t math.Vectori
function net.Packet:WriteVectori(t) end

--- 
function net.Packet:ReadMat3x4() end

--- 
--- @param t int
function net.Packet:WriteUInt64(t) end

--- 
function net.Packet:ReadMat4x3() end

--- 
function net.Packet:ReadMat4x2() end

--- 
function net.Packet:ReadVector2i() end

--- 
function net.Packet:ReadUInt8() end

--- 
--- @param t math.Vector2
function net.Packet:WriteVector2(t) end

--- 
function net.Packet:ReadMat4() end

--- 
function net.Packet:ReadEntity() end

--- 
function net.Packet:ReadLine() end

--- 
--- @param t int
function net.Packet:WriteInt64(t) end

--- 
--- @overload fun(hEnt: ents.BaseEntityBase): 
function net.Packet:WriteEntity() end

--- 
--- @param t number
function net.Packet:WriteLongDouble(t) end

--- 
function net.Packet:ReadVertex() end

--- 
--- @param t int
function net.Packet:WriteInt8(t) end

--- 
function net.Packet:ReadVector4i() end

--- 
function net.Packet:ReadVectori() end

--- 
--- @param offset int
function net.Packet:Seek(offset) end

--- 
function net.Packet:ReadSoundSource() end

--- 
--- @param hEnt class util::TWeakSharedHandle<class BaseEntity>
--- @overload fun(pl: class util::WeakHandle<class pragma::SPlayerComponent>): 
--- @overload fun(pl: class util::WeakHandle<class pragma::CPlayerComponent>): 
function net.Packet:WritePlayer(hEnt) end

--- 
--- @param t number
function net.Packet:WriteDouble(t) end

--- 
function net.Packet:ReadMat2x4() end

--- 
function net.Packet:GetSize() end

--- 
function net.Packet:Tell() end

--- 
--- @param size int
function net.Packet:Resize(size) end

--- 
--- @param size int
function net.Packet:Reserve(size) end

--- 
--- @param t math.Vector
function net.Packet:WriteVector(t) end

--- 
--- @param size int
function net.Packet:ReadBinaryString(size) end

--- 
function net.Packet:ReadBool() end

--- 
--- @param str string
--- @param bNullterminated bool
--- @overload fun(str: string): 
function net.Packet:WriteString(str, bNullterminated) end

--- 
function net.Packet:ReadChar() end

--- 
function net.Packet:ToBinaryString() end

--- 
function net.Packet:GetTimeSinceTransmission() end

--- 
function net.Packet:ReadMat3x2() end

--- 
--- @param t int
function net.Packet:WriteInt32(t) end

--- 
function net.Packet:ReadVector2() end

--- 
--- @param t math.Vector4
function net.Packet:WriteVector4(t) end

--- 
function net.Packet:ReadVector4() end

--- 
function net.Packet:ReadVector() end

--- 
--- @param dsOther net.Packet
--- @overload fun(dsOther: net.Packet, offset: int, size: int): 
function net.Packet:WriteBinary(dsOther) end

--- 
--- @overload fun(size: int): 
function net.Packet:ReadBinary() end

--- 
--- @overload fun(hEnt: ents.BaseEntityBase): 
function net.Packet:WriteUniqueEntity() end

--- 
--- @param t bool
function net.Packet:WriteBool(t) end

--- 
function net.Packet:ReadInt64() end

--- 
--- @param t math.Quaternion
function net.Packet:WriteQuaternion(t) end

--- 
function net.Packet:ReadMat2x3() end

--- 
function net.Packet:ReadUInt64() end

--- 
function net.Packet:ReadInt8() end

--- 
--- @param t int
function net.Packet:WriteUInt8(t) end

--- 
function net.Packet:ReadInt32() end

--- 
--- @param t int
function net.Packet:WriteUInt32(t) end

--- 
function net.Packet:ReadUInt32() end

--- 
--- @param t int
function net.Packet:WriteInt16(t) end

--- 
function net.Packet:ReadLongDouble() end

--- 
function net.Packet:ReadInt16() end

--- 
--- @param t int
function net.Packet:WriteUInt16(t) end

--- 
function net.Packet:ReadUInt16() end

--- 
--- @param t int
function net.Packet:WriteChar(t) end

--- 
--- @param t number
function net.Packet:WriteFloat(t) end

--- 
function net.Packet:ReadFloat() end

--- 
function net.Packet:ReadDouble() end

--- 
--- @param snd sound.Source
function net.Packet:WriteSoundSource(snd) end

--- 
function net.Packet:ReadPlayer() end

--- 
function net.Packet:ReadMat3() end

--- 
function net.Packet:ReadMat2() end

--- 
--- @param pattern string
function net.Packet:ReadStringUntil(pattern) end

--- 
--- @param len int
--- @overload fun(): 
function net.Packet:ReadString(len) end

--- 
--- @param t math.Mat2x4
function net.Packet:WriteMat2x4(t) end

--- 
function net.Packet:Clear() end

--- 
--- @param t math.EulerAngles
function net.Packet:WriteAngles(t) end

--- 
function net.Packet:ReadAngles() end

--- 
--- @param t math.Mat2
function net.Packet:WriteMat2(t) end

--- 
--- @param t math.Mat2x3
function net.Packet:WriteMat2x3(t) end

--- 
function net.Packet:__tostring() end

--- 
--- @param t math.Mat3x2
function net.Packet:WriteMat3x2(t) end

--- 
--- @param t math.Mat3
function net.Packet:WriteMat3(t) end

--- 
--- @param t math.Mat3x4
function net.Packet:WriteMat3x4(t) end

--- 
--- @param t math.Mat4x2
function net.Packet:WriteMat4x2(t) end

--- 
--- @param t math.Mat4x3
function net.Packet:WriteMat4x3(t) end

--- 
--- @param t math.Mat4
function net.Packet:WriteMat4(t) end

--- 
--- @param t math.Vector2i
function net.Packet:WriteVector2i(t) end

--- 
--- @param t math.Vector4i
function net.Packet:WriteVector4i(t) end

--- 
--- @param t game.Model.Vertex
function net.Packet:WriteVertex(t) end

--- 
--- @param o any
function net.Packet:ReadUniqueEntity(o) end


