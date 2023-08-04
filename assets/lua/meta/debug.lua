--- @meta
--- 
--- @class debug
debug = {}

--- 
--- @param v enum pragma::BaseAIComponent::MoveResult
--- @return string ret0
function debug.move_state_to_string(v) end

--- 
function debug.beep() end

--- 
function debug.stackdump() end

--- 
--- @param task enum pragma::ai::Task
--- @return string ret0
function debug.task_to_string(task) end

--- 
--- @param decoratorType enum pragma::ai::TaskDecorator::DecoratorType
--- @return string ret0
function debug.behavior_task_decorator_type_to_string(decoratorType) end

--- 
--- @param disposition enum DISPOSITION
--- @return string ret0
function debug.disposition_to_string(disposition) end

--- 
--- @param npcState enum NPCSTATE
--- @return string ret0
function debug.npc_state_to_string(npcState) end

--- 
--- @param start math.Vector
--- @param end math.Vector
--- @param renderInfo debug.DrawInfo
--- @overload fun(start: vector.Vector, end_: vector.Vector): 
--- @overload fun(start: vector.Vector, end_: vector.Vector, renderInfo: debug.DrawInfo): 
function debug.draw_line(start, end_, renderInfo) end

--- 
--- @param renderInfo debug.DrawInfo
--- @overload fun(n: vector.Vector, d: number, renderInfo: debug.DrawInfo): 
--- @overload fun(plane: math.Plane, renderInfo: debug.DrawInfo): 
function debug.draw_plane(renderInfo) end

--- 
--- @param memoryType enum pragma::ai::Memory::MemoryType
--- @return string ret0
function debug.memory_type_to_string(memoryType) end

--- 
--- @param linePoints table
--- @param renderInfo debug.DrawInfo
--- @return debug.RendererObject ret0
function debug.draw_lines(linePoints, renderInfo) end

--- 
--- @param selectorType enum pragma::ai::SelectorType
--- @return string ret0
function debug.behavior_selector_type_to_string(selectorType) end

--- 
--- @param result enum pragma::ai::BehaviorNode::Result
--- @return string ret0
function debug.behavior_task_result_to_string(result) end

--- 
--- @param type enum pragma::ai::BehaviorNode::Type
--- @return string ret0
function debug.behavior_task_type_to_string(type) end

--- 
--- @param renderInfo debug.DrawInfo
--- @overload fun(renderInfo: debug.DrawInfo): 
function debug.draw_point(renderInfo) end

--- 
--- @param points table
--- @param renderInfo debug.DrawInfo
--- @return debug.RendererObject ret0
--- @overload fun(cam: ents.CameraComponent, renderInfo: debug.DrawInfo): debug.RendererObject
function debug.draw_frustum(points, renderInfo) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param renderInfo debug.DrawInfo
--- @overload fun(start: vector.Vector, end_: vector.Vector, renderInfo: debug.DrawInfo): 
function debug.draw_box(min, max, renderInfo) end

--- 
--- @param radius number
--- @param renderInfo debug.DrawInfo
--- @param recursionLevel int
--- @overload fun(radius: number, renderInfo: debug.DrawInfo): 
--- @overload fun(radius: number, renderInfo: debug.DrawInfo, recursionLevel: int): 
function debug.draw_sphere(radius, renderInfo, recursionLevel) end

--- 
--- @param dist number
--- @param renderInfo debug.DrawInfo
--- @overload fun(dir: vector.Vector, dist: number, angle: number, renderInfo: debug.DrawInfo, segmentCount: int): 
function debug.draw_cone(dist, renderInfo) end

--- 
--- @param startRadius number
--- @param dist number
--- @param endRadius number
--- @param renderInfo debug.DrawInfo
--- @overload fun(startRadius: number, dir: vector.Vector, dist: number, endRadius: number, renderInfo: debug.DrawInfo, segmentCount: int): 
function debug.draw_truncated_cone(startRadius, dist, endRadius, renderInfo) end

--- 
--- @param dist number
--- @param radius number
--- @param renderInfo debug.DrawInfo
--- @overload fun(radius: number, dir: vector.Vector, dist: number, renderInfo: debug.DrawInfo, segmentCount: int): 
function debug.draw_cylinder(dist, radius, renderInfo) end

--- 
--- @param points 
--- @param segmentCount int
--- @param renderInfo debug.DrawInfo
--- @overload fun(points: , segmentCount: int, curvature: number, renderInfo: debug.DrawInfo): 
--- @overload fun(path: table, numSegments: int, renderInfo: debug.DrawInfo, curvature: number): 
function debug.draw_spline(points, segmentCount, renderInfo) end

--- 
--- @param points 
--- @param renderInfo debug.DrawInfo
--- @overload fun(path: table, renderInfo: debug.DrawInfo): 
function debug.draw_path(points, renderInfo) end

--- 
--- @param text string
--- @param size number
--- @param renderInfo debug.DrawInfo
--- @overload fun(text: string, size: math.Vector2, renderInfo: debug.DrawInfo): 
--- @overload fun(text: string, renderInfo: debug.DrawInfo): 
--- @overload fun(text: string, scale: number, renderInfo: debug.DrawInfo): 
--- @overload fun(text: string, size: math.Vector2, renderInfo: debug.DrawInfo): 
function debug.draw_text(text, size, renderInfo) end

--- 
--- @param renderInfo debug.DrawInfo
--- @overload fun(renderInfo: debug.DrawInfo): 
function debug.draw_pose(renderInfo) end

--- 
--- @param points table
--- @param renderInfo debug.DrawInfo
--- @return debug.RendererObject ret0
function debug.draw_points(points, renderInfo) end

--- 
--- @param mesh Model.Mesh.Sub
--- @param renderInfo debug.DrawInfo
--- @return debug.RendererObject ret0
--- @overload fun(verts: table, renderInfo: debug.DrawInfo): debug.RendererObject
function debug.draw_mesh(mesh, renderInfo) end

--- 
--- @param objects table
--- @return debug.RendererObject ret0
function debug.create_collection(objects) end


--- 
--- @class debug.DrawInfo
--- @overload fun(arg1: math.Transform, arg2: util.Color, arg3: util.Color, arg4: number):debug.DrawInfo
--- @overload fun(arg1: math.Transform, arg2: util.Color, arg3: number):debug.DrawInfo
--- @overload fun(arg1: math.Transform, arg2: util.Color, arg3: util.Color):debug.DrawInfo
--- @overload fun(arg1: math.Transform, arg2: util.Color):debug.DrawInfo
--- @overload fun():debug.DrawInfo
debug.DrawInfo = {}

--- 
--- @param arg1 util.Color
--- @return debug.DrawInfo ret0
function debug.DrawInfo:SetColor(arg1) end

--- 
--- @param arg1 math.Vector
--- @return debug.DrawInfo ret0
--- @overload fun(arg1: vector.Vector): debug.DrawInfo
function debug.DrawInfo:SetOrigin(arg1) end

--- 
--- @param arg1 math.Quaternion
--- @return debug.DrawInfo ret0
function debug.DrawInfo:SetRotation(arg1) end

--- 
--- @param arg1 number
--- @return debug.DrawInfo ret0
function debug.DrawInfo:SetDuration(arg1) end

--- 
--- @param arg1 util.Color
--- @return debug.DrawInfo ret0
function debug.DrawInfo:SetOutlineColor(arg1) end


--- 
--- @class debug.RendererObject
debug.RendererObject = {}

--- 
--- @param arg1 math.EulerAngles
function debug.RendererObject:SetAngles(arg1) end

--- 
--- @param arg1 vector.Vector
function debug.RendererObject:SetScale(arg1) end

--- 
--- @return math.EulerAngles ret0
function debug.RendererObject:GetAngles() end

--- 
--- @return vector.Vector ret0
function debug.RendererObject:GetScale() end

--- 
--- @param arg1 bool
function debug.RendererObject:SetVisible(arg1) end

--- 
--- @return math.Quaternion ret0
function debug.RendererObject:GetRotation() end

--- 
--- @return bool ret0
function debug.RendererObject:IsValid() end

--- 
function debug.RendererObject:Remove() end

--- 
--- @param arg1 math.Quaternion
function debug.RendererObject:SetRotation(arg1) end

--- 
--- @param arg1 math.ScaledTransform
function debug.RendererObject:SetPose(arg1) end

--- 
--- @return math.ScaledTransform ret0
function debug.RendererObject:GetPose() end

--- 
--- @return bool ret0
function debug.RendererObject:IsVisible() end

--- 
--- @return vector.Vector ret0
function debug.RendererObject:GetPos() end

--- 
--- @param arg1 vector.Vector
function debug.RendererObject:SetPos(arg1) end


