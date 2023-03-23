--- @meta
--- 
--- @class file
file = {}

--- 
--- @param path string
--- @return any ret0
--- @overload fun(path: string, exts: table): any
function file.get_file_extension(path) end

--- 
--- @param str string
--- @return string ret0
function file.get_file_path(str) end

--- 
--- @param srcFile string
--- @param dstFile string
--- @return bool ret0
function file.copy(srcFile, dstFile) end

--- 
--- @param path string
--- @param outFiles any
--- @param outDirs any
function file.find_external_game_asset_files(path, outFiles, outDirs) end

--- 
--- @param path string
--- @return bool ret0
function file.create_path(path) end

--- 
--- @param path string
--- @return string ret0
function file.strip_illegal_filename_characters(path) end

--- 
--- @param path string
--- @return bool ret0
--- @overload fun(name: string, fsearchmode: enum fsys::SearchFlags): bool
function file.is_directory(path) end

--- 
--- @param contents string
--- @return file.File ret0
function file.create_virtual(contents) end

--- 
--- @param path string
--- @return bool ret0
--- @overload fun(path: string, searchFlags: enum fsys::SearchFlags): bool
function file.exists(path) end

--- 
--- @param path string
--- @return bool ret0
function file.create_directory(path) end

--- 
--- @param ppath string
--- @return bool ret0
function file.delete(ppath) end

--- 
--- @param path0 string
--- @param path1 string
--- @return bool ret0
function file.compare_path(path0, path1) end

--- 
--- @param ppath string
--- @return bool ret0
function file.delete_directory(ppath) end

--- 
--- @param path string
--- @return file.File ret0
--- @overload fun(path: string, game: string): file.File
function file.open_external_asset_file(path) end

--- 
--- @param path string
--- @return string ret0
function file.to_relative_path(path) end

--- 
--- @param path string
--- @return int ret0
--- @overload fun(name: string, fsearchmode: enum fsys::SearchFlags): int
function file.get_size(path) end

--- 
--- @param path string
--- @return any ret0
--- @overload fun(path: string, searchFlag: enum fsys::SearchFlags): any
function file.find_lua_files(path) end

--- 
--- @param name string
--- @return int ret0
function file.get_attributes(name) end

--- 
--- @param path string
--- @return int ret0
--- @overload fun(path: string, searchFlags: enum fsys::SearchFlags): int
function file.get_flags(path) end

--- 
--- @param path string
--- @return any ret0
function file.find_absolute_path(path) end

--- 
--- @param path string
--- @return string ret0
function file.get_canonicalized_path(path) end

--- 
--- @param str string
--- @return string ret0
function file.get_file_name(str) end

--- 
--- @param path string
--- @param outFiles any
--- @param outDirs any
--- @overload fun(path: string, searchFlags: enum fsys::SearchFlags, outFiles: any, outDirs: any): 
function file.find(path, outFiles, outDirs) end

--- 
--- @param path string
--- @param t 
--- @return string ret0
--- @overload fun(path: string): string
function file.remove_file_extension(path, t) end

--- 
--- @param path string
--- @return any ret0
function file.read(path) end

--- 
--- @param strPath string
--- @param content string
--- @return bool ret0
function file.write(strPath, content) end

--- 
--- @param path string
--- @param openMode enum FileOpenMode
--- @return file.File ret0
--- @overload fun(path: string, openMode: enum FileOpenMode, searchFlags: enum fsys::SearchFlags): file.File
function file.open(path, openMode) end

--- 
--- @param path string
--- @return bool ret0
function file.is_empty(path) end


--- 
--- @class file.File
file.File = {}

--- 
function file.File:GetPath() end

--- 
--- @param col math.Mat4x2
function file.File:WriteMat4x2(col) end

--- 
--- @overload fun(len: int): 
function file.File:ReadString() end

--- 
--- @return int ret0
function file.File:GetSize() end

--- 
--- @return math.Mat2x4 ret0
function file.File:ReadMat2x4() end

--- 
--- @param arg2 number
function file.File:WriteDouble(arg2) end

--- 
--- @param pos int
function file.File:Seek(pos) end

--- 
function file.File:Tell() end

--- 
function file.File:ReadBool() end

--- 
--- @param str string
--- @overload fun(str: string, bNullTerminated: bool): 
function file.File:WriteString(str) end

--- 
function file.File:ReadChar() end

--- 
--- @param ds util.DataStream
--- @param size int
--- @overload fun(ds: util.DataStream): 
function file.File:Write(ds, size) end

--- 
function file.File:ReadLine() end

--- 
--- @param d int
function file.File:WriteInt64(d) end

--- 
--- @param arg2 number
function file.File:WriteLongDouble(arg2) end

--- 
--- @param ds util.DataStream
--- @param size int
--- @overload fun(size: int): 
function file.File:Read(ds, size) end

--- 
--- @param arg2 int
function file.File:WriteUInt32(arg2) end

--- 
--- @param start string
--- @param end string
--- @overload fun(start: string): 
--- @overload fun(): 
function file.File:IgnoreComments(start, end_) end

--- 
--- @param arg2 bool
function file.File:WriteBool(arg2) end

--- 
function file.File:ReadInt64() end

--- 
--- @param rot math.Quaternion
function file.File:WriteQuaternion(rot) end

--- 
--- @return math.Mat2x3 ret0
function file.File:ReadMat2x3() end

--- 
--- @param d int
function file.File:WriteUInt64(d) end

--- 
--- @return math.Mat3x4 ret0
function file.File:ReadMat3x4() end

--- 
function file.File:ReadUInt64() end

--- 
--- @param arg2 int
function file.File:WriteInt8(arg2) end

--- 
function file.File:ReadInt8() end

--- 
--- @param arg2 int
function file.File:WriteUInt8(arg2) end

--- 
function file.File:Close() end

--- 
--- @return util.Color ret0
function file.File:ReadColor() end

--- 
--- @param d string
function file.File:WriteChar(d) end

--- 
--- @param col math.Mat2x4
function file.File:WriteMat2x4(col) end

--- 
function file.File:ReadInt32() end

--- 
--- @return string ret0
function file.File:__tostring() end

--- 
function file.File:ReadUInt32() end

--- 
--- @param col math.Mat2
function file.File:WriteMat2(col) end

--- 
--- @param arg2 int
function file.File:WriteInt16(arg2) end

--- 
function file.File:ReadLongDouble() end

--- 
function file.File:ReadInt16() end

--- 
function file.File:ReadAngles() end

--- 
--- @param arg2 int
function file.File:WriteUInt16(arg2) end

--- 
function file.File:ReadUInt16() end

--- 
--- @param arg2 number
function file.File:WriteFloat(arg2) end

--- 
function file.File:ReadFloat() end

--- 
function file.File:ReadDouble() end

--- 
--- @param v math.Vector
--- @overload fun(v: vector.Vector): 
function file.File:WriteVector(v) end

--- 
--- @param col math.Mat4x3
function file.File:WriteMat4x3(col) end

--- 
function file.File:ReadVector() end

--- 
function file.File:ReadVector4() end

--- 
--- @param v math.Vector2
function file.File:WriteVector2(v) end

--- 
function file.File:ReadUInt8() end

--- 
function file.File:ReadVector2() end

--- 
--- @param arg2 int
function file.File:WriteInt32(arg2) end

--- 
--- @param v math.Vector4
function file.File:WriteVector4(v) end

--- 
--- @param ang math.EulerAngles
function file.File:WriteAngles(ang) end

--- 
--- @param col math.Mat2x3
function file.File:WriteMat2x3(col) end

--- 
--- @return math.Mat2 ret0
function file.File:ReadMat2() end

--- 
--- @param col math.Mat3x2
function file.File:WriteMat3x2(col) end

--- 
--- @param col math.Mat3
function file.File:WriteMat3(col) end

--- 
--- @param col math.Mat3x4
function file.File:WriteMat3x4(col) end

--- 
--- @param col math.Mat4
function file.File:WriteMat4(col) end

--- 
--- @return math.Mat4x2 ret0
function file.File:ReadMat4x2() end

--- 
function file.File:Eof() end

--- 
--- @return math.Mat3x2 ret0
function file.File:ReadMat3x2() end

--- 
--- @return math.Mat3 ret0
function file.File:ReadMat3() end

--- 
--- @return math.Mat4x3 ret0
function file.File:ReadMat4x3() end

--- 
--- @return math.Mat4 ret0
function file.File:ReadMat4() end

--- 
function file.File:ReadQuaternion() end

--- 
--- @param col util.Color
function file.File:WriteColor(col) end

--- 
function file.File:Size() end


