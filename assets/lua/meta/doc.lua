--- @meta
--- 
--- @class doc
doc = {}

--- 
--- @param buildDir string
function doc.autogenerate(buildDir) end

--- 
--- @param fileName string
--- @return doc.Collection ret0
function doc.load(fileName) end

--- 
function doc.generate_lad_assets() end

--- 
function doc.generate_zerobrane_autocomplete_script() end

--- 
--- @return string ret0
function doc.generate_convar_documentation() end


--- 
--- @class doc.Group
doc.Group = {}

--- 
function doc.Group:GetName() end


--- 
--- @class doc.Collection
doc.Collection = {}

--- 
function doc.Collection:GetEnumSets() end

--- 
function doc.Collection:GetParent() end

--- 
function doc.Collection:GetDerivedFrom() end

--- 
function doc.Collection:GetWikiURL() end

--- 
function doc.Collection:GetName() end

--- 
function doc.Collection:GetRelated() end

--- 
function doc.Collection:GetFullName() end

--- 
function doc.Collection:GetFlags() end

--- 
function doc.Collection:GetURL() end

--- 
function doc.Collection:GetChildren() end

--- 
function doc.Collection:GetDescription() end

--- 
function doc.Collection:GetFunctions() end

--- 
function doc.Collection:GetMembers() end


--- @enum FlagBit
doc.Collection = {
	FLAG_BIT_BASE = 1,
	FLAG_BIT_CLASS = 4,
	FLAG_BIT_LIBRARY = 2,
}

--- @enum FlagNone
doc.Collection = {
	FLAG_NONE = 0,
}

--- 
--- @class doc.Function
doc.Function = {}

--- 
function doc.Function:GetExampleCode() end

--- 
function doc.Function:GetOverloads() end

--- 
function doc.Function:GetFlags() end

--- 
function doc.Function:GetRelated() end

--- 
function doc.Function:GetGroups() end

--- 
function doc.Function:GetType() end

--- 
function doc.Function:GetGameStateFlags() end

--- 
function doc.Function:GetFullName() end

--- 
function doc.Function:GetName() end

--- 
function doc.Function:GetDescription() end

--- 
function doc.Function:GetURL() end


--- @enum Flag
doc.Function = {
	FLAG_BIT_DEBUG = 1,
	FLAG_DEPRECATED = 4,
	FLAG_NONE = 0,
	FLAG_VANILLA = 2,
}

--- @enum Type
doc.Function = {
	TYPE_FUNCTION = 0,
	TYPE_HOOK = 2,
	TYPE_METHOD = 1,
}

--- 
--- @class doc.Function.ExampleCode
--- @field code string 
--- @field description string 
doc.Function.ExampleCode = {}


--- 
--- @class doc.BaseCollectionObject
doc.BaseCollectionObject = {}

--- 
function doc.BaseCollectionObject:GetFullName() end

--- 
function doc.BaseCollectionObject:GetWikiURL() end


--- 
--- @class doc.Member
doc.Member = {}

--- 
function doc.Member:GetName() end

--- 
function doc.Member:GetWikiURL() end

--- 
function doc.Member:GetDefault() end

--- 
function doc.Member:GetMode() end

--- 
function doc.Member:GetDescription() end

--- 
function doc.Member:GetFullName() end

--- 
function doc.Member:GetGameStateFlags() end

--- 
--- @return struct pragma::doc::Variant ret0
function doc.Member:GetType() end


--- @enum Mode
doc.Member = {
	MODE_NONE = 0,
	MODE_READ = 1,
	MODE_WRITE = 2,
}

--- 
--- @class doc.Module
doc.Module = {}

--- 
function doc.Module:GetTarget() end

--- 
function doc.Module:GetWikiURL() end


--- 
--- @class doc.EnumSet
doc.EnumSet = {}

--- 
function doc.EnumSet:GetEnums() end

--- 
function doc.EnumSet:GetUnderlyingType() end

--- 
function doc.EnumSet:GetWikiURL() end

--- 
function doc.EnumSet:GetName() end

--- 
function doc.EnumSet:GetFullName() end


--- 
--- @class doc.Enum
doc.Enum = {}

--- 
function doc.Enum:GetName() end

--- 
function doc.Enum:GetWikiURL() end

--- 
function doc.Enum:GetValue() end

--- 
function doc.Enum:GetFullName() end

--- 
function doc.Enum:GetGameStateFlags() end

--- 
function doc.Enum:GetDescription() end

--- 
function doc.Enum:GetType() end


--- @enum Type
doc.Enum = {
	TYPE_BIT = 1,
	TYPE_REGULAR = 0,
}

--- 
--- @class doc.Overload
doc.Overload = {}

--- 
--- @overload fun(): 
function doc.Overload:GetParameters() end


--- 
--- @class doc.DerivedFrom
doc.DerivedFrom = {}

--- 
function doc.DerivedFrom:GetName() end

--- 
function doc.DerivedFrom:GetParent() end


