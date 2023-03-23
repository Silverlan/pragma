--- @meta
--- 
--- @class udm
udm = {}

--- 
--- @param assetType string
--- @return variant ret0
--- @overload fun(assetType: string, assetVersion: int): variant
--- @overload fun(): variant
--- @overload fun(rootProp: udm.Property): variant
--- @overload fun(rootProp: udm.Property, assetType: string): variant
--- @overload fun(rootProp: udm.Property, assetType: string, assetVersion: int): variant
function udm.create(assetType) end

--- 
--- @param udmStr string
--- @return variant ret0
function udm.parse(udmStr) end

--- 
--- @param type enum udm::Type
--- @return bool ret0
function udm.is_integral_type(type) end

--- 
--- @param type enum udm::Type
--- @return udm.Property ret0
function udm.create_property(type) end

--- 
--- @return udm.Property ret0
function udm.create_element() end

--- 
--- @param fileName string
--- @return variant ret0
function udm.convert_udm_file_to_ascii(fileName) end

--- 
--- @param dataBlock util.DataBlock
--- @param udm udm.LinkedPropertyWrapper
function udm.data_block_to_udm(dataBlock, udm) end

--- 
--- @param ds util.DataStream
--- @return util.DataStream ret0
function udm.compress_lz4(ds) end

--- 
--- @param o0 any
--- @param t0 enum udm::Type
--- @param t1 enum udm::Type
--- @return any ret0
function udm.convert(o0, t0, t1) end

--- 
--- @param fileName string
--- @return bool ret0
function udm.data_file_to_udm(fileName) end

--- 
--- @param t table
--- @return util.DataStream ret0
function udm.decompress_lz4(t) end

--- 
--- @return bool ret0
function udm.debug_test() end

--- 
--- @param t table
--- @return udm.StructDescription ret0
function udm.define_struct(t) end

--- 
--- @param t enum udm::Type
--- @return int ret0
function udm.enum_type_to_ascii(t) end

--- 
--- @param type class std::basic_string_view<char,struct std::char_traits<char> >
--- @return enum udm::Type ret0
function udm.ascii_type_to_enum(type) end

--- 
--- @param tFrom enum udm::Type
--- @param tTo enum udm::Type
--- @return bool ret0
function udm.is_convertible(tFrom, tTo) end

--- 
--- @param p udm.PropertyWrapper
--- @return string ret0
function udm.serialize(p) end

--- 
--- @param str string
--- @return variant ret0
function udm.deserialize(str) end

--- 
--- @param valueType enum udm::Type
--- @param arrayType enum udm::ArrayType
--- @return bool ret0
--- @overload fun(valueType: enum udm::Type, arrayType: enum udm::ArrayType): bool
function udm.is_supported_array_value_type(valueType, arrayType) end

--- 
--- @param udm udm.LinkedPropertyWrapper
--- @return string ret0
function udm.to_json(udm) end

--- 
--- @param t enum udm::Type
--- @return bool ret0
function udm.is_numeric_type(t) end

--- 
--- @param type enum udm::Type
--- @return bool ret0
function udm.is_floating_point_type(type) end

--- 
--- @param t enum udm::Type
--- @return bool ret0
function udm.is_generic_type(t) end

--- 
--- @param value0 any
--- @param value1 any
--- @param t number
--- @param type enum udm::Type
--- @return struct luabind::adl::udm_type<class luabind::adl::object,1,1,0> ret0
--- @overload fun(value0: any, value1: any, t: number): struct luabind::adl::udm_type<class luabind::adl::object,1,1,0>
function udm.lerp(value0, value1, t, type) end

--- 
--- @param t enum udm::Type
--- @return bool ret0
function udm.is_non_trivial_type(t) end

--- 
--- @param type enum udm::Type
--- @return bool ret0
function udm.is_array_type(type) end

--- 
--- @param type enum udm::Type
--- @return bool ret0
function udm.is_vector_type(type) end

--- 
--- @param type enum udm::Type
--- @return bool ret0
function udm.is_matrix_type(type) end

--- 
--- @param type enum udm::Type
--- @return int ret0
function udm.get_matrix_row_count(type) end

--- 
--- @param type enum udm::Type
--- @return int ret0
function udm.get_matrix_column_count(type) end

--- 
--- @param fileName string
--- @return variant ret0
function udm.get_format_type(fileName) end

--- 
--- @param type enum udm::Type
--- @return string ret0
function udm.type_to_string(type) end

--- 
--- @param type enum udm::Type
--- @return any ret0
function udm.get_class_type(type) end

--- 
--- @param value any
--- @param idx int
--- @param type enum udm::Type
--- @return int ret0
--- @overload fun(value: any, idx: int): int
function udm.get_numeric_component(value, idx, type) end

--- 
--- @param fileName string
--- @return variant ret0
function udm.convert_udm_file_to_binary(fileName) end

--- 
--- @param file file.File
--- @return variant ret0
--- @overload fun(fileName: string): variant
function udm.open(file) end

--- 
--- @param file file.File
--- @return variant ret0
--- @overload fun(fileName: string): variant
function udm.load(file) end

--- 
--- @param type enum udm::Type
--- @return int ret0
function udm.get_numeric_component_count(type) end


--- 
--- @class udm.HdrColor
--- @field b int 
--- @field r int 
--- @field a int 
--- @field g int 
--- @overload fun():udm.HdrColor
udm.HdrColor = {}

--- 
function udm.HdrColor:__tostring() end


--- 
--- @class udm.Property
udm.Property = {}

--- 
--- @return any ret0
function udm.Property:GetValueType() end

--- 
--- @return udm.Property ret0
function udm.Property:Copy() end

--- 
function udm.Property:Clear() end

--- 
--- @param arg2 string
--- @overload fun(arg2: int): 
function udm.Property:RemoveValue(arg2) end

--- 
--- @param arg2 string
--- @return any ret0
--- @overload fun(arg2: string, arg3: enum udm::Type): any
--- @overload fun(): any
--- @overload fun(arg2: enum udm::Type): any
function udm.Property:GetArrayValues(arg2) end

--- 
--- @param arg2 int
--- @param arg3 int
function udm.Property:AddValueRange(arg2, arg3) end

--- 
--- @param arg2 string
--- @return any ret0
--- @overload fun(): any
function udm.Property:GetChildren(arg2) end

--- 
--- @return table ret0
function udm.Property:ToTable() end

--- 
--- @param arg2 string
--- @return bool ret0
function udm.Property:HasValue(arg2) end

--- 
--- @param arg2 int
--- @param arg3 int
function udm.Property:RemoveValueRange(arg2, arg3) end

--- 
--- @param arg2 enum udm::Type
--- @return any ret0
--- @overload fun(): any
--- @overload fun(arg2: string, arg3: enum udm::Type): any
--- @overload fun(arg2: string): any
function udm.Property:GetValue(arg2) end

--- 
--- @return bool ret0
function udm.Property:IsValid() end

--- 
--- @param arg2 int
function udm.Property:Resize(arg2) end

--- 
function udm.Property:GetBlobData() end

--- 
--- @return enum udm::Type ret0
function udm.Property:GetType() end

--- 
--- @param arg2 enum udm::Type
function udm.Property:SetValueType(arg2) end

--- 
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(idx: int): udm.LinkedPropertyWrapper
--- @overload fun(key: string): udm.LinkedPropertyWrapper
function udm.Property:Get() end

--- 
--- @param propName string
--- @return string ret0
--- @overload fun(propName: string, prefix: string): string
--- @overload fun(arg2: enum udm::AsciiSaveFlags): string
--- @overload fun(): string
function udm.Property:ToAscii(propName) end

--- 
--- @param arg2 string
--- @param arg3 util.DataStream
--- @overload fun(arg2: int, arg3: util.DataStream): 
--- @overload fun(key: string, v: udm.Reference): 
--- @overload fun(key: string, v: udm.HdrColor): 
--- @overload fun(key: string, v: udm.Srgba): 
--- @overload fun(key: string, v: string): 
--- @overload fun(key: string, v: math.Mat3x4): 
--- @overload fun(key: string, v: math.Mat4): 
--- @overload fun(key: string, v: math.ScaledTransform): 
--- @overload fun(key: string, v: math.Transform): 
--- @overload fun(key: string, v: math.EulerAngles): 
--- @overload fun(key: string, v: math.Quaternion): 
--- @overload fun(key: string, v: math.Vector4): 
--- @overload fun(key: string, v: math.Vector): 
--- @overload fun(key: string, v: math.Vector2): 
--- @overload fun(key: string, v: number): 
--- @overload fun(key: string, v: bool): 
--- @overload fun(idx: int, v: udm.Reference): 
--- @overload fun(idx: int, v: udm.HdrColor): 
--- @overload fun(idx: int, v: udm.Srgba): 
--- @overload fun(idx: int, v: string): 
--- @overload fun(idx: int, v: math.Mat3x4): 
--- @overload fun(idx: int, v: math.Mat4): 
--- @overload fun(idx: int, v: math.ScaledTransform): 
--- @overload fun(idx: int, v: math.Transform): 
--- @overload fun(idx: int, v: math.EulerAngles): 
--- @overload fun(idx: int, v: math.Quaternion): 
--- @overload fun(idx: int, v: math.Vector4): 
--- @overload fun(idx: int, v: math.Vector): 
--- @overload fun(idx: int, v: math.Vector2): 
--- @overload fun(idx: int, v: number): 
--- @overload fun(idx: int, v: bool): 
--- @overload fun(key: string, v: vector.Vector): 
--- @overload fun(idx: int, v: vector.Vector): 
function udm.Property:Set(arg2, arg3) end

--- 
--- @param arg2 any
--- @overload fun(arg2: int, arg3: any): 
function udm.Property:InsertValue(arg2) end

--- 
--- @param arg2 string
--- @param arg3 enum udm::Type
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(arg2: string): udm.LinkedPropertyWrapper
function udm.Property:Add(arg2, arg3) end

--- 
--- @param arg2 string
--- @return udm.AssetData ret0
function udm.Property:AddAssetData(arg2) end

--- 
--- @return udm.AssetData ret0
function udm.Property:ToAssetData() end

--- 
--- @param arg2 int
--- @return any ret0
function udm.Property:GetArrayValueDirect(arg2) end

--- 
--- @param arg2 enum udm::Type
--- @return any ret0
function udm.Property:GetArrayValuesFromBlob(arg2) end

--- 
--- @param arg2 string
--- @param arg3 enum udm::Type
--- @param arg4 table
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table, arg5: enum udm::Type): 
function udm.Property:AddBlobFromArrayValues(arg2, arg3, arg4) end

--- 
function udm.Property:__tostring() end

--- 
--- @param arg2 int
--- @return any ret0
--- @overload fun(arg2: int, arg3: enum udm::Type): any
function udm.Property:GetArrayValue(arg2) end

--- 
--- @param arg2 string
--- @param arg3 udm.StructDescription
--- @param arg4 int
--- @param arg5 util.DataStream
--- @param arg6 enum udm::Type
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table, arg5: enum udm::Type): 
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table): 
--- @overload fun(arg2: enum udm::Type, arg3: table, arg4: enum udm::Type): 
--- @overload fun(arg2: enum udm::Type, arg3: table): 
function udm.Property:SetArrayValues(arg2, arg3, arg4, arg5, arg6) end

--- 
--- @return int ret0
function udm.Property:GetSize() end

--- 
--- @param arg2 string
--- @param arg3 int
--- @param arg4 enum udm::Type
--- @param arg5 enum udm::ArrayType
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(arg2: string, arg3: int, arg4: enum udm::Type): udm.LinkedPropertyWrapper
--- @overload fun(arg2: string, arg3: int): udm.LinkedPropertyWrapper
function udm.Property:AddArray(arg2, arg3, arg4, arg5) end

--- 
--- @param arg2 string
--- @return udm.LinkedPropertyWrapper ret0
function udm.Property:GetFromPath(arg2) end

--- 
--- @param arg2 string
--- @return int ret0
--- @overload fun(): int
function udm.Property:GetChildCount(arg2) end

--- 
--- @param arg2 string
--- @param arg3 any
--- @overload fun(arg2: string, arg3: enum udm::Type): 
--- @overload fun(arg2: int, arg3: enum udm::Type, arg4: any): 
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: any): 
function udm.Property:SetValue(arg2, arg3) end

--- 
--- @param arg2 udm.PropertyWrapper
--- @param arg3 enum udm::MergeFlags
--- @overload fun(arg2: udm.PropertyWrapper): 
function udm.Property:Merge(arg2, arg3) end


--- 
--- @class udm.Struct
--- @overload fun(arg1: udm.StructDescription):udm.Struct
--- @overload fun():udm.Struct
udm.Struct = {}

--- 
--- @return udm.StructDescription ret0
function udm.Struct:GetDescription() end

--- 
--- @return any ret0
function udm.Struct:GetValue() end


--- 
--- @class udm.LinkedPropertyWrapper: udm.PropertyWrapper
udm.LinkedPropertyWrapper = {}

--- 
--- @return udm.AssetData ret0
function udm.LinkedPropertyWrapper:GetAssetData() end

--- 
--- @return udm.Property ret0
function udm.LinkedPropertyWrapper:ClaimOwnership() end

--- 
--- @return string ret0
function udm.LinkedPropertyWrapper:GetPath() end


--- 
--- @class udm.Reference
--- @overload fun(arg1: string):udm.Reference
--- @overload fun():udm.Reference
udm.Reference = {}

--- 
--- @return udm.PropertyWrapper ret0
function udm.Reference:GetProperty() end

--- 
function udm.Reference:__tostring() end


--- 
--- @class udm.AssetData: udm.LinkedPropertyWrapper
udm.AssetData = {}

--- 
--- @return string ret0
function udm.AssetData:GetAssetType() end

--- 
--- @return int ret0
function udm.AssetData:GetAssetVersion() end

--- 
--- @param prop udm.Property
function udm.AssetData:SetData(prop) end

--- 
--- @param arg1 string
function udm.AssetData:SetAssetType(arg1) end

--- 
--- @param arg1 int
function udm.AssetData:SetAssetVersion(arg1) end

--- 
--- @return udm.LinkedPropertyWrapper ret0
function udm.AssetData:GetData() end


--- 
--- @class udm.Element
udm.Element = {}

--- 
--- @return any ret0
function udm.Element:GetValueType() end

--- 
--- @return udm.Property ret0
function udm.Element:Copy() end

--- 
function udm.Element:Clear() end

--- 
--- @param arg2 string
--- @overload fun(arg2: int): 
function udm.Element:RemoveValue(arg2) end

--- 
--- @param arg2 string
--- @return any ret0
--- @overload fun(arg2: string, arg3: enum udm::Type): any
--- @overload fun(): any
--- @overload fun(arg2: enum udm::Type): any
function udm.Element:GetArrayValues(arg2) end

--- 
--- @param arg2 int
--- @param arg3 int
function udm.Element:AddValueRange(arg2, arg3) end

--- 
--- @param arg2 string
--- @return any ret0
--- @overload fun(): any
function udm.Element:GetChildren(arg2) end

--- 
--- @return table ret0
function udm.Element:ToTable() end

--- 
--- @param arg2 string
--- @return bool ret0
function udm.Element:HasValue(arg2) end

--- 
--- @param arg2 int
--- @param arg3 int
function udm.Element:RemoveValueRange(arg2, arg3) end

--- 
--- @param arg2 enum udm::Type
--- @return any ret0
--- @overload fun(): any
--- @overload fun(arg2: string, arg3: enum udm::Type): any
--- @overload fun(arg2: string): any
function udm.Element:GetValue(arg2) end

--- 
--- @return bool ret0
function udm.Element:IsValid() end

--- 
--- @param arg2 int
function udm.Element:Resize(arg2) end

--- 
function udm.Element:GetBlobData() end

--- 
--- @return enum udm::Type ret0
function udm.Element:GetType() end

--- 
--- @param arg2 enum udm::Type
function udm.Element:SetValueType(arg2) end

--- 
--- @param idx int
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(key: string): udm.LinkedPropertyWrapper
function udm.Element:Get(idx) end

--- 
--- @param arg2 enum udm::AsciiSaveFlags
--- @return any ret0
--- @overload fun(): any
function udm.Element:ToAscii(arg2) end

--- 
--- @param arg2 string
--- @param arg3 util.DataStream
--- @overload fun(arg2: int, arg3: util.DataStream): 
--- @overload fun(key: string, v: udm.Reference): 
--- @overload fun(key: string, v: udm.HdrColor): 
--- @overload fun(key: string, v: udm.Srgba): 
--- @overload fun(key: string, v: string): 
--- @overload fun(key: string, v: math.Mat3x4): 
--- @overload fun(key: string, v: math.Mat4): 
--- @overload fun(key: string, v: math.ScaledTransform): 
--- @overload fun(key: string, v: math.Transform): 
--- @overload fun(key: string, v: math.EulerAngles): 
--- @overload fun(key: string, v: math.Quaternion): 
--- @overload fun(key: string, v: math.Vector4): 
--- @overload fun(key: string, v: math.Vector): 
--- @overload fun(key: string, v: math.Vector2): 
--- @overload fun(key: string, v: number): 
--- @overload fun(key: string, v: bool): 
--- @overload fun(idx: int, v: udm.Reference): 
--- @overload fun(idx: int, v: udm.HdrColor): 
--- @overload fun(idx: int, v: udm.Srgba): 
--- @overload fun(idx: int, v: string): 
--- @overload fun(idx: int, v: math.Mat3x4): 
--- @overload fun(idx: int, v: math.Mat4): 
--- @overload fun(idx: int, v: math.ScaledTransform): 
--- @overload fun(idx: int, v: math.Transform): 
--- @overload fun(idx: int, v: math.EulerAngles): 
--- @overload fun(idx: int, v: math.Quaternion): 
--- @overload fun(idx: int, v: math.Vector4): 
--- @overload fun(idx: int, v: math.Vector): 
--- @overload fun(idx: int, v: math.Vector2): 
--- @overload fun(idx: int, v: number): 
--- @overload fun(idx: int, v: bool): 
--- @overload fun(key: string, v: vector.Vector): 
--- @overload fun(idx: int, v: vector.Vector): 
function udm.Element:Set(arg2, arg3) end

--- 
--- @param arg2 any
--- @overload fun(arg2: int, arg3: any): 
function udm.Element:InsertValue(arg2) end

--- 
--- @param arg2 string
--- @param arg3 enum udm::Type
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(arg2: string): udm.LinkedPropertyWrapper
function udm.Element:Add(arg2, arg3) end

--- 
--- @param arg2 string
--- @return udm.AssetData ret0
function udm.Element:AddAssetData(arg2) end

--- 
--- @return udm.AssetData ret0
function udm.Element:ToAssetData() end

--- 
--- @param arg2 int
--- @return any ret0
function udm.Element:GetArrayValueDirect(arg2) end

--- 
--- @param arg2 enum udm::Type
--- @return any ret0
function udm.Element:GetArrayValuesFromBlob(arg2) end

--- 
--- @param arg2 string
--- @param arg3 enum udm::Type
--- @param arg4 table
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table, arg5: enum udm::Type): 
function udm.Element:AddBlobFromArrayValues(arg2, arg3, arg4) end

--- 
function udm.Element:__tostring() end

--- 
--- @param arg2 int
--- @return any ret0
--- @overload fun(arg2: int, arg3: enum udm::Type): any
function udm.Element:GetArrayValue(arg2) end

--- 
--- @param arg2 string
--- @param arg3 udm.StructDescription
--- @param arg4 int
--- @param arg5 util.DataStream
--- @param arg6 enum udm::Type
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table, arg5: enum udm::Type): 
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table): 
--- @overload fun(arg2: enum udm::Type, arg3: table, arg4: enum udm::Type): 
--- @overload fun(arg2: enum udm::Type, arg3: table): 
function udm.Element:SetArrayValues(arg2, arg3, arg4, arg5, arg6) end

--- 
--- @return int ret0
function udm.Element:GetSize() end

--- 
--- @param arg2 string
--- @param arg3 int
--- @param arg4 enum udm::Type
--- @param arg5 enum udm::ArrayType
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(arg2: string, arg3: int, arg4: enum udm::Type): udm.LinkedPropertyWrapper
--- @overload fun(arg2: string, arg3: int): udm.LinkedPropertyWrapper
function udm.Element:AddArray(arg2, arg3, arg4, arg5) end

--- 
--- @param arg2 string
--- @return udm.LinkedPropertyWrapper ret0
function udm.Element:GetFromPath(arg2) end

--- 
--- @param arg2 string
--- @return int ret0
--- @overload fun(): int
function udm.Element:GetChildCount(arg2) end

--- 
--- @param arg2 string
--- @param arg3 any
--- @overload fun(arg2: string, arg3: enum udm::Type): 
--- @overload fun(arg2: int, arg3: enum udm::Type, arg4: any): 
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: any): 
function udm.Element:SetValue(arg2, arg3) end

--- 
--- @param arg2 udm.PropertyWrapper
--- @param arg3 enum udm::MergeFlags
--- @overload fun(arg2: udm.PropertyWrapper): 
function udm.Element:Merge(arg2, arg3) end


--- 
--- @class udm.Data
udm.Data = {}

--- 
function udm.Data:__tostring() end

--- 
--- @return string ret0
function udm.Data:ToAscii() end

--- 
--- @param key string
--- @return udm.LinkedPropertyWrapper ret0
function udm.Data:Get(key) end

--- 
--- @param f file.File
--- @overload fun(fileName: string): 
function udm.Data:Save(f) end

--- 
--- @return udm.AssetData ret0
function udm.Data:GetAssetData() end

--- 
function udm.Data:ResolveReferences() end

--- 
--- @param path string
function udm.Data:LoadProperty(path) end

--- 
--- @return udm.Element ret0
function udm.Data:GetRootElement() end

--- 
--- @param f file.File
--- @param flags enum udm::AsciiSaveFlags
--- @overload fun(f: file.File): 
--- @overload fun(fileName: string, flags: enum udm::AsciiSaveFlags): 
--- @overload fun(fileName: string): 
function udm.Data:SaveAscii(f, flags) end


--- 
--- @class udm.Array
udm.Array = {}

--- 
--- @return table ret0
function udm.Array:ToTable() end

--- 
--- @return enum udm::ArrayType ret0
function udm.Array:GetArrayType() end

--- 
function udm.Array:__tostring() end


--- 
--- @class udm.PropertyWrapper
--- @overload fun():udm.PropertyWrapper
udm.PropertyWrapper = {}

--- 
--- @return udm.Property ret0
function udm.PropertyWrapper:Copy() end

--- 
--- @return any ret0
function udm.PropertyWrapper:GetValueType() end

--- 
function udm.PropertyWrapper:Clear() end

--- 
--- @param arg2 string
--- @overload fun(arg2: int): 
function udm.PropertyWrapper:RemoveValue(arg2) end

--- 
--- @param arg2 string
--- @return any ret0
--- @overload fun(arg2: string, arg3: enum udm::Type): any
--- @overload fun(): any
--- @overload fun(arg2: enum udm::Type): any
function udm.PropertyWrapper:GetArrayValues(arg2) end

--- 
--- @param arg2 int
--- @param arg3 int
function udm.PropertyWrapper:AddValueRange(arg2, arg3) end

--- 
--- @param arg2 string
--- @return any ret0
--- @overload fun(): any
function udm.PropertyWrapper:GetChildren(arg2) end

--- 
--- @return class LuaUdmArrayIterator ret0
function udm.PropertyWrapper:It() end

--- 
--- @param arg2 string
--- @return bool ret0
function udm.PropertyWrapper:HasValue(arg2) end

--- 
--- @param arg2 int
--- @param arg3 int
function udm.PropertyWrapper:RemoveValueRange(arg2, arg3) end

--- 
--- @return table ret0
function udm.PropertyWrapper:ToTable() end

--- 
function udm.PropertyWrapper:DebugPrint() end

--- 
--- @return enum udm::Type ret0
function udm.PropertyWrapper:GetType() end

--- 
--- @param arg2 enum udm::Type
--- @return any ret0
--- @overload fun(): any
--- @overload fun(arg2: string, arg3: enum udm::Type): any
--- @overload fun(arg2: string): any
function udm.PropertyWrapper:GetValue(arg2) end

--- 
--- @return bool ret0
function udm.PropertyWrapper:IsValid() end

--- 
--- @param arg2 any
--- @overload fun(arg2: int, arg3: any): 
function udm.PropertyWrapper:InsertValue(arg2) end

--- 
--- @param arg2 string
--- @param arg3 udm.StructDescription
--- @param arg4 int
--- @param arg5 util.DataStream
--- @param arg6 enum udm::Type
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table, arg5: enum udm::Type): 
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table): 
--- @overload fun(arg2: enum udm::Type, arg3: table, arg4: enum udm::Type): 
--- @overload fun(arg2: enum udm::Type, arg3: table): 
function udm.PropertyWrapper:SetArrayValues(arg2, arg3, arg4, arg5, arg6) end

--- 
--- @return int ret0
function udm.PropertyWrapper:GetSize() end

--- 
function udm.PropertyWrapper:__tostring() end

--- 
--- @param arg2 enum udm::AsciiSaveFlags
--- @return any ret0
--- @overload fun(): any
function udm.PropertyWrapper:ToAscii(arg2) end

--- 
--- @param idx int
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(key: string): udm.LinkedPropertyWrapper
function udm.PropertyWrapper:Get(idx) end

--- 
--- @param arg2 string
--- @param arg3 util.DataStream
--- @overload fun(arg2: int, arg3: util.DataStream): 
--- @overload fun(key: string, v: udm.Reference): 
--- @overload fun(key: string, v: udm.HdrColor): 
--- @overload fun(key: string, v: udm.Srgba): 
--- @overload fun(key: string, v: string): 
--- @overload fun(key: string, v: math.Mat3x4): 
--- @overload fun(key: string, v: math.Mat4): 
--- @overload fun(key: string, v: math.ScaledTransform): 
--- @overload fun(key: string, v: math.Transform): 
--- @overload fun(key: string, v: math.EulerAngles): 
--- @overload fun(key: string, v: math.Quaternion): 
--- @overload fun(key: string, v: math.Vector4): 
--- @overload fun(key: string, v: math.Vector): 
--- @overload fun(key: string, v: math.Vector2): 
--- @overload fun(key: string, v: number): 
--- @overload fun(key: string, v: bool): 
--- @overload fun(idx: int, v: udm.Reference): 
--- @overload fun(idx: int, v: udm.HdrColor): 
--- @overload fun(idx: int, v: udm.Srgba): 
--- @overload fun(idx: int, v: string): 
--- @overload fun(idx: int, v: math.Mat3x4): 
--- @overload fun(idx: int, v: math.Mat4): 
--- @overload fun(idx: int, v: math.ScaledTransform): 
--- @overload fun(idx: int, v: math.Transform): 
--- @overload fun(idx: int, v: math.EulerAngles): 
--- @overload fun(idx: int, v: math.Quaternion): 
--- @overload fun(idx: int, v: math.Vector4): 
--- @overload fun(idx: int, v: math.Vector): 
--- @overload fun(idx: int, v: math.Vector2): 
--- @overload fun(idx: int, v: number): 
--- @overload fun(idx: int, v: bool): 
--- @overload fun(key: string, v: vector.Vector): 
--- @overload fun(idx: int, v: vector.Vector): 
function udm.PropertyWrapper:Set(arg2, arg3) end

--- 
--- @param arg2 string
--- @return int ret0
--- @overload fun(): int
function udm.PropertyWrapper:GetChildCount(arg2) end

--- 
--- @param arg2 string
--- @param arg3 enum udm::Type
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(arg2: string): udm.LinkedPropertyWrapper
function udm.PropertyWrapper:Add(arg2, arg3) end

--- 
--- @param arg2 string
--- @return udm.AssetData ret0
function udm.PropertyWrapper:AddAssetData(arg2) end

--- 
--- @return udm.AssetData ret0
function udm.PropertyWrapper:ToAssetData() end

--- 
--- @param arg2 int
--- @return any ret0
function udm.PropertyWrapper:GetArrayValueDirect(arg2) end

--- 
--- @param arg2 enum udm::Type
--- @return any ret0
function udm.PropertyWrapper:GetArrayValuesFromBlob(arg2) end

--- 
--- @param arg2 string
--- @param arg3 enum udm::Type
--- @param arg4 table
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: table, arg5: enum udm::Type): 
function udm.PropertyWrapper:AddBlobFromArrayValues(arg2, arg3, arg4) end

--- 
function udm.PropertyWrapper:GetBlobData() end

--- 
--- @param arg2 int
--- @return any ret0
--- @overload fun(arg2: int, arg3: enum udm::Type): any
function udm.PropertyWrapper:GetArrayValue(arg2) end

--- 
--- @param arg2 enum udm::Type
function udm.PropertyWrapper:SetValueType(arg2) end

--- 
--- @param arg2 string
--- @param arg3 int
--- @param arg4 enum udm::Type
--- @param arg5 enum udm::ArrayType
--- @return udm.LinkedPropertyWrapper ret0
--- @overload fun(arg2: string, arg3: int, arg4: enum udm::Type): udm.LinkedPropertyWrapper
--- @overload fun(arg2: string, arg3: int): udm.LinkedPropertyWrapper
function udm.PropertyWrapper:AddArray(arg2, arg3, arg4, arg5) end

--- 
--- @param arg2 string
--- @return udm.LinkedPropertyWrapper ret0
function udm.PropertyWrapper:GetFromPath(arg2) end

--- 
--- @param arg2 int
function udm.PropertyWrapper:Resize(arg2) end

--- 
--- @param arg2 string
--- @param arg3 any
--- @overload fun(arg2: string, arg3: enum udm::Type): 
--- @overload fun(arg2: int, arg3: enum udm::Type, arg4: any): 
--- @overload fun(arg2: string, arg3: enum udm::Type, arg4: any): 
function udm.PropertyWrapper:SetValue(arg2, arg3) end

--- 
--- @param arg2 udm.PropertyWrapper
--- @param arg3 enum udm::MergeFlags
--- @overload fun(arg2: udm.PropertyWrapper): 
function udm.PropertyWrapper:Merge(arg2, arg3) end


--- 
--- @class udm.Srgba
--- @field b int 
--- @field r int 
--- @field a int 
--- @field g int 
--- @overload fun():udm.Srgba
udm.Srgba = {}

--- 
function udm.Srgba:__tostring() end


--- 
--- @class udm.StructDescription
--- @overload fun():udm.StructDescription
udm.StructDescription = {}

--- 
--- @return any ret0
function udm.StructDescription:GetTypes() end

--- 
--- @return int ret0
function udm.StructDescription:GetMemberCount() end

--- 
--- @return any ret0
function udm.StructDescription:GetNames() end


--- 
--- @class pfm.udm
pfm.udm = {}


