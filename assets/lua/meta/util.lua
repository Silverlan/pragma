--- @meta
--- 
--- @class prosper.util
prosper.util = {}

--- 
--- @return table ret0
function prosper.util.get_square_uv_coordinates() end

--- 
--- @return int ret0
function prosper.util.get_square_vertex_count() end

--- 
--- @return enum prosper::Format ret0
function prosper.util.get_square_vertex_format() end

--- 
--- @return prosper.Buffer ret0
function prosper.util.get_line_vertex_buffer() end

--- 
--- @return enum prosper::Format ret0
function prosper.util.get_square_uv_format() end

--- 
--- @return enum prosper::Format ret0
function prosper.util.get_line_vertex_format() end

--- 
--- @param size int
--- @return prosper.Buffer ret0
--- @overload fun(ds: util.DataStream): prosper.Buffer
function prosper.util.allocate_temporary_buffer(size) end

--- 
--- @return int ret0
function prosper.util.get_line_vertex_count() end

--- 
--- @return table ret0
function prosper.util.get_line_vertices() end

--- 
--- @param cmd prosper.CommandBuffer
--- @param dsg prosper.DescriptorSet
--- @param dstRt prosper.RenderTarget
function prosper.util.record_resize_image(cmd, dsg, dstRt) end

--- 
--- @param tex prosper.Texture
--- @return prosper.DescriptorSet ret0
function prosper.util.create_generic_image_descriptor_set(tex) end

--- 
--- @return prosper.Buffer ret0
function prosper.util.get_square_vertex_uv_buffer() end

--- 
--- @return prosper.Buffer ret0
function prosper.util.get_square_vertex_buffer() end

--- 
--- @return prosper.Buffer ret0
function prosper.util.get_square_uv_buffer() end

--- 
--- @return table ret0
function prosper.util.get_square_vertices() end


--- 
--- @class util
util = {}

--- 
--- @return bool ret0
--- @overload fun(arg: any): bool
function util.is_table() end

--- 
--- @param o userData
--- @return int ret0
function util.get_object_hash(o) end

--- 
--- @param cur number
--- @param dur number
--- @param fadeIn number
--- @return number ret0
--- @overload fun(cur: number, dur: number, fadeIn: number, fadeOut: number): number
function util.get_faded_time_factor(cur, dur, fadeIn) end

--- 
--- @param o any
--- @return bool ret0
--- @overload fun(): bool
function util.is_valid(o) end

--- 
--- @param val number
--- @param min number
--- @return number ret0
--- @overload fun(val: number, min: number, max: number): number
function util.get_scale_factor(val, min) end

--- 
--- @param cubemap prosper.Texture
--- @return any ret0
function util.cubemap_to_equirectangular_texture(cubemap) end

--- 
--- @param spath string
--- @overload fun(spath: string, selectFile: string): 
function util.open_path_in_explorer(spath) end

--- 
--- @param bytes int
--- @return string ret0
function util.get_pretty_bytes(bytes) end

--- 
--- @param t number
--- @return string ret0
function util.get_pretty_time(t) end

--- 
--- @param units number
--- @return number ret0
function util.units_to_metres(units) end

--- 
--- @param vLocal math.Vector
--- @param rLocal math.Quaternion
--- @param v math.Vector
--- @param r math.Quaternion
--- @overload fun(vLocal: math.Vector, rLocal: math.Quaternion, v: math.Vector): 
--- @overload fun(rLocal: math.Quaternion, r: math.Quaternion): 
function util.world_to_local(vLocal, rLocal, v, r) end

--- 
--- @param fileName string
--- @return any ret0
function util.read_scene_file(fileName) end

--- 
--- @param metres number
--- @return number ret0
function util.metres_to_units(metres) end

--- 
--- @param varType enum util::VarType
--- @return string ret0
function util.variable_type_to_string(varType) end

--- 
--- @param url string
function util.open_url_in_browser(url) end

--- 
--- @return string ret0
function util.get_addon_path() end

--- 
--- @param str string
--- @return string ret0
function util.get_string_hash(str) end

--- 
--- @param str string
--- @return util.Uuid ret0
--- @overload fun(seed: int): util.Uuid
--- @overload fun(): util.Uuid
function util.generate_uuid_v4(str) end

--- 
--- @param t 
--- @param className string
--- @return any ret0
--- @overload fun(t: , className: string, oBase0: any): any
--- @overload fun(t: , className: string, oBase0: any, oBase1: any): any
--- @overload fun(t: , className: string, oBase0: any, oBase1: any, oBase2: any): any
--- @overload fun(t: , className: string, oBase0: any, oBase1: any, oBase2: any, oBase3: any): any
--- @overload fun(t: , className: string, oBase0: any, oBase1: any, oBase2: any, oBase3: any, oBase4: any): any
--- @overload fun(pclassName: string): any
--- @overload fun(className: string, oBase0: any): any
--- @overload fun(className: string, oBase0: any, oBase1: any): any
--- @overload fun(className: string, oBase0: any, oBase1: any, oBase2: any): any
--- @overload fun(className: string, oBase0: any, oBase1: any, oBase2: any, oBase3: any): any
--- @overload fun(className: string, oBase0: any, oBase1: any, oBase2: any, oBase3: any, oBase4: any): any
function util.register_class(t, className) end

--- 
--- @param fileName string
--- @return any ret0
--- @overload fun(fileName: string, loadAsynch: bool): any
--- @overload fun(fileName: string, loadAsynch: bool, targetFormat: enum uimg::Format): any
function util.load_image(fileName) end

--- 
--- @param vLocal math.Vector
--- @param rLocal math.Quaternion
--- @param v math.Vector
--- @param r math.Quaternion
--- @overload fun(vLocal: math.Vector, rLocal: math.Quaternion, v: math.Vector): 
--- @overload fun(r0: math.Quaternion, r1: math.Quaternion): 
function util.local_to_world(vLocal, rLocal, v, r) end

--- 
--- @param bulletInfo game.BulletInfo
--- @return any ret0
--- @overload fun(bulletInfo: game.BulletInfo, hitReport: bool): any
--- @overload fun(bulletInfo: game.BulletInfo): any
function util.fire_bullets(bulletInfo) end

--- 
--- @param ms int
--- @return string ret0
--- @overload fun(ms: int, segments: int): string
--- @overload fun(ms: int, segments: int, noMs: bool): string
function util.get_pretty_duration(ms) end

--- 
--- @param  struct luabind::detail::nil_type
--- @overload fun(arg: struct luabind::detail::nil_type, bool: bool): 
--- @overload fun(): 
--- @overload fun(o: any, removeSafely: bool): 
--- @overload fun(o: any): 
function util.remove(arg) end

--- 
--- @param vProp util.FloatProperty
--- @param vDst number
--- @param duration number
--- @return any ret0
--- @overload fun(vProp: util.IntProperty, vDst: int, duration: number): any
--- @overload fun(vProp: util.EulerAnglesProperty, vDst: math.EulerAngles, duration: number): any
--- @overload fun(vProp: util.QuaternionProperty, vDst: math.Quaternion, duration: number): any
--- @overload fun(vProp: util.Vector4iProperty, vDst: math.Vector4i, duration: number): any
--- @overload fun(vProp: util.Vector4Property, vDst: math.Vector4, duration: number): any
--- @overload fun(vProp: util.VectoriProperty, vDst: math.Vectori, duration: number): any
--- @overload fun(vProp: util.VectorProperty, vDst: math.Vector, duration: number): any
--- @overload fun(vProp: util.Vector2iProperty, vDst: math.Vector2i, duration: number): any
--- @overload fun(colProp: util.ColorProperty, colDst: util.Color, duration: number): any
function util.fade_property(vProp, vDst, duration) end

--- 
--- @param value number
--- @return string ret0
--- @overload fun(value: number, places: int): string
function util.round_string(value) end

--- 
--- @param o any
--- @return string ret0
function util.get_type_name(o) end

--- 
--- @param lights table
--- @param meshes table
--- @param entities table
--- @param width int
--- @param height int
--- @param optLightmapDataCache ents.LightMapComponent.DataCache
--- @return util.ParallelJobImage ret0
function util.bake_directional_lightmap_atlas(lights, meshes, entities, width, height, optLightmapDataCache) end

--- 
--- @param o0 any
--- @param o1 any
--- @return bool ret0
function util.is_same_object(o0, o1) end

--- 
--- @param point math.Vector
--- @param vp math.Mat4
--- @param nearZ number
--- @param farZ number
--- @return number ret0
function util.calc_screen_space_distance_to_world_space_position(point, vp, nearZ, farZ) end

--- 
--- @param w int
--- @param h int
--- @param aspectRatio number
function util.clamp_resolution_to_aspect_ratio(w, h, aspectRatio) end

--- 
--- @return int ret0
function util.create_muzzle_flash() end

--- 
--- @param oClass any
--- @param key string
--- @return any ret0
function util.get_class_value(oClass, key) end

--- 
--- @param o any
--- @return bool ret0
--- @overload fun(): bool
function util.is_valid_entity(o) end

--- 
--- @param width int
--- @param height int
--- @return util.ParallelJobImageLayerSet ret0
--- @overload fun(width: int, height: int, samples: int): util.ParallelJobImageLayerSet
--- @overload fun(width: int, height: int, samples: int, hdrOutput: bool): util.ParallelJobImageLayerSet
--- @overload fun(width: int, height: int, samples: int, hdrOutput: bool, denoise: bool): util.ParallelJobImageLayerSet
function util.capture_raytraced_screenshot(width, height) end

--- 
--- @param format enum uimg::ImageFormat
--- @return string ret0
function util.get_image_format_file_extension(format) end

--- 
--- @param pzipFileName string
--- @param t 
function util.pack_zip_archive(pzipFileName, t) end

--- 
--- @param point math.Vector
--- @param vp math.Mat4
--- @param nearZ number
--- @param farZ number
function util.world_space_point_to_screen_space_uv(point, vp, nearZ, farZ) end

--- 
--- @param dir math.Vector
--- @param vp math.Mat4
--- @return math.Vector2 ret0
function util.world_space_direction_to_screen_space(dir, vp) end

--- 
--- @param depth number
--- @param nearZ number
--- @param farZ number
--- @return number ret0
function util.depth_to_distance(depth, nearZ, farZ) end

--- 
--- @param hairPerArea number
--- @param mesh game.Model.Mesh.Sub
--- @return util.HairData ret0
function util.generate_hair_data(hairPerArea, mesh) end

--- 
--- @param hairConfig util.HairConfig
--- @param hairData util.HairData
--- @return util.HairStrandData ret0
--- @overload fun(hairConfig: unirender.Shader.HairConfig, hairData: util.HairData): util.HairStrandData
function util.generate_hair_file(hairConfig, hairData) end

--- 
--- @param amplitude number
--- @param frequency number
--- @param duration number
--- @param fadeIn number
--- @param fadeOut number
--- @overload fun(pos: math.Vector, radius: number, amplitude: number, frequency: number, duration: number, fadeIn: number, fadeOut: number): 
function util.shake_screen(amplitude, frequency, duration, fadeIn, fadeOut) end

--- 
--- @param gibletInfo class GibletCreateInfo
--- @overload fun(createInfo: game.GibletCreateInfo): 
function util.create_giblet(gibletInfo) end

--- 
--- @param splashDamageInfo util.SplashDamageInfo
function util.create_explosion(splashDamageInfo) end

--- 
--- @param forward math.Vector
--- @param right math.Vector
--- @param up math.Vector
--- @param fov number
--- @param nearZ number
--- @param farZ number
--- @param aspectRatio number
--- @param uv math.Vector2
--- @return math.Vector ret0
--- @overload fun(hCam: ents.CameraComponent, uv: math.Vector2): math.Vector
function util.calc_world_direction_from_2d_coordinates(forward, right, up, fov, nearZ, farZ, aspectRatio, uv) end

--- 
--- @return string ret0
--- @overload fun(format: string): string
function util.get_date_time() end

--- 
--- @param splashDamageInfo util.SplashDamageInfo
function util.splash_damage(splashDamageInfo) end

--- 
--- @return int ret0
function util.create_particle_tracer() end

--- 
--- @param img prosper.Image
--- @param fileName string
--- @param imgWriteInfo util.TextureInfo
--- @return bool ret0
--- @overload fun(t: , fileName: string, texInfo: util.TextureInfo): bool
--- @overload fun(t: , fileName: string, texInfo: util.TextureInfo, cubemap: bool): bool
--- @overload fun(imgBuffer: util.ImageBuffer, fileName: string, format: enum uimg::ImageFormat): bool
--- @overload fun(imgBuffer: util.ImageBuffer, fileName: string, format: enum uimg::ImageFormat, quality: number, tw: struct pragma::lua::LuaThreadWrapper): bool
--- @overload fun(imgBuffer: util.ImageBuffer, fileName: string, format: enum uimg::ImageFormat, quality: number): bool
--- @overload fun(imgBuffer: util.ImageBuffer, fileName: string, imgWriteInfo: util.TextureInfo, tw: struct pragma::lua::LuaThreadWrapper): bool
--- @overload fun(imgBuffer: util.ImageBuffer, fileName: string, imgWriteInfo: util.TextureInfo, cubemap: bool): bool
--- @overload fun(imgBuffer: util.ImageBuffer, fileName: string, imgWriteInfo: util.TextureInfo, cubemap: bool): bool
function util.save_image(img, fileName, imgWriteInfo) end

--- 
--- @return string ret0
function util.get_clipboard_string() end

--- 
--- @param str string
function util.set_clipboard_string(str) end


--- 
--- @class util.BoolProperty: util.BaseProperty
--- @overload fun(arg1: bool):util.BoolProperty
--- @overload fun():util.BoolProperty
util.BoolProperty = {}

--- 
function util.BoolProperty:__tostring() end

--- 
--- @param oCallback any
function util.BoolProperty:AddCallback(oCallback) end

--- 
function util.BoolProperty:Get() end

--- 
function util.BoolProperty:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.BoolProperty:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.BoolProperty:SetLocked(bLocked) end

--- 
function util.BoolProperty:Unlink() end

--- 
function util.BoolProperty:IsLocked() end

--- 
--- @param val bool
function util.BoolProperty:Set(val) end

--- 
--- @param propOther util.IntProperty
--- @overload fun(propOther: util.FloatProperty): 
--- @overload fun(propOther: util.StringProperty): 
--- @overload fun(propOther: util.BoolProperty): 
function util.BoolProperty:Link(propOther) end


--- 
--- @class util.DirectoryChangeListener
util.DirectoryChangeListener = {}

--- 
--- @param enabled bool
function util.DirectoryChangeListener:SetEnabled(enabled) end

--- 
--- @return int ret0
function util.DirectoryChangeListener:Poll() end

--- 
--- @return int ret0
function util.DirectoryChangeListener:__tostring() end

--- 
--- @return bool ret0
function util.DirectoryChangeListener:IsEnabled() end


--- @enum ListenerFlag
util.DirectoryChangeListener = {
	LISTENER_FLAG_WATCH_DIRECTORY_CHANGES = 8,
	LISTENER_FLAG_ABSOLUTE_PATH = 2,
	LISTENER_FLAG_BIT_WATCH_SUB_DIRECTORIES = 1,
	LISTENER_FLAG_NONE = 0,
	LISTENER_FLAG_START_DISABLED = 4,
}

--- 
--- @class util.Mat4Vector
--- @overload fun():util.Mat4Vector
util.Mat4Vector = {}

--- 
--- @return math.Mat4 ret0
function util.Mat4Vector:Back() end

--- 
--- @return int ret0
function util.Mat4Vector:__len() end

--- 
--- @param name int
function util.Mat4Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat4 ret0
function util.Mat4Vector:Front() end

--- 
--- @return table ret0
function util.Mat4Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat4Vector:Capacity() end

--- 
--- @param name table
function util.Mat4Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4
function util.Mat4Vector:Set(arg1, arg2) end

--- 
function util.Mat4Vector:Clear() end

--- 
--- @return int ret0
function util.Mat4Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4
function util.Mat4Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat4 ret0
function util.Mat4Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4
--- @overload fun(name: int): 
function util.Mat4Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat4Vector:Reserve(name) end

--- 
--- @param name math.Mat4
function util.Mat4Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat4Vector:Erase(name) end


--- 
--- @class util.ParallelJobString: util.ParallelJob
util.ParallelJobString = {}

--- 
function util.ParallelJobString:GetResult() end


--- 
--- @class util.Mat3Property: util.BaseProperty
--- @overload fun(arg1: math.Mat3):util.Mat3Property
--- @overload fun():util.Mat3Property
util.Mat3Property = {}

--- 
function util.Mat3Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat3Property:AddCallback(oCallback) end

--- 
function util.Mat3Property:Get() end

--- 
function util.Mat3Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat3Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat3Property:SetLocked(bLocked) end

--- 
function util.Mat3Property:Unlink() end

--- 
function util.Mat3Property:IsLocked() end

--- 
--- @param val math.Mat3
function util.Mat3Property:Set(val) end

--- 
--- @param propOther util.Mat3Property
function util.Mat3Property:Link(propOther) end


--- 
--- @class util.Mat2x3Property: util.BaseProperty
--- @overload fun(arg1: math.Mat2x3):util.Mat2x3Property
--- @overload fun():util.Mat2x3Property
util.Mat2x3Property = {}

--- 
function util.Mat2x3Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat2x3Property:AddCallback(oCallback) end

--- 
function util.Mat2x3Property:Get() end

--- 
function util.Mat2x3Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat2x3Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat2x3Property:SetLocked(bLocked) end

--- 
function util.Mat2x3Property:Unlink() end

--- 
function util.Mat2x3Property:IsLocked() end

--- 
--- @param val math.Mat2x3
function util.Mat2x3Property:Set(val) end

--- 
--- @param propOther util.Mat2x3Property
function util.Mat2x3Property:Link(propOther) end


--- 
--- @class util.LongDoubleVector
--- @overload fun():util.LongDoubleVector
util.LongDoubleVector = {}

--- 
--- @return number ret0
function util.LongDoubleVector:Back() end

--- 
--- @return int ret0
function util.LongDoubleVector:__len() end

--- 
--- @param name int
function util.LongDoubleVector:ShrinkToFit(name) end

--- 
--- @return number ret0
function util.LongDoubleVector:Front() end

--- 
--- @return table ret0
function util.LongDoubleVector:ToTable() end

--- 
--- @return int ret0
function util.LongDoubleVector:Capacity() end

--- 
--- @param name table
function util.LongDoubleVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 number
function util.LongDoubleVector:Set(arg1, arg2) end

--- 
function util.LongDoubleVector:Clear() end

--- 
--- @return int ret0
function util.LongDoubleVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 number
function util.LongDoubleVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return number ret0
function util.LongDoubleVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 number
--- @overload fun(name: int): 
function util.LongDoubleVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.LongDoubleVector:Reserve(name) end

--- 
--- @param name number
function util.LongDoubleVector:PushBack(name) end

--- 
--- @param name int
function util.LongDoubleVector:Erase(name) end


--- 
--- @class util.Vector3iVector
--- @overload fun():util.Vector3iVector
util.Vector3iVector = {}

--- 
--- @return math.Vectori ret0
function util.Vector3iVector:Back() end

--- 
--- @return int ret0
function util.Vector3iVector:__len() end

--- 
--- @param name int
function util.Vector3iVector:ShrinkToFit(name) end

--- 
--- @return math.Vectori ret0
function util.Vector3iVector:Front() end

--- 
--- @return table ret0
function util.Vector3iVector:ToTable() end

--- 
--- @return int ret0
function util.Vector3iVector:Capacity() end

--- 
--- @param name table
function util.Vector3iVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vectori
function util.Vector3iVector:Set(arg1, arg2) end

--- 
function util.Vector3iVector:Clear() end

--- 
--- @return int ret0
function util.Vector3iVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Vectori
function util.Vector3iVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Vectori ret0
function util.Vector3iVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vectori
--- @overload fun(name: int): 
function util.Vector3iVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Vector3iVector:Reserve(name) end

--- 
--- @param name math.Vectori
function util.Vector3iVector:PushBack(name) end

--- 
--- @param name int
function util.Vector3iVector:Erase(name) end


--- 
--- @class util.ImageLayerSet
util.ImageLayerSet = {}

--- 
function util.ImageLayerSet:__tostring() end

--- 
--- @param name string
--- @return util.ImageBuffer ret0
function util.ImageLayerSet:GetImage(name) end

--- 
--- @return map ret0
function util.ImageLayerSet:GetImages() end


--- 
--- @class util.ParallelJobImageLayerSet: util.ParallelJob
util.ParallelJobImageLayerSet = {}

--- 
--- @return util.ImageBuffer ret0
function util.ParallelJobImageLayerSet:GetImage() end

--- 
function util.ParallelJobImageLayerSet:GetResult() end


--- 
--- @class util.Mat3x4Vector
--- @overload fun():util.Mat3x4Vector
util.Mat3x4Vector = {}

--- 
--- @return math.Mat3x4 ret0
function util.Mat3x4Vector:Back() end

--- 
--- @return int ret0
function util.Mat3x4Vector:__len() end

--- 
--- @param name int
function util.Mat3x4Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat3x4 ret0
function util.Mat3x4Vector:Front() end

--- 
--- @return table ret0
function util.Mat3x4Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat3x4Vector:Capacity() end

--- 
--- @param name table
function util.Mat3x4Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3x4
function util.Mat3x4Vector:Set(arg1, arg2) end

--- 
function util.Mat3x4Vector:Clear() end

--- 
--- @return int ret0
function util.Mat3x4Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3x4
function util.Mat3x4Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat3x4 ret0
function util.Mat3x4Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3x4
--- @overload fun(name: int): 
function util.Mat3x4Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat3x4Vector:Reserve(name) end

--- 
--- @param name math.Mat3x4
function util.Mat3x4Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat3x4Vector:Erase(name) end


--- 
--- @class util.ShortVector
--- @overload fun():util.ShortVector
util.ShortVector = {}

--- 
--- @return int ret0
function util.ShortVector:Back() end

--- 
--- @return int ret0
function util.ShortVector:__len() end

--- 
--- @param name int
function util.ShortVector:ShrinkToFit(name) end

--- 
--- @return int ret0
function util.ShortVector:Front() end

--- 
--- @return table ret0
function util.ShortVector:ToTable() end

--- 
--- @return int ret0
function util.ShortVector:Capacity() end

--- 
--- @param name table
function util.ShortVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 int
function util.ShortVector:Set(arg1, arg2) end

--- 
function util.ShortVector:Clear() end

--- 
--- @return int ret0
function util.ShortVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 int
function util.ShortVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return int ret0
function util.ShortVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 int
--- @overload fun(name: int): 
function util.ShortVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.ShortVector:Reserve(name) end

--- 
--- @param name int
function util.ShortVector:PushBack(name) end

--- 
--- @param name int
function util.ShortVector:Erase(name) end


--- 
--- @class util.Uuid
--- @overload fun(args: string):util.Uuid
util.Uuid = {}

--- 
--- @return bool ret0
function util.Uuid:IsValid() end

--- 
--- @return string ret0
function util.Uuid:__tostring() end


--- 
--- @class util.ParallelJobImage: util.ParallelJob
util.ParallelJobImage = {}

--- 
function util.ParallelJobImage:GetResult() end


--- 
--- @class util.EulerAnglesVector
--- @overload fun():util.EulerAnglesVector
util.EulerAnglesVector = {}

--- 
--- @return math.EulerAngles ret0
function util.EulerAnglesVector:Back() end

--- 
--- @return int ret0
function util.EulerAnglesVector:__len() end

--- 
--- @param name int
function util.EulerAnglesVector:ShrinkToFit(name) end

--- 
--- @return math.EulerAngles ret0
function util.EulerAnglesVector:Front() end

--- 
--- @return table ret0
function util.EulerAnglesVector:ToTable() end

--- 
--- @return int ret0
function util.EulerAnglesVector:Capacity() end

--- 
--- @param name table
function util.EulerAnglesVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.EulerAngles
function util.EulerAnglesVector:Set(arg1, arg2) end

--- 
function util.EulerAnglesVector:Clear() end

--- 
--- @return int ret0
function util.EulerAnglesVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.EulerAngles
function util.EulerAnglesVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.EulerAngles ret0
function util.EulerAnglesVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.EulerAngles
--- @overload fun(name: int): 
function util.EulerAnglesVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.EulerAnglesVector:Reserve(name) end

--- 
--- @param name math.EulerAngles
function util.EulerAnglesVector:PushBack(name) end

--- 
--- @param name int
function util.EulerAnglesVector:Erase(name) end


--- 
--- @class util.ColorProperty: util.BaseProperty
--- @overload fun(arg1: math.Vector4):util.ColorProperty
--- @overload fun(arg1: math.Vector):util.ColorProperty
--- @overload fun(arg1: string):util.ColorProperty
--- @overload fun(arg1: int, arg2: int, arg3: int, arg4: int):util.ColorProperty
--- @overload fun(arg1: util.Color):util.ColorProperty
--- @overload fun():util.ColorProperty
util.ColorProperty = {}

--- 
--- @param 1 number
function util.ColorProperty:__div(arg1) end

--- 
function util.ColorProperty:Unlink() end

--- 
--- @param val util.Color
function util.ColorProperty:Set(val) end

--- 
--- @param oCallback any
function util.ColorProperty:AddCallback(oCallback) end

--- 
function util.ColorProperty:__tostring() end

--- 
function util.ColorProperty:InvokeCallbacks() end

--- 
function util.ColorProperty:Get() end

--- 
--- @param oCallback any
function util.ColorProperty:AddModifier(oCallback) end

--- 
--- @param 1 util.ColorProperty
--- @overload fun(arg1: util.Color): 
--- @overload fun(arg1: util.ColorProperty): 
function util.ColorProperty:__eq(arg1) end

--- 
--- @param bLocked bool
function util.ColorProperty:SetLocked(bLocked) end

--- 
function util.ColorProperty:IsLocked() end

--- 
--- @param propOther util.Vector4Property
--- @overload fun(propOther: util.VectorProperty): 
--- @overload fun(propOther: util.StringProperty): 
--- @overload fun(propOther: util.ColorProperty): 
function util.ColorProperty:Link(propOther) end

--- 
--- @param 1 util.ColorProperty
--- @overload fun(arg1: util.ColorProperty): 
--- @overload fun(arg1: util.Color): 
function util.ColorProperty:__add(arg1) end

--- 
--- @param 1 util.ColorProperty
--- @overload fun(arg1: util.ColorProperty): 
--- @overload fun(arg1: util.Color): 
function util.ColorProperty:__sub(arg1) end

--- 
--- @param 1 util.ColorProperty
--- @overload fun(arg1: number): 
function util.ColorProperty:__mul(arg1) end


--- 
--- @class util.BaseProperty
util.BaseProperty = {}


--- 
--- @class util.Vector4iProperty: util.BaseProperty
--- @overload fun(arg1: int, arg2: int, arg3: int, arg4: int):util.Vector4iProperty
--- @overload fun(arg1: math.Vector4i):util.Vector4iProperty
--- @overload fun():util.Vector4iProperty
util.Vector4iProperty = {}

--- 
--- @param 1 number
function util.Vector4iProperty:__div(arg1) end

--- 
function util.Vector4iProperty:__tostring() end

--- 
--- @param oCallback any
function util.Vector4iProperty:AddCallback(oCallback) end

--- 
function util.Vector4iProperty:InvokeCallbacks() end

--- 
function util.Vector4iProperty:Get() end

--- 
--- @param 1 util.Vector4iProperty
--- @overload fun(arg1: math.Vector4i): 
--- @overload fun(arg1: util.Vector4iProperty): 
function util.Vector4iProperty:__eq(arg1) end

--- 
--- @param oCallback any
function util.Vector4iProperty:AddModifier(oCallback) end

--- 
function util.Vector4iProperty:Unlink() end

--- 
--- @param val math.Vector4i
function util.Vector4iProperty:Set(val) end

--- 
--- @param bLocked bool
function util.Vector4iProperty:SetLocked(bLocked) end

--- 
function util.Vector4iProperty:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.Vector4Property): 
--- @overload fun(propOther: util.Vector4iProperty): 
function util.Vector4iProperty:Link(propOther) end

--- 
--- @param 1 util.Vector4iProperty
--- @overload fun(arg1: util.Vector4iProperty): 
--- @overload fun(arg1: math.Vector4i): 
function util.Vector4iProperty:__add(arg1) end

--- 
--- @param 1 util.Vector4iProperty
--- @overload fun(arg1: util.Vector4iProperty): 
--- @overload fun(arg1: math.Vector4i): 
function util.Vector4iProperty:__sub(arg1) end

--- 
--- @param 1 util.Vector4iProperty
--- @overload fun(arg1: number): 
function util.Vector4iProperty:__mul(arg1) end


--- 
--- @class util.IntVector
--- @overload fun():util.IntVector
util.IntVector = {}

--- 
--- @return int ret0
function util.IntVector:Back() end

--- 
--- @return int ret0
function util.IntVector:__len() end

--- 
--- @param name int
function util.IntVector:ShrinkToFit(name) end

--- 
--- @return int ret0
function util.IntVector:Front() end

--- 
--- @return table ret0
function util.IntVector:ToTable() end

--- 
--- @return int ret0
function util.IntVector:Capacity() end

--- 
--- @param name table
function util.IntVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 int
function util.IntVector:Set(arg1, arg2) end

--- 
function util.IntVector:Clear() end

--- 
--- @return int ret0
function util.IntVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 int
function util.IntVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return int ret0
function util.IntVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 int
--- @overload fun(name: int): 
function util.IntVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.IntVector:Reserve(name) end

--- 
--- @param name int
function util.IntVector:PushBack(name) end

--- 
--- @param name int
function util.IntVector:Erase(name) end


--- 
--- @class util.HairStrandData
util.HairStrandData = {}

--- 
--- @param outData udm.AssetData
--- @return bool ret0
function util.HairStrandData:Save(outData) end

--- 
--- @param idx int
--- @return math.Vector2 ret0
function util.HairStrandData:GetStrandUv(idx) end

--- 
--- @param data udm.AssetData
--- @return bool ret0
function util.HairStrandData:Load(data) end

--- 
--- @return int ret0
function util.HairStrandData:GetStrandCount() end

--- 
--- @param idx int
--- @return int ret0
function util.HairStrandData:GetSegmentCount(idx) end

--- 
--- @param idx int
--- @return math.Vector ret0
function util.HairStrandData:GetStrandPoint(idx) end

--- 
--- @param idx int
--- @return number ret0
function util.HairStrandData:GetStrandThickness(idx) end


--- 
--- @class util.ParallelJob
util.ParallelJob = {}

--- 
--- @return bool ret0
function util.ParallelJob:IsThreadActive() end

--- 
--- @return int ret0
function util.ParallelJob:GetResultCode() end

--- 
--- @return number ret0
function util.ParallelJob:GetProgress() end

--- 
--- @return bool ret0
function util.ParallelJob:Poll() end

--- 
--- @return enum util::JobStatus ret0
function util.ParallelJob:GetStatus() end

--- 
--- @return bool ret0
function util.ParallelJob:IsValid() end

--- 
--- @return string ret0
function util.ParallelJob:GetResultMessage() end

--- 
function util.ParallelJob:__tostring() end

--- 
function util.ParallelJob:Start() end

--- 
function util.ParallelJob:Cancel() end

--- 
function util.ParallelJob:Wait() end

--- 
--- @return bool ret0
function util.ParallelJob:IsComplete() end

--- 
--- @return bool ret0
function util.ParallelJob:IsPending() end

--- 
--- @return bool ret0
function util.ParallelJob:IsCancelled() end

--- 
--- @return bool ret0
function util.ParallelJob:IsSuccessful() end


--- @enum JobStatus
util.ParallelJob = {
	JOB_STATUS_PENDING = 4,
	JOB_STATUS_INVALID = 255,
	JOB_STATUS_CANCELLED = 3,
	JOB_STATUS_FAILED = 0,
	JOB_STATUS_INITIAL = 2,
	JOB_STATUS_SUCCESSFUL = 1,
}

--- 
--- @class util.CallbackHandler
--- @overload fun():util.CallbackHandler
util.CallbackHandler = {}

--- 
--- @param name string
--- @param o any
function util.CallbackHandler:AddCallback(name, o) end

--- 
function util.CallbackHandler:__tostring() end

--- 
--- @param name string
--- @param o1 any
--- @param o2 any
--- @param o3 any
--- @param o4 any
--- @param o5 any
--- @param o6 any
--- @param o7 any
--- @param o8 any
--- @overload fun(name: string, o1: any, o2: any, o3: any, o4: any, o5: any, o6: any, o7: any): 
--- @overload fun(name: string, o1: any, o2: any, o3: any, o4: any, o5: any, o6: any): 
--- @overload fun(name: string, o1: any, o2: any, o3: any, o4: any, o5: any): 
--- @overload fun(name: string, o1: any, o2: any, o3: any, o4: any): 
--- @overload fun(name: string, o1: any, o2: any, o3: any): 
--- @overload fun(name: string, o1: any, o2: any): 
--- @overload fun(name: string, o1: any): 
--- @overload fun(name: string): 
function util.CallbackHandler:CallCallbacks(name, o1, o2, o3, o4, o5, o6, o7, o8) end

--- 
--- @param 1 util.CallbackHandler
function util.CallbackHandler:__eq(arg1) end


--- 
--- @class util.Path
--- @overload fun(arg1: string):util.Path
--- @overload fun(arg1: util.Path):util.Path
--- @overload fun():util.Path
util.Path = {}

--- 
function util.Path:MoveUp() end

--- 
--- @return int ret0
function util.Path:GetComponentCount() end

--- 
--- @param 1 string
--- @overload fun(arg1: util.Path): 
function util.Path:__eq(arg1) end

--- 
--- @return bool ret0
function util.Path:IsEmpty() end

--- 
function util.Path:IsPath() end

--- 
--- @param 1 string
--- @overload fun(arg1: util.Path): 
function util.Path:__add(arg1) end

--- 
function util.Path:GetString() end

--- 
function util.Path:GetFileName() end

--- 
--- @param offset int
--- @return class std::basic_string_view<char,struct std::char_traits<char> > ret0_1
--- @return int ret0_2
function util.Path:GetComponent(offset) end

--- 
function util.Path:__tostring() end

--- 
--- @param other string
--- @return bool ret0
--- @overload fun(pOther: util.Path): bool
function util.Path:MakeRelative(other) end

--- 
function util.Path:Copy() end

--- 
function util.Path:GetPath() end

--- 
function util.Path:GetBack() end

--- 
function util.Path:ToComponents() end

--- 
function util.Path:GetFront() end

--- 
function util.Path:PopFront() end

--- 
function util.Path:PopBack() end

--- 
function util.Path:Canonicalize() end

--- 
function util.Path:IsFile() end

--- 
function util.Path:GetFileExtension() end

--- 
--- @param extensions table
--- @overload fun(): 
function util.Path:RemoveFileExtension(extensions) end


--- 
--- @class util.CharVector
--- @overload fun():util.CharVector
util.CharVector = {}

--- 
--- @return int ret0
function util.CharVector:Back() end

--- 
--- @return int ret0
function util.CharVector:__len() end

--- 
--- @param name int
function util.CharVector:ShrinkToFit(name) end

--- 
--- @return int ret0
function util.CharVector:Front() end

--- 
--- @return table ret0
function util.CharVector:ToTable() end

--- 
--- @return int ret0
function util.CharVector:Capacity() end

--- 
--- @param name table
function util.CharVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 int
function util.CharVector:Set(arg1, arg2) end

--- 
function util.CharVector:Clear() end

--- 
--- @return int ret0
function util.CharVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 int
function util.CharVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return int ret0
function util.CharVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 int
--- @overload fun(name: int): 
function util.CharVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.CharVector:Reserve(name) end

--- 
--- @param name int
function util.CharVector:PushBack(name) end

--- 
--- @param name int
function util.CharVector:Erase(name) end


--- 
--- @class util.BoolVector
--- @overload fun():util.BoolVector
util.BoolVector = {}

--- 
--- @return bool ret0
function util.BoolVector:Back() end

--- 
--- @return int ret0
function util.BoolVector:__len() end

--- 
--- @param name int
function util.BoolVector:ShrinkToFit(name) end

--- 
--- @return bool ret0
function util.BoolVector:Front() end

--- 
--- @return table ret0
function util.BoolVector:ToTable() end

--- 
--- @return int ret0
function util.BoolVector:Capacity() end

--- 
--- @param name table
function util.BoolVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 bool
function util.BoolVector:Set(arg1, arg2) end

--- 
function util.BoolVector:Clear() end

--- 
--- @return int ret0
function util.BoolVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 bool
function util.BoolVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return bool ret0
function util.BoolVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 bool
--- @overload fun(name: int): 
function util.BoolVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.BoolVector:Reserve(name) end

--- 
--- @param name bool
function util.BoolVector:PushBack(name) end

--- 
--- @param name int
function util.BoolVector:Erase(name) end


--- 
--- @class util.Mat3x4Property: util.BaseProperty
--- @overload fun(arg1: math.Mat3x4):util.Mat3x4Property
--- @overload fun():util.Mat3x4Property
util.Mat3x4Property = {}

--- 
function util.Mat3x4Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat3x4Property:AddCallback(oCallback) end

--- 
function util.Mat3x4Property:Get() end

--- 
function util.Mat3x4Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat3x4Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat3x4Property:SetLocked(bLocked) end

--- 
function util.Mat3x4Property:Unlink() end

--- 
function util.Mat3x4Property:IsLocked() end

--- 
--- @param val math.Mat3x4
function util.Mat3x4Property:Set(val) end

--- 
--- @param propOther util.Mat3x4Property
function util.Mat3x4Property:Link(propOther) end


--- 
--- @class util.WorldData
--- @overload fun(args: class NetworkState):util.WorldData
util.WorldData = {}

--- 
--- @return util.WorldData.EntityData ret0
function util.WorldData:FindWorld() end

--- 
--- @param arg1 number
function util.WorldData:SetLightMapIntensity(arg1) end

--- 
--- @param arg1 number
function util.WorldData:SetLightMapExposure(arg1) end

--- 
--- @return table ret0
function util.WorldData:GetEntities() end

--- 
--- @return number ret0
function util.WorldData:GetLightMapExposure() end

--- 
--- @param assetData udm.AssetData
--- @param mapName string
--- @return bool ret0_1
--- @return string ret0_2
function util.WorldData:Save(assetData, mapName) end

--- 
--- @param arg1 util.ImageBuffer
function util.WorldData:SetLightMapAtlas(arg1) end

--- 
--- @param arg1 util.WorldData.EntityData
--- @param arg2 bool
--- @overload fun(arg1: util.WorldData.EntityData, arg2: bool): 
function util.WorldData:AddEntity(arg1, arg2) end

--- 
--- @param arg1 bool
function util.WorldData:SetLightMapEnabled(arg1) end

--- 
--- @return number ret0
function util.WorldData:GetLightMapIntensity() end

--- 
--- @return table ret0
function util.WorldData:GetMaterialTable() end


--- @enum DataFlagNone
util.WorldData = {
	DATA_FLAG_NONE = 0,
}

--- @enum DataFlagHas
util.WorldData = {
	DATA_FLAG_HAS_BSP_TREE_BIT = 2,
	DATA_FLAG_HAS_LIGHTMAP_ATLAS_BIT = 1,
}

--- 
--- @class util.WorldData.Output
--- @field delay number 
--- @field target string 
--- @field name int 
--- @field param string 
--- @field input string 
--- @overload fun():util.WorldData.Output
util.WorldData.Output = {}


--- 
--- @class util.WorldData.EntityData
--- @overload fun():util.WorldData.EntityData
util.WorldData.EntityData = {}

--- 
--- @return bool ret0
function util.WorldData.EntityData:IsWorld() end

--- 
--- @param arg1 string
--- @param arg2 string
function util.WorldData.EntityData:SetKeyValue(arg1, arg2) end

--- 
--- @return table ret0
function util.WorldData.EntityData:GetLeaves() end

--- 
--- @return math.Transform ret0
function util.WorldData.EntityData:GetPose() end

--- 
--- @param arg1 string
function util.WorldData.EntityData:SetClassName(arg1) end

--- 
--- @param arg1 int
--- @param arg2 int
function util.WorldData.EntityData:SetLeafData(arg1, arg2) end

--- 
--- @param arg1 util.WorldData.Output
function util.WorldData.EntityData:AddOutput(arg1) end

--- 
--- @return string ret0
function util.WorldData.EntityData:GetClassName() end

--- 
--- @param arg1 math.Vector
function util.WorldData.EntityData:SetOrigin(arg1) end

--- 
--- @return int ret0_1
--- @return int ret0_2
function util.WorldData.EntityData:GetLeafData() end

--- 
--- @return map ret0
function util.WorldData.EntityData:GetKeyValues() end

--- 
--- @return math.Vector ret0
function util.WorldData.EntityData:GetOrigin() end

--- 
--- @return int ret0
function util.WorldData.EntityData:GetMapIndex() end

--- 
--- @param arg1 math.Quaternion
function util.WorldData.EntityData:SetRotation(arg1) end

--- 
--- @param arg1 string
--- @return util.ComponentData ret0
function util.WorldData.EntityData:AddComponent(arg1) end

--- 
--- @return bool ret0
function util.WorldData.EntityData:IsSkybox() end

--- 
--- @return table ret0
function util.WorldData.EntityData:GetOutputs() end

--- 
--- @param arg1 string
--- @param arg2 string
--- @return string ret0
--- @overload fun(arg1: string): string
function util.WorldData.EntityData:GetKeyValue(arg1, arg2) end

--- 
--- @return bool ret0
function util.WorldData.EntityData:IsClientSideOnly() end

--- 
--- @param arg1 enum pragma::asset::EntityData::Flags
function util.WorldData.EntityData:SetFlags(arg1) end

--- 
--- @return map ret0
function util.WorldData.EntityData:GetComponents() end

--- 
--- @return enum pragma::asset::EntityData::Flags ret0
function util.WorldData.EntityData:GetFlags() end


--- @enum Flag
util.WorldData.EntityData = {
	FLAG_CLIENTSIDE_ONLY_BIT = 1,
	FLAG_NONE = 0,
}

--- 
--- @class util.Mat3x2Vector
--- @overload fun():util.Mat3x2Vector
util.Mat3x2Vector = {}

--- 
--- @return math.Mat3x2 ret0
function util.Mat3x2Vector:Back() end

--- 
--- @return int ret0
function util.Mat3x2Vector:__len() end

--- 
--- @param name int
function util.Mat3x2Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat3x2 ret0
function util.Mat3x2Vector:Front() end

--- 
--- @return table ret0
function util.Mat3x2Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat3x2Vector:Capacity() end

--- 
--- @param name table
function util.Mat3x2Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3x2
function util.Mat3x2Vector:Set(arg1, arg2) end

--- 
function util.Mat3x2Vector:Clear() end

--- 
--- @return int ret0
function util.Mat3x2Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3x2
function util.Mat3x2Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat3x2 ret0
function util.Mat3x2Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3x2
--- @overload fun(name: int): 
function util.Mat3x2Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat3x2Vector:Reserve(name) end

--- 
--- @param name math.Mat3x2
function util.Mat3x2Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat3x2Vector:Erase(name) end


--- 
--- @class util.Vector3Vector
--- @overload fun():util.Vector3Vector
util.Vector3Vector = {}

--- 
--- @return math.Vector ret0
function util.Vector3Vector:Back() end

--- 
--- @return int ret0
function util.Vector3Vector:__len() end

--- 
--- @param name int
function util.Vector3Vector:ShrinkToFit(name) end

--- 
--- @return math.Vector ret0
function util.Vector3Vector:Front() end

--- 
--- @return table ret0
function util.Vector3Vector:ToTable() end

--- 
--- @return int ret0
function util.Vector3Vector:Capacity() end

--- 
--- @param name table
function util.Vector3Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector
function util.Vector3Vector:Set(arg1, arg2) end

--- 
function util.Vector3Vector:Clear() end

--- 
--- @return int ret0
function util.Vector3Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Vector
function util.Vector3Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Vector ret0
function util.Vector3Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector
--- @overload fun(name: int): 
function util.Vector3Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Vector3Vector:Reserve(name) end

--- 
--- @param name math.Vector
function util.Vector3Vector:PushBack(name) end

--- 
--- @param name int
function util.Vector3Vector:Erase(name) end


--- 
--- @class util.Mat2Property: util.BaseProperty
--- @overload fun(arg1: math.Mat2):util.Mat2Property
--- @overload fun():util.Mat2Property
util.Mat2Property = {}

--- 
function util.Mat2Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat2Property:AddCallback(oCallback) end

--- 
function util.Mat2Property:Get() end

--- 
function util.Mat2Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat2Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat2Property:SetLocked(bLocked) end

--- 
function util.Mat2Property:Unlink() end

--- 
function util.Mat2Property:IsLocked() end

--- 
--- @param val math.Mat2
function util.Mat2Property:Set(val) end

--- 
--- @param propOther util.Mat2Property
function util.Mat2Property:Link(propOther) end


--- 
--- @class util.EntityProperty: util.BaseProperty
--- @overload fun(arg1: ents.BaseEntityBase):util.EntityProperty
--- @overload fun():util.EntityProperty
util.EntityProperty = {}

--- 
function util.EntityProperty:__tostring() end

--- 
--- @param propOther util.EntityProperty
function util.EntityProperty:Link(propOther) end


--- 
--- @class util.ThreadPool
--- @overload fun(arg2: int, arg3: string):util.ThreadPool
--- @overload fun(arg2: int):util.ThreadPool
util.ThreadPool = {}

--- 
--- @return int ret0
function util.ThreadPool:GetCompletedTaskCount() end

--- 
--- @param arg1 bool
function util.ThreadPool:Stop(arg1) end

--- 
--- @return int ret0
function util.ThreadPool:GetTaskCount() end

--- 
--- @param arg1 int
function util.ThreadPool:PushResults(arg1) end

--- 
--- @param arg1 int
function util.ThreadPool:WaitForPendingCount(arg1) end

--- 
--- @param arg1 int
--- @return bool ret0
--- @overload fun(): bool
function util.ThreadPool:IsComplete(arg1) end

--- 
function util.ThreadPool:AddBarrier() end

--- 
--- @param threadTask util.ThreadPool.ThreadTask
--- @return int ret0
function util.ThreadPool:AddTask(threadTask) end

--- 
function util.ThreadPool:WaitForCompletion() end

--- 
--- @return int ret0
function util.ThreadPool:GetPendingTaskCount() end


--- 
--- @class util.ThreadPool.ThreadTask
--- @overload fun():util.ThreadPool.ThreadTask
util.ThreadPool.ThreadTask = {}


--- 
--- @class util.SplashDamageInfo
--- @field damageInfo game.DamageInfo 
--- @field origin  
--- @field radius number 
--- @overload fun():util.SplashDamageInfo
util.SplashDamageInfo = {}

--- 
--- @param coneDirection math.Vector
--- @param coneAngle number
function util.SplashDamageInfo:SetCone(coneDirection, coneAngle) end

--- 
--- @param oCallback any
function util.SplashDamageInfo:SetCallback(oCallback) end


--- 
--- @class util.StringVector
--- @overload fun():util.StringVector
util.StringVector = {}

--- 
--- @return string ret0
function util.StringVector:Back() end

--- 
--- @return int ret0
function util.StringVector:__len() end

--- 
--- @param name int
function util.StringVector:ShrinkToFit(name) end

--- 
--- @return string ret0
function util.StringVector:Front() end

--- 
--- @return table ret0
function util.StringVector:ToTable() end

--- 
--- @return int ret0
function util.StringVector:Capacity() end

--- 
--- @param name table
function util.StringVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 string
function util.StringVector:Set(arg1, arg2) end

--- 
function util.StringVector:Clear() end

--- 
--- @return int ret0
function util.StringVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 string
function util.StringVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return string ret0
function util.StringVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 string
--- @overload fun(name: int): 
function util.StringVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.StringVector:Reserve(name) end

--- 
--- @param name string
function util.StringVector:PushBack(name) end

--- 
--- @param name int
function util.StringVector:Erase(name) end


--- 
--- @class util.ComponentData
--- @overload fun():util.ComponentData
util.ComponentData = {}

--- 
--- @return udm.LinkedPropertyWrapper ret0
function util.ComponentData:GetData() end

--- 
--- @param arg1 enum pragma::asset::ComponentData::Flags
function util.ComponentData:SetFlags(arg1) end

--- 
--- @return enum pragma::asset::ComponentData::Flags ret0
function util.ComponentData:GetFlags() end


--- @enum Flag
util.ComponentData = {
	FLAG_CLIENTSIDE_ONLY_BIT = 1,
	FLAG_NONE = 0,
}

--- 
--- @class util.Version
--- @field major int 
--- @field minor int 
--- @field revision int 
--- @overload fun(arg1: string):util.Version
--- @overload fun(arg1: int, arg2: int, arg3: int):util.Version
--- @overload fun(arg1: int, arg2: int):util.Version
--- @overload fun():util.Version
util.Version = {}

--- 
function util.Version:__tostring() end

--- 
--- @param 1 util.Version
function util.Version:__eq(arg1) end

--- 
--- @return string ret0
function util.Version:ToString() end

--- 
--- @param 1 util.Version
function util.Version:__lt(arg1) end

--- 
--- @param 1 util.Version
function util.Version:__le(arg1) end

--- 
function util.Version:Reset() end


--- 
--- @class util.Vector4Property: util.BaseProperty
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):util.Vector4Property
--- @overload fun(arg1: math.Vector4):util.Vector4Property
--- @overload fun():util.Vector4Property
util.Vector4Property = {}

--- 
--- @param 1 number
function util.Vector4Property:__div(arg1) end

--- 
function util.Vector4Property:__tostring() end

--- 
--- @param oCallback any
function util.Vector4Property:AddCallback(oCallback) end

--- 
function util.Vector4Property:InvokeCallbacks() end

--- 
function util.Vector4Property:Get() end

--- 
--- @param 1 util.Vector4Property
--- @overload fun(arg1: math.Vector4): 
--- @overload fun(arg1: util.Vector4Property): 
function util.Vector4Property:__eq(arg1) end

--- 
--- @param oCallback any
function util.Vector4Property:AddModifier(oCallback) end

--- 
function util.Vector4Property:Unlink() end

--- 
--- @param val math.Vector4
function util.Vector4Property:Set(val) end

--- 
--- @param bLocked bool
function util.Vector4Property:SetLocked(bLocked) end

--- 
function util.Vector4Property:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.Vector4iProperty): 
--- @overload fun(propOther: util.Vector4Property): 
function util.Vector4Property:Link(propOther) end

--- 
--- @param 1 util.Vector4Property
--- @overload fun(arg1: util.Vector4Property): 
--- @overload fun(arg1: math.Vector4): 
function util.Vector4Property:__add(arg1) end

--- 
--- @param 1 util.Vector4Property
--- @overload fun(arg1: util.Vector4Property): 
--- @overload fun(arg1: math.Vector4): 
function util.Vector4Property:__sub(arg1) end

--- 
--- @param 1 util.Vector4Property
--- @overload fun(arg1: number): 
function util.Vector4Property:__mul(arg1) end


--- 
--- @class util.VectorProperty: util.BaseProperty
--- @overload fun(arg1: number, arg2: number, arg3: number):util.VectorProperty
--- @overload fun(arg1: math.Vector):util.VectorProperty
--- @overload fun():util.VectorProperty
util.VectorProperty = {}

--- 
--- @param 1 number
function util.VectorProperty:__div(arg1) end

--- 
function util.VectorProperty:__tostring() end

--- 
--- @param oCallback any
function util.VectorProperty:AddCallback(oCallback) end

--- 
function util.VectorProperty:InvokeCallbacks() end

--- 
function util.VectorProperty:Get() end

--- 
--- @param 1 util.VectorProperty
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: util.VectorProperty): 
function util.VectorProperty:__eq(arg1) end

--- 
--- @param oCallback any
function util.VectorProperty:AddModifier(oCallback) end

--- 
function util.VectorProperty:Unlink() end

--- 
--- @param val math.Vector
function util.VectorProperty:Set(val) end

--- 
--- @param bLocked bool
function util.VectorProperty:SetLocked(bLocked) end

--- 
function util.VectorProperty:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.VectoriProperty): 
--- @overload fun(propOther: util.ColorProperty): 
--- @overload fun(propOther: util.VectorProperty): 
function util.VectorProperty:Link(propOther) end

--- 
--- @param 1 util.VectorProperty
--- @overload fun(arg1: util.VectorProperty): 
--- @overload fun(arg1: math.Vector): 
function util.VectorProperty:__add(arg1) end

--- 
--- @param 1 util.VectorProperty
--- @overload fun(arg1: util.VectorProperty): 
--- @overload fun(arg1: math.Vector): 
function util.VectorProperty:__sub(arg1) end

--- 
--- @param 1 util.VectorProperty
--- @overload fun(arg1: number): 
function util.VectorProperty:__mul(arg1) end


--- 
--- @class util.ColorVector
--- @overload fun():util.ColorVector
util.ColorVector = {}

--- 
--- @return util.Color ret0
function util.ColorVector:Back() end

--- 
--- @return int ret0
function util.ColorVector:__len() end

--- 
--- @param name int
function util.ColorVector:ShrinkToFit(name) end

--- 
--- @return util.Color ret0
function util.ColorVector:Front() end

--- 
--- @return table ret0
function util.ColorVector:ToTable() end

--- 
--- @return int ret0
function util.ColorVector:Capacity() end

--- 
--- @param name table
function util.ColorVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 util.Color
function util.ColorVector:Set(arg1, arg2) end

--- 
function util.ColorVector:Clear() end

--- 
--- @return int ret0
function util.ColorVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 util.Color
function util.ColorVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return util.Color ret0
function util.ColorVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 util.Color
--- @overload fun(name: int): 
function util.ColorVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.ColorVector:Reserve(name) end

--- 
--- @param name util.Color
function util.ColorVector:PushBack(name) end

--- 
--- @param name int
function util.ColorVector:Erase(name) end


--- 
--- @class util.HSVColor
--- @field s number 
--- @field h number 
--- @field v number 
--- @overload fun(arg1: number, arg2: number, arg3: number):util.HSVColor
--- @overload fun():util.HSVColor
util.HSVColor = {}

--- 
function util.HSVColor:ToRGBColor() end

--- 
--- @param hsv1 util.HSVColor
--- @param t number
function util.HSVColor:Lerp(hsv1, t) end

--- 
function util.HSVColor:__tostring() end

--- 
--- @param 1 util.HSVColor
function util.HSVColor:__eq(arg1) end


--- 
--- @class util.RenderTile
--- @field x number 
--- @field h number 
--- @field y number 
--- @field w number 
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):util.RenderTile
--- @overload fun():util.RenderTile
util.RenderTile = {}

--- 
function util.RenderTile:__tostring() end


--- 
--- @class util.Mat2x3Vector
--- @overload fun():util.Mat2x3Vector
util.Mat2x3Vector = {}

--- 
--- @return math.Mat2x3 ret0
function util.Mat2x3Vector:Back() end

--- 
--- @return int ret0
function util.Mat2x3Vector:__len() end

--- 
--- @param name int
function util.Mat2x3Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat2x3 ret0
function util.Mat2x3Vector:Front() end

--- 
--- @return table ret0
function util.Mat2x3Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat2x3Vector:Capacity() end

--- 
--- @param name table
function util.Mat2x3Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2x3
function util.Mat2x3Vector:Set(arg1, arg2) end

--- 
function util.Mat2x3Vector:Clear() end

--- 
--- @return int ret0
function util.Mat2x3Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2x3
function util.Mat2x3Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat2x3 ret0
function util.Mat2x3Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2x3
--- @overload fun(name: int): 
function util.Mat2x3Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat2x3Vector:Reserve(name) end

--- 
--- @param name math.Mat2x3
function util.Mat2x3Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat2x3Vector:Erase(name) end


--- 
--- @class util.EulerAnglesProperty: util.BaseProperty
--- @overload fun(arg1: string):util.EulerAnglesProperty
--- @overload fun(arg1: number, arg2: number, arg3: number):util.EulerAnglesProperty
--- @overload fun(arg1: math.EulerAngles):util.EulerAnglesProperty
--- @overload fun():util.EulerAnglesProperty
util.EulerAnglesProperty = {}

--- 
--- @param 1 number
function util.EulerAnglesProperty:__div(arg1) end

--- 
function util.EulerAnglesProperty:Unlink() end

--- 
--- @param val math.EulerAngles
function util.EulerAnglesProperty:Set(val) end

--- 
--- @param oCallback any
function util.EulerAnglesProperty:AddCallback(oCallback) end

--- 
function util.EulerAnglesProperty:__tostring() end

--- 
function util.EulerAnglesProperty:InvokeCallbacks() end

--- 
function util.EulerAnglesProperty:Get() end

--- 
--- @param oCallback any
function util.EulerAnglesProperty:AddModifier(oCallback) end

--- 
--- @param 1 util.EulerAnglesProperty
--- @overload fun(arg1: math.EulerAngles): 
--- @overload fun(arg1: util.EulerAnglesProperty): 
function util.EulerAnglesProperty:__eq(arg1) end

--- 
--- @param bLocked bool
function util.EulerAnglesProperty:SetLocked(bLocked) end

--- 
function util.EulerAnglesProperty:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.EulerAnglesProperty): 
function util.EulerAnglesProperty:Link(propOther) end

--- 
--- @param 1 util.EulerAnglesProperty
--- @overload fun(arg1: util.EulerAnglesProperty): 
--- @overload fun(arg1: math.EulerAngles): 
function util.EulerAnglesProperty:__add(arg1) end

--- 
--- @param 1 util.EulerAnglesProperty
--- @overload fun(arg1: util.EulerAnglesProperty): 
--- @overload fun(arg1: math.EulerAngles): 
function util.EulerAnglesProperty:__sub(arg1) end

--- 
--- @param 1 util.EulerAnglesProperty
--- @overload fun(arg1: number): 
function util.EulerAnglesProperty:__mul(arg1) end


--- 
--- @class util.ZipFile
util.ZipFile = {}

--- 
--- @return table ret0
function util.ZipFile:GetFileList() end

--- 
--- @param zipFileName string
--- @param outputZipFileName string
--- @return bool ret0_1
--- @return string ret0_2
function util.ZipFile:ExtractFile(zipFileName, outputZipFileName) end


--- @enum OpenMode
util.ZipFile = {
	OPEN_MODE_READ = 0,
	OPEN_MODE_WRITE = 1,
}

--- 
--- @class util.DataBlock
util.DataBlock = {}

--- 
--- @param key string
function util.DataBlock:AddBlock(key) end

--- 
--- @param key string
function util.DataBlock:IsFloat(key) end

--- 
--- @param key string
function util.DataBlock:IsBool(key) end

--- 
--- @param key string
function util.DataBlock:IsString(key) end

--- 
--- @param key string
function util.DataBlock:IsColor(key) end

--- 
--- @param key string
function util.DataBlock:IsInt(key) end

--- 
--- @param key string
function util.DataBlock:IsVector(key) end

--- 
--- @param key string
function util.DataBlock:IsVector4(key) end

--- 
--- @param val string
function util.DataBlock:GetData(val) end

--- 
--- @param key string
--- @param blockIndex int
--- @overload fun(key: string): 
function util.DataBlock:FindBlock(key, blockIndex) end

--- 
--- @overload fun(tabDepth: int): 
--- @overload fun(rootIdentifier: string): 
--- @overload fun(rootIdentifier: string, tabDepth: int): 
function util.DataBlock:ToString() end

--- 
--- @param val string
--- @param def string
--- @overload fun(val: string): 
function util.DataBlock:GetString(val, def) end

--- 
--- @param val string
--- @param def math.Vector2
--- @overload fun(val: string): 
function util.DataBlock:GetVector2(val, def) end

--- 
--- @param val string
--- @param def int
--- @overload fun(val: string): 
function util.DataBlock:GetInt(val, def) end

--- 
--- @param val string
--- @param def number
--- @overload fun(val: string): 
function util.DataBlock:GetFloat(val, def) end

--- 
--- @param val string
--- @param def bool
--- @overload fun(val: string): 
function util.DataBlock:GetBool(val, def) end

--- 
--- @param val string
--- @param def util.Color
--- @overload fun(val: string): 
function util.DataBlock:GetColor(val, def) end

--- 
--- @param val string
--- @param def math.Vector
--- @overload fun(val: string): 
function util.DataBlock:GetVector(val, def) end

--- 
--- @param val string
--- @param def math.Vector4
--- @overload fun(val: string): 
function util.DataBlock:GetVector4(val, def) end

--- 
function util.DataBlock:GetChildBlocks() end

--- 
--- @param type string
--- @param key string
--- @param val string
function util.DataBlock:SetValue(type, key, val) end

--- 
--- @param other util.DataBlock
function util.DataBlock:Merge(other) end

--- 
--- @param key string
function util.DataBlock:GetValueType(key) end

--- 
function util.DataBlock:GetKeys() end

--- 
--- @param key string
function util.DataBlock:RemoveValue(key) end

--- 
function util.DataBlock:IsEmpty() end

--- 
--- @param key string
function util.DataBlock:HasValue(key) end


--- 
--- @class util.FloatVector
--- @overload fun():util.FloatVector
util.FloatVector = {}

--- 
--- @return number ret0
function util.FloatVector:Back() end

--- 
--- @return int ret0
function util.FloatVector:__len() end

--- 
--- @param name int
function util.FloatVector:ShrinkToFit(name) end

--- 
--- @return number ret0
function util.FloatVector:Front() end

--- 
--- @return table ret0
function util.FloatVector:ToTable() end

--- 
--- @return int ret0
function util.FloatVector:Capacity() end

--- 
--- @param name table
function util.FloatVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 number
function util.FloatVector:Set(arg1, arg2) end

--- 
function util.FloatVector:Clear() end

--- 
--- @return int ret0
function util.FloatVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 number
function util.FloatVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return number ret0
function util.FloatVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 number
--- @overload fun(name: int): 
function util.FloatVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.FloatVector:Reserve(name) end

--- 
--- @param name number
function util.FloatVector:PushBack(name) end

--- 
--- @param name int
function util.FloatVector:Erase(name) end


--- 
--- @class util.Mat2Vector
--- @overload fun():util.Mat2Vector
util.Mat2Vector = {}

--- 
--- @return math.Mat2 ret0
function util.Mat2Vector:Back() end

--- 
--- @return int ret0
function util.Mat2Vector:__len() end

--- 
--- @param name int
function util.Mat2Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat2 ret0
function util.Mat2Vector:Front() end

--- 
--- @return table ret0
function util.Mat2Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat2Vector:Capacity() end

--- 
--- @param name table
function util.Mat2Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2
function util.Mat2Vector:Set(arg1, arg2) end

--- 
function util.Mat2Vector:Clear() end

--- 
--- @return int ret0
function util.Mat2Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2
function util.Mat2Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat2 ret0
function util.Mat2Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2
--- @overload fun(name: int): 
function util.Mat2Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat2Vector:Reserve(name) end

--- 
--- @param name math.Mat2
function util.Mat2Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat2Vector:Erase(name) end


--- 
--- @class util.Vector4iVector
--- @overload fun():util.Vector4iVector
util.Vector4iVector = {}

--- 
--- @return math.Vector4i ret0
function util.Vector4iVector:Back() end

--- 
--- @return int ret0
function util.Vector4iVector:__len() end

--- 
--- @param name int
function util.Vector4iVector:ShrinkToFit(name) end

--- 
--- @return math.Vector4i ret0
function util.Vector4iVector:Front() end

--- 
--- @return table ret0
function util.Vector4iVector:ToTable() end

--- 
--- @return int ret0
function util.Vector4iVector:Capacity() end

--- 
--- @param name table
function util.Vector4iVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector4i
function util.Vector4iVector:Set(arg1, arg2) end

--- 
function util.Vector4iVector:Clear() end

--- 
--- @return int ret0
function util.Vector4iVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Vector4i
function util.Vector4iVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Vector4i ret0
function util.Vector4iVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector4i
--- @overload fun(name: int): 
function util.Vector4iVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Vector4iVector:Reserve(name) end

--- 
--- @param name math.Vector4i
function util.Vector4iVector:PushBack(name) end

--- 
--- @param name int
function util.Vector4iVector:Erase(name) end


--- 
--- @class util.Mat2x4Vector
--- @overload fun():util.Mat2x4Vector
util.Mat2x4Vector = {}

--- 
--- @return math.Mat2x4 ret0
function util.Mat2x4Vector:Back() end

--- 
--- @return int ret0
function util.Mat2x4Vector:__len() end

--- 
--- @param name int
function util.Mat2x4Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat2x4 ret0
function util.Mat2x4Vector:Front() end

--- 
--- @return table ret0
function util.Mat2x4Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat2x4Vector:Capacity() end

--- 
--- @param name table
function util.Mat2x4Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2x4
function util.Mat2x4Vector:Set(arg1, arg2) end

--- 
function util.Mat2x4Vector:Clear() end

--- 
--- @return int ret0
function util.Mat2x4Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2x4
function util.Mat2x4Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat2x4 ret0
function util.Mat2x4Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat2x4
--- @overload fun(name: int): 
function util.Mat2x4Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat2x4Vector:Reserve(name) end

--- 
--- @param name math.Mat2x4
function util.Mat2x4Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat2x4Vector:Erase(name) end


--- 
--- @class util.QuaternionVector
--- @overload fun():util.QuaternionVector
util.QuaternionVector = {}

--- 
--- @return math.Quaternion ret0
function util.QuaternionVector:Back() end

--- 
--- @return int ret0
function util.QuaternionVector:__len() end

--- 
--- @param name int
function util.QuaternionVector:ShrinkToFit(name) end

--- 
--- @return math.Quaternion ret0
function util.QuaternionVector:Front() end

--- 
--- @return table ret0
function util.QuaternionVector:ToTable() end

--- 
--- @return int ret0
function util.QuaternionVector:Capacity() end

--- 
--- @param name table
function util.QuaternionVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Quaternion
function util.QuaternionVector:Set(arg1, arg2) end

--- 
function util.QuaternionVector:Clear() end

--- 
--- @return int ret0
function util.QuaternionVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Quaternion
function util.QuaternionVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Quaternion ret0
function util.QuaternionVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Quaternion
--- @overload fun(name: int): 
function util.QuaternionVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.QuaternionVector:Reserve(name) end

--- 
--- @param name math.Quaternion
function util.QuaternionVector:PushBack(name) end

--- 
--- @param name int
function util.QuaternionVector:Erase(name) end


--- 
--- @class util.LongLongVector
--- @overload fun():util.LongLongVector
util.LongLongVector = {}

--- 
--- @return int ret0
function util.LongLongVector:Back() end

--- 
--- @return int ret0
function util.LongLongVector:__len() end

--- 
--- @param name int
function util.LongLongVector:ShrinkToFit(name) end

--- 
--- @return int ret0
function util.LongLongVector:Front() end

--- 
--- @return table ret0
function util.LongLongVector:ToTable() end

--- 
--- @return int ret0
function util.LongLongVector:Capacity() end

--- 
--- @param name table
function util.LongLongVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 int
function util.LongLongVector:Set(arg1, arg2) end

--- 
function util.LongLongVector:Clear() end

--- 
--- @return int ret0
function util.LongLongVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 int
function util.LongLongVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return int ret0
function util.LongLongVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 int
--- @overload fun(name: int): 
function util.LongLongVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.LongLongVector:Reserve(name) end

--- 
--- @param name int
function util.LongLongVector:PushBack(name) end

--- 
--- @param name int
function util.LongLongVector:Erase(name) end


--- 
--- @class util.DoubleVector
--- @overload fun():util.DoubleVector
util.DoubleVector = {}

--- 
--- @return number ret0
function util.DoubleVector:Back() end

--- 
--- @return int ret0
function util.DoubleVector:__len() end

--- 
--- @param name int
function util.DoubleVector:ShrinkToFit(name) end

--- 
--- @return number ret0
function util.DoubleVector:Front() end

--- 
--- @return table ret0
function util.DoubleVector:ToTable() end

--- 
--- @return int ret0
function util.DoubleVector:Capacity() end

--- 
--- @param name table
function util.DoubleVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 number
function util.DoubleVector:Set(arg1, arg2) end

--- 
function util.DoubleVector:Clear() end

--- 
--- @return int ret0
function util.DoubleVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 number
function util.DoubleVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return number ret0
function util.DoubleVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 number
--- @overload fun(name: int): 
function util.DoubleVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.DoubleVector:Reserve(name) end

--- 
--- @param name number
function util.DoubleVector:PushBack(name) end

--- 
--- @param name int
function util.DoubleVector:Erase(name) end


--- 
--- @class util.Vector2iVector
--- @overload fun():util.Vector2iVector
util.Vector2iVector = {}

--- 
--- @return math.Vector2i ret0
function util.Vector2iVector:Back() end

--- 
--- @return int ret0
function util.Vector2iVector:__len() end

--- 
--- @param name int
function util.Vector2iVector:ShrinkToFit(name) end

--- 
--- @return math.Vector2i ret0
function util.Vector2iVector:Front() end

--- 
--- @return table ret0
function util.Vector2iVector:ToTable() end

--- 
--- @return int ret0
function util.Vector2iVector:Capacity() end

--- 
--- @param name table
function util.Vector2iVector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector2i
function util.Vector2iVector:Set(arg1, arg2) end

--- 
function util.Vector2iVector:Clear() end

--- 
--- @return int ret0
function util.Vector2iVector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Vector2i
function util.Vector2iVector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Vector2i ret0
function util.Vector2iVector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector2i
--- @overload fun(name: int): 
function util.Vector2iVector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Vector2iVector:Reserve(name) end

--- 
--- @param name math.Vector2i
function util.Vector2iVector:PushBack(name) end

--- 
--- @param name int
function util.Vector2iVector:Erase(name) end


--- 
--- @class util.FloatProperty
--- @overload fun(arg1: number):util.FloatProperty
--- @overload fun():util.FloatProperty
util.FloatProperty = {}

--- 
function util.FloatProperty:__tostring() end

--- 
--- @param oCallback any
function util.FloatProperty:AddCallback(oCallback) end

--- 
function util.FloatProperty:InvokeCallbacks() end

--- 
function util.FloatProperty:Get() end

--- 
--- @param oCallback any
function util.FloatProperty:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.FloatProperty:SetLocked(bLocked) end

--- 
function util.FloatProperty:Unlink() end

--- 
function util.FloatProperty:IsLocked() end

--- 
--- @param val number
function util.FloatProperty:Set(val) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: util.FloatProperty): 
--- @overload fun(arg1: number): 
function util.FloatProperty:__le(arg1) end

--- 
--- @param propOther util.IntProperty
--- @overload fun(propOther: util.FloatProperty): 
--- @overload fun(propOther: util.StringProperty): 
--- @overload fun(propOther: util.BoolProperty): 
function util.FloatProperty:Link(propOther) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: util.FloatProperty): 
--- @overload fun(arg1: number): 
function util.FloatProperty:__add(arg1) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: util.FloatProperty): 
--- @overload fun(arg1: number): 
function util.FloatProperty:__sub(arg1) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: util.FloatProperty): 
--- @overload fun(arg1: number): 
function util.FloatProperty:__mul(arg1) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: util.FloatProperty): 
--- @overload fun(arg1: number): 
function util.FloatProperty:__div(arg1) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: util.FloatProperty): 
--- @overload fun(arg1: number): 
function util.FloatProperty:__lt(arg1) end

--- 
--- @param 1 util.FloatProperty
--- @overload fun(arg1: number): 
--- @overload fun(arg1: util.FloatProperty): 
function util.FloatProperty:__eq(arg1) end


--- 
--- @class util.ResultCode
--- @overload fun(arg1: string, arg2: int):util.ResultCode
--- @overload fun():util.ResultCode
util.ResultCode = {}

--- 
--- @return bool ret0
function util.ResultCode:IsError() end

--- 
--- @return string ret0
function util.ResultCode:GetMessage() end

--- 
--- @return int ret0
function util.ResultCode:GetValue() end

--- 
function util.ResultCode:__tostring() end


--- 
--- @class util.ImageBuffer
util.ImageBuffer = {}

--- 
--- @param toneMapping int
function util.ImageBuffer:ApplyToneMapping(toneMapping) end

--- 
function util.ImageBuffer:GetFormat() end

--- 
--- @param gamma number
--- @overload fun(): 
function util.ImageBuffer:ApplyGammaCorrection(gamma) end

--- 
--- @param x int
--- @param y int
--- @param channel int
--- @param value int
function util.ImageBuffer:SetPixelValueHDR(x, y, channel, value) end

--- 
--- @param exposure number
function util.ImageBuffer:ApplyExposure(exposure) end

--- 
function util.ImageBuffer:IsFloatFormat() end

--- 
--- @param color math.Vector4
--- @overload fun(color: util.Color): 
function util.ImageBuffer:Clear(color) end

--- 
--- @param x int
--- @param y int
--- @param channel int
function util.ImageBuffer:GetPixelValue(x, y, channel) end

--- 
--- @param x int
--- @param y int
--- @param channel int
--- @param value number
function util.ImageBuffer:SetPixelValue(x, y, channel, value) end

--- 
function util.ImageBuffer:IsHDRFormat() end

--- 
--- @param pixelIdx int
--- @param color util.Color
--- @overload fun(x: int, y: int, color: util.Color): 
--- @overload fun(arg1: int, arg2: math.Vector4): 
--- @overload fun(arg1: int, arg2: int, arg3: math.Vector4): 
function util.ImageBuffer:SetPixelColor(pixelIdx, color) end

--- 
--- @param x int
--- @param y int
--- @param channel int
--- @param value int
function util.ImageBuffer:SetPixelValueLDR(x, y, channel, value) end

--- 
function util.ImageBuffer:GetPixelCount() end

--- 
function util.ImageBuffer:CalcLuminance() end

--- 
function util.ImageBuffer:HasAlphaChannel() end

--- 
--- @param dst util.ImageBuffer
--- @param xSrc int
--- @param ySrc int
--- @param xDst int
--- @param yDst int
--- @param w int
--- @param h int
--- @overload fun(format: int): 
--- @overload fun(): 
function util.ImageBuffer:Copy(dst, xSrc, ySrc, xDst, yDst, w, h) end

--- 
function util.ImageBuffer:__tostring() end

--- 
function util.ImageBuffer:GetData() end

--- 
--- @overload fun(tw: struct pragma::lua::LuaThreadWrapper): 
function util.ImageBuffer:ToLDR() end

--- 
--- @param a number
function util.ImageBuffer:ClearAlpha(a) end

--- 
function util.ImageBuffer:GetWidth() end

--- 
function util.ImageBuffer:GetHeight() end

--- 
function util.ImageBuffer:GetChannelCount() end

--- 
function util.ImageBuffer:GetChannelSize() end

--- 
function util.ImageBuffer:GetPixelSize() end

--- 
function util.ImageBuffer:IsLDRFormat() end

--- 
--- @param arg1 util.ImageBuffer
--- @param arg2 int
--- @param arg3 int
--- @overload fun(arg1: util.ImageBuffer, arg2: int, arg3: int, arg4: int, arg5: int, arg6: int, arg7: int): 
function util.ImageBuffer:Insert(arg1, arg2, arg3) end

--- 
--- @param format int
function util.ImageBuffer:Convert(format) end

--- 
function util.ImageBuffer:GetSize() end

--- 
function util.ImageBuffer:ToHDR() end

--- 
function util.ImageBuffer:ToFloat() end

--- 
--- @param w int
--- @param h int
function util.ImageBuffer:Resize(w, h) end

--- 
--- @param x int
--- @param y int
--- @overload fun(x: int, y: int): 
function util.ImageBuffer:GetPixelIndex(x, y) end

--- 
--- @param x int
--- @param y int
--- @overload fun(x: int, y: int): 
function util.ImageBuffer:GetPixelOffset(x, y) end

--- 
function util.ImageBuffer:FlipHorizontally() end

--- 
function util.ImageBuffer:FlipVertically() end

--- 
--- @param flipH bool
--- @param flipV bool
function util.ImageBuffer:Flip(flipH, flipV) end

--- 
--- @param channel0 enum uimg::Channel
--- @param channel1 enum uimg::Channel
--- @param tw struct pragma::lua::LuaThreadWrapper
--- @overload fun(channel0: enum uimg::Channel, channel1: enum uimg::Channel): 
function util.ImageBuffer:SwapChannels(channel0, channel1, tw) end


--- @enum FormatRgba
util.ImageBuffer = {
	FORMAT_RGBA16 = 8,
	FORMAT_RGBA_HDR = 8,
	FORMAT_RGBA32 = 12,
	FORMAT_RGBA8 = 4,
	FORMAT_RGBA_LDR = 4,
	FORMAT_RGBA_FLOAT = 12,
}

--- @enum Channel
util.ImageBuffer = {
	CHANNEL_RED = 0,
	CHANNEL_GREEN = 1,
	CHANNEL_A = 3,
	CHANNEL_ALPHA = 3,
	CHANNEL_R = 0,
	CHANNEL_BLUE = 2,
	CHANNEL_B = 2,
	CHANNEL_G = 1,
}

--- @enum ToneMapping
util.ImageBuffer = {
	TONE_MAPPING_ACES = 4,
	TONE_MAPPING_UNCHARTED = 3,
	TONE_MAPPING_GRAN_TURISMO = 5,
	TONE_MAPPING_GAMMA_CORRECTION = 0,
	TONE_MAPPING_HEJIL_RICHARD = 2,
	TONE_MAPPING_REINHARD = 1,
}

--- @enum FormatRgb
util.ImageBuffer = {
	FORMAT_RGB16 = 7,
	FORMAT_RGB_HDR = 7,
	FORMAT_RGB_FLOAT = 11,
	FORMAT_RGB32 = 11,
	FORMAT_RGB8 = 3,
	FORMAT_RGB_LDR = 3,
}

--- @enum Format
util.ImageBuffer = {
	FORMAT_COUNT = 13,
	FORMAT_NONE = 0,
}

--- 
--- @class util.Mat3x2Property: util.BaseProperty
--- @overload fun(arg1: math.Mat3x2):util.Mat3x2Property
--- @overload fun():util.Mat3x2Property
util.Mat3x2Property = {}

--- 
function util.Mat3x2Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat3x2Property:AddCallback(oCallback) end

--- 
function util.Mat3x2Property:Get() end

--- 
function util.Mat3x2Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat3x2Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat3x2Property:SetLocked(bLocked) end

--- 
function util.Mat3x2Property:Unlink() end

--- 
function util.Mat3x2Property:IsLocked() end

--- 
--- @param val math.Mat3x2
function util.Mat3x2Property:Set(val) end

--- 
--- @param propOther util.Mat3x2Property
function util.Mat3x2Property:Link(propOther) end


--- 
--- @class util.Vector2Vector
--- @overload fun():util.Vector2Vector
util.Vector2Vector = {}

--- 
--- @return math.Vector2 ret0
function util.Vector2Vector:Back() end

--- 
--- @return int ret0
function util.Vector2Vector:__len() end

--- 
--- @param name int
function util.Vector2Vector:ShrinkToFit(name) end

--- 
--- @return math.Vector2 ret0
function util.Vector2Vector:Front() end

--- 
--- @return table ret0
function util.Vector2Vector:ToTable() end

--- 
--- @return int ret0
function util.Vector2Vector:Capacity() end

--- 
--- @param name table
function util.Vector2Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector2
function util.Vector2Vector:Set(arg1, arg2) end

--- 
function util.Vector2Vector:Clear() end

--- 
--- @return int ret0
function util.Vector2Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Vector2
function util.Vector2Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Vector2 ret0
function util.Vector2Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector2
--- @overload fun(name: int): 
function util.Vector2Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Vector2Vector:Reserve(name) end

--- 
--- @param name math.Vector2
function util.Vector2Vector:PushBack(name) end

--- 
--- @param name int
function util.Vector2Vector:Erase(name) end


--- 
--- @class util.Vector4Vector
--- @overload fun():util.Vector4Vector
util.Vector4Vector = {}

--- 
--- @return math.Vector4 ret0
function util.Vector4Vector:Back() end

--- 
--- @return int ret0
function util.Vector4Vector:__len() end

--- 
--- @param name int
function util.Vector4Vector:ShrinkToFit(name) end

--- 
--- @return math.Vector4 ret0
function util.Vector4Vector:Front() end

--- 
--- @return table ret0
function util.Vector4Vector:ToTable() end

--- 
--- @return int ret0
function util.Vector4Vector:Capacity() end

--- 
--- @param name table
function util.Vector4Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector4
function util.Vector4Vector:Set(arg1, arg2) end

--- 
function util.Vector4Vector:Clear() end

--- 
--- @return int ret0
function util.Vector4Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Vector4
function util.Vector4Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Vector4 ret0
function util.Vector4Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Vector4
--- @overload fun(name: int): 
function util.Vector4Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Vector4Vector:Reserve(name) end

--- 
--- @param name math.Vector4
function util.Vector4Vector:PushBack(name) end

--- 
--- @param name int
function util.Vector4Vector:Erase(name) end


--- 
--- @class util.Mat3Vector
--- @overload fun():util.Mat3Vector
util.Mat3Vector = {}

--- 
--- @return math.Mat3 ret0
function util.Mat3Vector:Back() end

--- 
--- @return int ret0
function util.Mat3Vector:__len() end

--- 
--- @param name int
function util.Mat3Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat3 ret0
function util.Mat3Vector:Front() end

--- 
--- @return table ret0
function util.Mat3Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat3Vector:Capacity() end

--- 
--- @param name table
function util.Mat3Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3
function util.Mat3Vector:Set(arg1, arg2) end

--- 
function util.Mat3Vector:Clear() end

--- 
--- @return int ret0
function util.Mat3Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3
function util.Mat3Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat3 ret0
function util.Mat3Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat3
--- @overload fun(name: int): 
function util.Mat3Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat3Vector:Reserve(name) end

--- 
--- @param name math.Mat3
function util.Mat3Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat3Vector:Erase(name) end


--- 
--- @class util.Mat4x2Vector
--- @overload fun():util.Mat4x2Vector
util.Mat4x2Vector = {}

--- 
--- @return math.Mat4x2 ret0
function util.Mat4x2Vector:Back() end

--- 
--- @return int ret0
function util.Mat4x2Vector:__len() end

--- 
--- @param name int
function util.Mat4x2Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat4x2 ret0
function util.Mat4x2Vector:Front() end

--- 
--- @return table ret0
function util.Mat4x2Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat4x2Vector:Capacity() end

--- 
--- @param name table
function util.Mat4x2Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4x2
function util.Mat4x2Vector:Set(arg1, arg2) end

--- 
function util.Mat4x2Vector:Clear() end

--- 
--- @return int ret0
function util.Mat4x2Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4x2
function util.Mat4x2Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat4x2 ret0
function util.Mat4x2Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4x2
--- @overload fun(name: int): 
function util.Mat4x2Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat4x2Vector:Reserve(name) end

--- 
--- @param name math.Mat4x2
function util.Mat4x2Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat4x2Vector:Erase(name) end


--- 
--- @class util.Mat4x3Vector
--- @overload fun():util.Mat4x3Vector
util.Mat4x3Vector = {}

--- 
--- @return math.Mat4x3 ret0
function util.Mat4x3Vector:Back() end

--- 
--- @return int ret0
function util.Mat4x3Vector:__len() end

--- 
--- @param name int
function util.Mat4x3Vector:ShrinkToFit(name) end

--- 
--- @return math.Mat4x3 ret0
function util.Mat4x3Vector:Front() end

--- 
--- @return table ret0
function util.Mat4x3Vector:ToTable() end

--- 
--- @return int ret0
function util.Mat4x3Vector:Capacity() end

--- 
--- @param name table
function util.Mat4x3Vector:SetValues(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4x3
function util.Mat4x3Vector:Set(arg1, arg2) end

--- 
function util.Mat4x3Vector:Clear() end

--- 
--- @return int ret0
function util.Mat4x3Vector:Size() end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4x3
function util.Mat4x3Vector:Insert(arg1, arg2) end

--- 
--- @param name int
--- @return math.Mat4x3 ret0
function util.Mat4x3Vector:At(name) end

--- 
--- @param arg1 int
--- @param arg2 math.Mat4x3
--- @overload fun(name: int): 
function util.Mat4x3Vector:Resize(arg1, arg2) end

--- 
--- @param name int
function util.Mat4x3Vector:Reserve(name) end

--- 
--- @param name math.Mat4x3
function util.Mat4x3Vector:PushBack(name) end

--- 
--- @param name int
function util.Mat4x3Vector:Erase(name) end


--- 
--- @class util.Callback
util.Callback = {}

--- 
function util.Callback:IsValid() end

--- 
--- @param arg0 any
--- @param arg1 any
--- @param arg2 any
--- @param arg3 any
--- @param arg4 any
--- @param arg5 any
--- @param arg6 any
--- @param arg7 any
--- @param arg8 any
--- @param arg9 any
--- @param arg10 any
--- @param arg11 any
--- @param arg12 any
--- @param arg13 any
--- @param arg14 any
--- @param arg15 any
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any, arg9: any, arg10: any, arg11: any, arg12: any, arg13: any, arg14: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any, arg9: any, arg10: any, arg11: any, arg12: any, arg13: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any, arg9: any, arg10: any, arg11: any, arg12: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any, arg9: any, arg10: any, arg11: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any, arg9: any, arg10: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any, arg9: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any, arg8: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any, arg7: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any, arg6: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any, arg4: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any, arg3: any): 
--- @overload fun(arg0: any, arg1: any, arg2: any): 
--- @overload fun(arg0: any, arg1: any): 
--- @overload fun(arg0: any): 
--- @overload fun(): 
function util.Callback:Call(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) end

--- 
function util.Callback:__tostring() end

--- 
function util.Callback:Remove() end


--- 
--- @class util.IntProperty
--- @overload fun(arg1: int):util.IntProperty
--- @overload fun():util.IntProperty
util.IntProperty = {}

--- 
function util.IntProperty:__tostring() end

--- 
--- @param oCallback any
function util.IntProperty:AddCallback(oCallback) end

--- 
function util.IntProperty:InvokeCallbacks() end

--- 
function util.IntProperty:Get() end

--- 
--- @param oCallback any
function util.IntProperty:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.IntProperty:SetLocked(bLocked) end

--- 
function util.IntProperty:Unlink() end

--- 
function util.IntProperty:IsLocked() end

--- 
--- @param val int
function util.IntProperty:Set(val) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: util.IntProperty): 
--- @overload fun(arg1: int): 
function util.IntProperty:__le(arg1) end

--- 
--- @param propOther util.FloatProperty
--- @overload fun(propOther: util.IntProperty): 
--- @overload fun(propOther: util.StringProperty): 
--- @overload fun(propOther: util.BoolProperty): 
function util.IntProperty:Link(propOther) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: util.IntProperty): 
--- @overload fun(arg1: int): 
function util.IntProperty:__add(arg1) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: util.IntProperty): 
--- @overload fun(arg1: int): 
function util.IntProperty:__sub(arg1) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: util.IntProperty): 
--- @overload fun(arg1: int): 
function util.IntProperty:__mul(arg1) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: util.IntProperty): 
--- @overload fun(arg1: int): 
function util.IntProperty:__div(arg1) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: util.IntProperty): 
--- @overload fun(arg1: int): 
function util.IntProperty:__lt(arg1) end

--- 
--- @param 1 util.IntProperty
--- @overload fun(arg1: int): 
--- @overload fun(arg1: util.IntProperty): 
function util.IntProperty:__eq(arg1) end


--- 
--- @class util.Vector2iProperty: util.BaseProperty
--- @overload fun(arg1: int, arg2: int):util.Vector2iProperty
--- @overload fun(arg1: math.Vector2i):util.Vector2iProperty
--- @overload fun():util.Vector2iProperty
util.Vector2iProperty = {}

--- 
--- @param 1 number
function util.Vector2iProperty:__div(arg1) end

--- 
function util.Vector2iProperty:__tostring() end

--- 
--- @param oCallback any
function util.Vector2iProperty:AddCallback(oCallback) end

--- 
function util.Vector2iProperty:InvokeCallbacks() end

--- 
function util.Vector2iProperty:Get() end

--- 
--- @param 1 util.Vector2iProperty
--- @overload fun(arg1: math.Vector2i): 
--- @overload fun(arg1: util.Vector2iProperty): 
function util.Vector2iProperty:__eq(arg1) end

--- 
--- @param oCallback any
function util.Vector2iProperty:AddModifier(oCallback) end

--- 
function util.Vector2iProperty:Unlink() end

--- 
--- @param val math.Vector2i
function util.Vector2iProperty:Set(val) end

--- 
--- @param bLocked bool
function util.Vector2iProperty:SetLocked(bLocked) end

--- 
function util.Vector2iProperty:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.Vector2Property): 
--- @overload fun(propOther: util.Vector2iProperty): 
function util.Vector2iProperty:Link(propOther) end

--- 
--- @param 1 util.Vector2iProperty
--- @overload fun(arg1: util.Vector2iProperty): 
--- @overload fun(arg1: math.Vector2i): 
function util.Vector2iProperty:__add(arg1) end

--- 
--- @param 1 util.Vector2iProperty
--- @overload fun(arg1: util.Vector2iProperty): 
--- @overload fun(arg1: math.Vector2i): 
function util.Vector2iProperty:__sub(arg1) end

--- 
--- @param 1 util.Vector2iProperty
--- @overload fun(arg1: number): 
function util.Vector2iProperty:__mul(arg1) end


--- 
--- @class util.VectoriProperty: util.BaseProperty
--- @overload fun(arg1: int, arg2: int, arg3: int):util.VectoriProperty
--- @overload fun(arg1: math.Vectori):util.VectoriProperty
--- @overload fun():util.VectoriProperty
util.VectoriProperty = {}

--- 
--- @param 1 number
function util.VectoriProperty:__div(arg1) end

--- 
function util.VectoriProperty:__tostring() end

--- 
--- @param oCallback any
function util.VectoriProperty:AddCallback(oCallback) end

--- 
function util.VectoriProperty:InvokeCallbacks() end

--- 
function util.VectoriProperty:Get() end

--- 
--- @param 1 util.VectoriProperty
--- @overload fun(arg1: math.Vectori): 
--- @overload fun(arg1: util.VectoriProperty): 
function util.VectoriProperty:__eq(arg1) end

--- 
--- @param oCallback any
function util.VectoriProperty:AddModifier(oCallback) end

--- 
function util.VectoriProperty:Unlink() end

--- 
--- @param val math.Vectori
function util.VectoriProperty:Set(val) end

--- 
--- @param bLocked bool
function util.VectoriProperty:SetLocked(bLocked) end

--- 
function util.VectoriProperty:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.VectorProperty): 
--- @overload fun(propOther: util.VectoriProperty): 
function util.VectoriProperty:Link(propOther) end

--- 
--- @param 1 util.VectoriProperty
--- @overload fun(arg1: util.VectoriProperty): 
--- @overload fun(arg1: math.Vectori): 
function util.VectoriProperty:__add(arg1) end

--- 
--- @param 1 util.VectoriProperty
--- @overload fun(arg1: util.VectoriProperty): 
--- @overload fun(arg1: math.Vectori): 
function util.VectoriProperty:__sub(arg1) end

--- 
--- @param 1 util.VectoriProperty
--- @overload fun(arg1: number): 
function util.VectoriProperty:__mul(arg1) end


--- 
--- @class util.Vector2Property: util.BaseProperty
--- @overload fun(arg1: number, arg2: number):util.Vector2Property
--- @overload fun(arg1: math.Vector2):util.Vector2Property
--- @overload fun():util.Vector2Property
util.Vector2Property = {}

--- 
--- @param 1 number
function util.Vector2Property:__div(arg1) end

--- 
function util.Vector2Property:__tostring() end

--- 
--- @param oCallback any
function util.Vector2Property:AddCallback(oCallback) end

--- 
function util.Vector2Property:InvokeCallbacks() end

--- 
function util.Vector2Property:Get() end

--- 
--- @param 1 util.Vector2Property
--- @overload fun(arg1: math.Vector2): 
--- @overload fun(arg1: util.Vector2Property): 
function util.Vector2Property:__eq(arg1) end

--- 
--- @param oCallback any
function util.Vector2Property:AddModifier(oCallback) end

--- 
function util.Vector2Property:Unlink() end

--- 
--- @param val math.Vector2
function util.Vector2Property:Set(val) end

--- 
--- @param bLocked bool
function util.Vector2Property:SetLocked(bLocked) end

--- 
function util.Vector2Property:IsLocked() end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.Vector2iProperty): 
--- @overload fun(propOther: util.Vector2Property): 
function util.Vector2Property:Link(propOther) end

--- 
--- @param 1 util.Vector2Property
--- @overload fun(arg1: util.Vector2Property): 
--- @overload fun(arg1: math.Vector2): 
function util.Vector2Property:__add(arg1) end

--- 
--- @param 1 util.Vector2Property
--- @overload fun(arg1: util.Vector2Property): 
--- @overload fun(arg1: math.Vector2): 
function util.Vector2Property:__sub(arg1) end

--- 
--- @param 1 util.Vector2Property
--- @overload fun(arg1: number): 
function util.Vector2Property:__mul(arg1) end


--- 
--- @class util.QuaternionProperty: util.BaseProperty
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):util.QuaternionProperty
--- @overload fun():util.QuaternionProperty
util.QuaternionProperty = {}

--- 
function util.QuaternionProperty:__tostring() end

--- 
--- @param oCallback any
function util.QuaternionProperty:AddCallback(oCallback) end

--- 
function util.QuaternionProperty:Get() end

--- 
function util.QuaternionProperty:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.QuaternionProperty:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.QuaternionProperty:SetLocked(bLocked) end

--- 
function util.QuaternionProperty:Unlink() end

--- 
function util.QuaternionProperty:IsLocked() end

--- 
--- @param val math.Quaternion
function util.QuaternionProperty:Set(val) end

--- 
--- @param propOther util.StringProperty
--- @overload fun(propOther: util.QuaternionProperty): 
function util.QuaternionProperty:Link(propOther) end


--- 
--- @class util.Mat4x3Property: util.BaseProperty
--- @overload fun(arg1: math.Mat4x3):util.Mat4x3Property
--- @overload fun():util.Mat4x3Property
util.Mat4x3Property = {}

--- 
function util.Mat4x3Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat4x3Property:AddCallback(oCallback) end

--- 
function util.Mat4x3Property:Get() end

--- 
function util.Mat4x3Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat4x3Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat4x3Property:SetLocked(bLocked) end

--- 
function util.Mat4x3Property:Unlink() end

--- 
function util.Mat4x3Property:IsLocked() end

--- 
--- @param val math.Mat4x3
function util.Mat4x3Property:Set(val) end

--- 
--- @param propOther util.Mat4x3Property
function util.Mat4x3Property:Link(propOther) end


--- 
--- @class util.Mat4Property: util.BaseProperty
--- @overload fun(arg1: math.Mat4):util.Mat4Property
--- @overload fun():util.Mat4Property
util.Mat4Property = {}

--- 
function util.Mat4Property:__tostring() end

--- 
--- @param oCallback any
function util.Mat4Property:AddCallback(oCallback) end

--- 
function util.Mat4Property:Get() end

--- 
function util.Mat4Property:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.Mat4Property:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.Mat4Property:SetLocked(bLocked) end

--- 
function util.Mat4Property:Unlink() end

--- 
function util.Mat4Property:IsLocked() end

--- 
--- @param val math.Mat4
function util.Mat4Property:Set(val) end

--- 
--- @param propOther util.Mat4Property
function util.Mat4Property:Link(propOther) end


--- 
--- @class util.HairData
util.HairData = {}


--- 
--- @class util.DataStream
--- @overload fun(arg1: int):util.DataStream
--- @overload fun():util.DataStream
util.DataStream = {}

--- 
function util.DataStream:ReadMat2() end

--- 
--- @param t int
function util.DataStream:WriteInt8(t) end

--- 
function util.DataStream:ReadVertex() end

--- 
--- @param offset int
function util.DataStream:Seek(offset) end

--- 
function util.DataStream:ReadVectori() end

--- 
function util.DataStream:ReadMat2x4() end

--- 
--- @param t number
function util.DataStream:WriteDouble(t) end

--- 
function util.DataStream:ReadVector4i() end

--- 
--- @param size int
function util.DataStream:Resize(size) end

--- 
function util.DataStream:GetSize() end

--- 
function util.DataStream:Tell() end

--- 
function util.DataStream:Clear() end

--- 
--- @param size int
function util.DataStream:Reserve(size) end

--- 
--- @param t math.Vector
function util.DataStream:WriteVector(t) end

--- 
--- @param size int
function util.DataStream:ReadBinaryString(size) end

--- 
function util.DataStream:ReadBool() end

--- 
function util.DataStream:ReadChar() end

--- 
--- @param str string
--- @param bNullterminated bool
--- @overload fun(str: string): 
function util.DataStream:WriteString(str, bNullterminated) end

--- 
function util.DataStream:ToBinaryString() end

--- 
function util.DataStream:ReadMat3x2() end

--- 
--- @param pattern string
function util.DataStream:ReadStringUntil(pattern) end

--- 
function util.DataStream:ReadVector2() end

--- 
--- @param t int
function util.DataStream:WriteInt32(t) end

--- 
--- @param t math.Vector4
function util.DataStream:WriteVector4(t) end

--- 
function util.DataStream:ReadVector4() end

--- 
function util.DataStream:ReadVector() end

--- 
function util.DataStream:ReadLine() end

--- 
--- @param t int
function util.DataStream:WriteInt64(t) end

--- 
--- @param t number
function util.DataStream:WriteLongDouble(t) end

--- 
--- @param dsOther util.DataStream
--- @overload fun(dsOther: util.DataStream, offset: int, size: int): 
function util.DataStream:WriteBinary(dsOther) end

--- 
--- @overload fun(size: int): 
function util.DataStream:ReadBinary() end

--- 
--- @param t bool
function util.DataStream:WriteBool(t) end

--- 
function util.DataStream:ReadInt64() end

--- 
--- @param t math.Quaternion
function util.DataStream:WriteQuaternion(t) end

--- 
function util.DataStream:ReadMat2x3() end

--- 
--- @param t int
function util.DataStream:WriteUInt64(t) end

--- 
--- @param t math.Vectori
function util.DataStream:WriteVectori(t) end

--- 
function util.DataStream:ReadMat3x4() end

--- 
function util.DataStream:ReadUInt64() end

--- 
function util.DataStream:ReadInt8() end

--- 
--- @param t int
function util.DataStream:WriteUInt8(t) end

--- 
function util.DataStream:ReadUInt8() end

--- 
--- @param t math.Vector2
function util.DataStream:WriteVector2(t) end

--- 
function util.DataStream:ReadVector2i() end

--- 
function util.DataStream:ReadInt32() end

--- 
--- @param t int
function util.DataStream:WriteUInt32(t) end

--- 
function util.DataStream:ReadUInt32() end

--- 
--- @param t int
function util.DataStream:WriteInt16(t) end

--- 
function util.DataStream:ReadLongDouble() end

--- 
function util.DataStream:ReadInt16() end

--- 
--- @param t int
function util.DataStream:WriteUInt16(t) end

--- 
function util.DataStream:ReadUInt16() end

--- 
--- @param t int
function util.DataStream:WriteChar(t) end

--- 
--- @param t math.Mat2x4
function util.DataStream:WriteMat2x4(t) end

--- 
--- @param t number
function util.DataStream:WriteFloat(t) end

--- 
function util.DataStream:ReadFloat() end

--- 
function util.DataStream:ReadDouble() end

--- 
function util.DataStream:ReadQuaternion() end

--- 
function util.DataStream:ReadMat4() end

--- 
function util.DataStream:ReadMat4x3() end

--- 
function util.DataStream:ReadMat4x2() end

--- 
function util.DataStream:ReadMat3() end

--- 
--- @param len int
--- @overload fun(): 
function util.DataStream:ReadString(len) end

--- 
--- @param t math.EulerAngles
function util.DataStream:WriteAngles(t) end

--- 
function util.DataStream:ReadAngles() end

--- 
--- @param t math.Mat2
function util.DataStream:WriteMat2(t) end

--- 
--- @param t math.Mat2x3
function util.DataStream:WriteMat2x3(t) end

--- 
function util.DataStream:__tostring() end

--- 
--- @param t math.Mat3x2
function util.DataStream:WriteMat3x2(t) end

--- 
--- @param t math.Mat3
function util.DataStream:WriteMat3(t) end

--- 
--- @param t math.Mat3x4
function util.DataStream:WriteMat3x4(t) end

--- 
--- @param t math.Mat4x2
function util.DataStream:WriteMat4x2(t) end

--- 
--- @param t math.Mat4x3
function util.DataStream:WriteMat4x3(t) end

--- 
--- @param t math.Mat4
function util.DataStream:WriteMat4(t) end

--- 
--- @param t math.Vector2i
function util.DataStream:WriteVector2i(t) end

--- 
--- @param t math.Vector4i
function util.DataStream:WriteVector4i(t) end

--- 
--- @param t game.Model.Vertex
function util.DataStream:WriteVertex(t) end


--- 
--- @class util.Color
--- @field r int 
--- @field b int 
--- @field g int 
--- @field a int 
--- @overload fun(arg1: string):util.Color
--- @overload fun(arg1: math.Vector4):util.Color
--- @overload fun(arg1: math.Vector):util.Color
--- @overload fun(arg1: int, arg2: int, arg3: int, arg4: int):util.Color
--- @overload fun(arg1: int, arg2: int, arg3: int):util.Color
--- @overload fun():util.Color
util.Color = {}

--- 
--- @param 1 util.Color
function util.Color:__eq(arg1) end

--- 
--- @param 1 util.Color
function util.Color:__add(arg1) end

--- 
--- @param 1 number
function util.Color:__div(arg1) end

--- 
--- @param 1 util.Color
function util.Color:__sub(arg1) end

--- 
function util.Color:__tostring() end

--- 
--- @param 1 util.Color
--- @overload fun(arg1: number): 
function util.Color:__mul(arg1) end

--- 
function util.Color:ToVector() end

--- 
function util.Color:ToVector4() end

--- 
function util.Color:ToHexColor() end

--- 
function util.Color:ToHexColorRGB() end

--- 
function util.Color:ToHSVColor() end

--- 
function util.Color:GetComplementaryColor() end

--- 
function util.Color:GetContrastColor() end

--- 
function util.Color:Copy() end

--- 
function util.Color:CalcPerceivedLuminance() end

--- 
--- @param r int
--- @param g int
--- @param b int
--- @param a int
--- @overload fun(arg1: util.Color): 
function util.Color:Set(r, g, b, a) end

--- 
--- @param other util.Color
--- @param factor number
function util.Color:Lerp(other, factor) end


--- 
--- @class util.StringProperty: util.BaseProperty
--- @overload fun(arg1: string):util.StringProperty
--- @overload fun():util.StringProperty
util.StringProperty = {}

--- 
function util.StringProperty:__tostring() end

--- 
--- @param oCallback any
function util.StringProperty:AddCallback(oCallback) end

--- 
function util.StringProperty:Get() end

--- 
function util.StringProperty:InvokeCallbacks() end

--- 
--- @param oCallback any
function util.StringProperty:AddModifier(oCallback) end

--- 
--- @param bLocked bool
function util.StringProperty:SetLocked(bLocked) end

--- 
function util.StringProperty:Unlink() end

--- 
function util.StringProperty:IsLocked() end

--- 
--- @param val string
function util.StringProperty:Set(val) end

--- 
--- @param propOther util.EulerAnglesProperty
--- @overload fun(propOther: util.QuaternionProperty): 
--- @overload fun(propOther: util.Vector4iProperty): 
--- @overload fun(propOther: util.Vector4Property): 
--- @overload fun(propOther: util.VectoriProperty): 
--- @overload fun(propOther: util.VectorProperty): 
--- @overload fun(propOther: util.Vector2iProperty): 
--- @overload fun(propOther: util.Vector2Property): 
--- @overload fun(propOther: util.IntProperty): 
--- @overload fun(propOther: util.FloatProperty): 
--- @overload fun(propOther: util.ColorProperty): 
--- @overload fun(propOther: util.BoolProperty): 
--- @overload fun(propOther: util.StringProperty): 
function util.StringProperty:Link(propOther) end


--- 
--- @class util.HairConfig
--- @field numSegments int 
--- @field hairPerSquareMeter number 
--- @field defaultThickness number 
--- @field defaultLength number 
--- @field curvature number 
--- @field defaultHairStrength number 
--- @field randomHairLengthFactor number 
--- @overload fun():util.HairConfig
util.HairConfig = {}


--- 
--- @class util.TextureInfo
--- @field wrapMode int 
--- @field flags int 
--- @field inputFormat int 
--- @field outputFormat int 
--- @field mipMapFilter int 
--- @field containerFormat int 
--- @overload fun():util.TextureInfo
util.TextureInfo = {}

--- 
function util.TextureInfo:SetNormalMap() end


--- @enum InputFormat
util.TextureInfo = {
	INPUT_FORMAT_R32_FLOAT = 4,
	INPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT = 0,
	INPUT_FORMAT_R16G16B16A16_FLOAT = 2,
	INPUT_FORMAT_R8G8B8A8_UINT = 1,
	INPUT_FORMAT_R32G32B32A32_FLOAT = 3,
}

--- @enum OutputFormatColorMap
util.TextureInfo = {
	OUTPUT_FORMAT_COLOR_MAP_SHARP_ALPHA = 5,
	OUTPUT_FORMAT_COLOR_MAP = 3,
	OUTPUT_FORMAT_COLOR_MAP_SMOOTH_ALPHA = 6,
	OUTPUT_FORMAT_COLOR_MAP_1BIT_ALPHA = 4,
}

--- @enum FlagBit
util.TextureInfo = {
	FLAG_BIT_CONVERT_TO_NORMAL_MAP = 1,
	FLAG_BIT_GENERATE_MIPMAPS = 4,
	FLAG_BIT_SRGB = 2,
}

--- @enum WrapMode
util.TextureInfo = {
	WRAP_MODE_CLAMP = 0,
	WRAP_MODE_REPEAT = 1,
	WRAP_MODE_MIRROR = 2,
}

--- @enum MipmapFilter
util.TextureInfo = {
	MIPMAP_FILTER_KAISER = 1,
	MIPMAP_FILTER_BOX = 0,
}

--- @enum OutputFormat
util.TextureInfo = {
	OUTPUT_FORMAT_BC1 = 8,
	OUTPUT_FORMAT_BC2 = 10,
	OUTPUT_FORMAT_HDR_COLOR_MAP = 17,
	OUTPUT_FORMAT_BC1A = 9,
	OUTPUT_FORMAT_BC3 = 11,
	OUTPUT_FORMAT_BC3N = 12,
	OUTPUT_FORMAT_BC3_RGBM = 19,
	OUTPUT_FORMAT_BC4 = 13,
	OUTPUT_FORMAT_BC5 = 14,
	OUTPUT_FORMAT_BC6 = 17,
	OUTPUT_FORMAT_BC7 = 18,
	OUTPUT_FORMAT_CTX1 = 16,
	OUTPUT_FORMAT_DXT1 = 3,
	OUTPUT_FORMAT_DXT1A = 4,
	OUTPUT_FORMAT_DXT1N = 15,
	OUTPUT_FORMAT_DXT3 = 5,
	OUTPUT_FORMAT_DXT5 = 6,
	OUTPUT_FORMAT_DXT5N = 7,
	OUTPUT_FORMAT_GRADIENT_MAP = 13,
	OUTPUT_FORMAT_ETC1 = 20,
	OUTPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT = 0,
	OUTPUT_FORMAT_NORMAL_MAP = 6,
	OUTPUT_FORMAT_RGB = 1,
	OUTPUT_FORMAT_RGBA = 2,
}

--- @enum OutputFormatEtc2
util.TextureInfo = {
	OUTPUT_FORMAT_ETC2_RGBA = 24,
	OUTPUT_FORMAT_ETC2_R = 21,
	OUTPUT_FORMAT_ETC2_RGB = 23,
	OUTPUT_FORMAT_ETC2_RG = 22,
	OUTPUT_FORMAT_ETC2_RGB_A1 = 25,
	OUTPUT_FORMAT_ETC2_RGBM = 26,
}

--- @enum ContainerFormat
util.TextureInfo = {
	CONTAINER_FORMAT_DDS = 0,
	CONTAINER_FORMAT_KTX = 1,
}

--- @enum FlagNone
util.TextureInfo = {
	FLAG_NONE = 0,
}

--- 
--- @class util.Texture
util.Texture = {}

--- 
--- @return int ret0
function util.Texture:GetWidth() end

--- 
--- @return int ret0
function util.Texture:GetHeight() end

--- 
--- @return prosper.Texture ret0
function util.Texture:GetVkTexture() end


--- 
--- @class pfm.util
pfm.util = {}


