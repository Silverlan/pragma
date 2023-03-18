--- @meta
--- 
--- @class dmx
dmx = {}


--- 
--- @class dmx.Data
dmx.Data = {}

--- 
function dmx.Data:GetElements() end

--- 
function dmx.Data:GetRootAttribute() end

--- 
function dmx.Data:__tostring() end


--- 
--- @class dmx.Attribute
dmx.Attribute = {}

--- 
function dmx.Attribute:GetValue() end

--- 
function dmx.Attribute:GetType() end

--- 
function dmx.Attribute:__tostring() end

--- 
--- @param val dmx.Attribute
function dmx.Attribute:RemoveArrayValue(val) end

--- 
--- @param attrOther dmx.Attribute
function dmx.Attribute:__eq(attrOther) end

--- 
function dmx.Attribute:IsValid() end

--- 
--- @param val dmx.Attribute
function dmx.Attribute:AddArrayValue(val) end

--- 
function dmx.Attribute:GetValueAsString() end

--- 
--- @param name string
function dmx.Attribute:Get(name) end


--- @enum Type
dmx.Attribute = {
	TYPE_TIME_ARRAY = 24,
	TYPE_ANGLE = 13,
	TYPE_BINARY_ARRAY = 23,
	TYPE_TIME = 7,
	TYPE_BINARY = 6,
	TYPE_ANGLE_ARRAY = 30,
	TYPE_BOOL = 4,
	TYPE_BOOL_ARRAY = 21,
	TYPE_MATRIX_ARRAY = 32,
	TYPE_COLOR = 9,
	TYPE_COLOR_ARRAY = 26,
	TYPE_ELEMENT = 1,
	TYPE_ELEMENT_ARRAY = 18,
	TYPE_NONE = 0,
	TYPE_FLOAT = 3,
	TYPE_FLOAT_ARRAY = 20,
	TYPE_INT = 2,
	TYPE_INT_ARRAY = 19,
	TYPE_INVALID = -1,
	TYPE_MATRIX = 15,
	TYPE_OBJECTID = 8,
	TYPE_QUATERNION = 14,
	TYPE_OBJECTID_ARRAY = 25,
	TYPE_QUATERNION_ARRAY = 31,
	TYPE_STRING = 5,
	TYPE_STRING_ARRAY = 22,
	TYPE_UINT64 = 16,
	TYPE_UINT8 = 17,
	TYPE_VECTOR2 = 10,
	TYPE_VECTOR2_ARRAY = 27,
	TYPE_VECTOR3 = 11,
	TYPE_VECTOR3_ARRAY = 28,
	TYPE_VECTOR4 = 12,
	TYPE_VECTOR4_ARRAY = 29,
}

--- 
--- @class dmx.Element
dmx.Element = {}

--- 
function dmx.Element:GetGUID() end

--- 
--- @param name string
function dmx.Element:GetAttrV(name) end

--- 
--- @param id string
function dmx.Element:GetAttributeValue(id) end

--- 
--- @param name string
function dmx.Element:GetAttr(name) end

--- 
function dmx.Element:GetType() end

--- 
--- @param elOther dmx.Element
function dmx.Element:__eq(elOther) end

--- 
function dmx.Element:GetName() end

--- 
--- @param id string
function dmx.Element:GetAttribute(id) end

--- 
function dmx.Element:GetAttributes() end

--- 
--- @param name string
function dmx.Element:Get(name) end

--- 
function dmx.Element:__tostring() end


