--- @meta
--- 
--- @class asset
asset = {}

--- 
--- @param path string
--- @param type enum pragma::asset::Type
--- @return table ret0
function asset.find(path, type) end

--- 
--- @param nw class NetworkState
--- @param type enum pragma::asset::Type
--- @return int ret0
function asset.clear_unused(nw, type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return variant ret0
function asset.reload(name, type) end

--- 
--- @param name0 string
--- @param name1 string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.matches(name0, name1, type) end

--- 
--- @return int ret0
function asset.clear_flagged_models() end

--- 
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.get_type_identifier(type) end

--- 
--- @param mdl game.Model
function asset.flag_model_for_cache_removal(mdl) end

--- 
--- @param identifier string
--- @return enum pragma::asset::Type ret0
function asset.get_type_enum(identifier) end

--- 
--- @return int ret0
function asset.clear_unused_materials() end

--- 
--- @param arg0 class Engine
function asset.lock_asset_watchers(arg0) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.normalize_asset_name(name, type) end

--- 
--- @param arg0 class Engine
function asset.unlock_asset_watchers(arg0) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return enum util::AssetState ret0
function asset.get_asset_state(name, type) end

--- 
--- @param arg0 class Engine
function asset.poll_asset_watchers(arg0) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.wait_until_loaded(name, type) end

--- 
--- @param type enum pragma::asset::Type
--- @return table ret0
function asset.get_supported_import_file_extensions(type) end

--- 
--- @param type enum pragma::asset::Type
--- @return table ret0
function asset.get_supported_export_file_extensions(type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.get_normalized_path(name, type) end

--- 
--- @param ext string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.is_supported_extension(ext, type) end

--- 
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.get_legacy_extension(type) end

--- 
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.get_asset_root_directory(type) end

--- 
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.get_binary_udm_extension(type) end

--- 
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.get_ascii_udm_extension(type) end

--- 
--- @param f file.File
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.determine_format_from_data(f, type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
function asset.wait_until_all_pending_jobs_complete(name, type) end

--- 
--- @param fileName string
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.determine_format_from_filename(fileName, type) end

--- 
--- @param ext string
--- @return enum pragma::asset::Type ret0
function asset.determine_type_from_extension(ext) end

--- 
--- @param format0 string
--- @param format1 string
--- @return bool ret0
function asset.matches_format(format0, format1) end

--- 
--- @param path string
--- @param type enum pragma::asset::Type
--- @return string ret0
--- @overload fun(path: string, type: enum pragma::asset::Type, rootPath: string): string
function asset.relative_path_to_absolute_path(path, type) end

--- 
--- @param path string
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.absolute_path_to_relative_path(path, type) end

--- 
--- @param type enum pragma::asset::Type
--- @param binary bool
--- @return string ret0
function asset.get_udm_format_extension(type, binary) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return string ret0
function asset.find_file(name, type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.is_loaded(name, type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return variant ret0
function asset.precache(name, type) end

--- 
--- @param type enum pragma::asset::Type
function asset.poll(type) end

--- 
function asset.poll_all() end

--- 
--- @param type enum pragma::asset::Type
--- @param formatType enum pragma::asset::FormatType
--- @return table ret0
--- @overload fun(type: enum pragma::asset::Type): table
function asset.get_supported_extensions(type, formatType) end

--- 
--- @return int ret0
function asset.clear_unused_models() end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return variant ret0
--- @overload fun(f: file.File, type: enum pragma::asset::Type): variant
function asset.load(name, type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.delete(name, type) end

--- 
--- @param name string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.exists(name, type) end

--- 
--- @return int ret0
function asset.clear_unused_textures() end

--- 
--- @param nw class NetworkState
--- @param name string
--- @param type enum pragma::asset::Type
--- @return bool ret0
function asset.import(nw, name, type) end


--- 
--- @class asset.MapExportInfo
--- @field includeMapLightSources bool 
--- @overload fun():asset.MapExportInfo
asset.MapExportInfo = {}

--- 
--- @param arg1 ents.LightComponent
function asset.MapExportInfo:AddLightSource(arg1) end

--- 
--- @param arg1 ents.CameraComponent
function asset.MapExportInfo:AddCamera(arg1) end


--- 
--- @class asset.TextureImportInfo
--- @field srgb bool 
--- @field normalMap bool 
--- @overload fun():asset.TextureImportInfo
asset.TextureImportInfo = {}


