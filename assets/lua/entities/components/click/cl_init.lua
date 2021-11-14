util.register_class("ents.ClickComponent",BaseEntityComponent)

local numClickComponents = 0
local cbClick
function ents.ClickComponent:Initialize()
	BaseEntityComponent.Initialize(self)
	if(numClickComponents == 0) then
		local pl = ents.get_local_player()
		if(pl ~= nil) then
			cbClick = pl:AddEventCallback(ents.PlayerComponent.EVENT_HANDLE_ACTION_INPUT,function(action,pressed,magnitude)
				local handled = ents.ClickComponent.inject_click_input(action,pressed)
				return handled
			end)
		end
	end
	numClickComponents = numClickComponents +1
end
local function get_viewport_data()
	local cursorPos = input.get_cursor_pos()
	local vpData = {}
	-- Check if element under cursor is a viewport
	local elFocus = gui.get_focused_element()
	if(util.is_valid(elFocus)) then
		local viewport
		if(elFocus:GetClass() == "wiviewport") then viewport = elFocus
		else
			-- Check if the focused element has a viewport
			for vp,_ in pairs(gui.WIViewport.get_viewports()) do
				if(vp:IsValid() and vp:IsVisible() and vp:IsDescendantOf(elFocus)) then
					if(viewport == nil) then viewport = vp
					else
						-- Found more than one viewport; Pick the closest one to the mouse cursor
						local pos0 = viewport:GetAbsolutePos()
						local d0 = geometry.closest_point_on_aabb_to_point(pos0,pos0 +viewport:GetSize(),cursorPos)

						local pos1 = vp:GetAbsolutePos()
						local d1 = geometry.closest_point_on_aabb_to_point(pos1,pos1 +vp:GetSize(),cursorPos)
						if(d1 < d0) then -- Viewport is closer
							viewport = vp
						end
					end
				end
			end
		end
		if(viewport ~= nil) then
			-- Found viewport! Using viewport as base for click input
			local absPos = viewport:GetAbsolutePos()
			vpData.width = viewport:GetWidth()
			vpData.height = viewport:GetHeight()
			vpData.x = absPos.x
			vpData.y = absPos.y
			vpData.cursorPos = viewport:GetCursorPos()
			vpData.camera = viewport:GetCamera()
			vpData.viewport = viewport
			return vpData
		end
	end

	local scene = game.get_render_scene()
	local cam = scene:GetActiveCamera()
	local res = engine.get_window_resolution()
	vpData.width = res.x
	vpData.height = res.y
	vpData.x = 0
	vpData.y = 0
	vpData.cursorPos = input.get_cursor_pos()
	vpData.camera = cam
	return vpData
end
local lastActorsClicked = {}
function ents.ClickComponent.inject_click_input(action,pressed,filter)
	if(action ~= input.ACTION_ATTACK and action ~= input.ACTION_ATTACK2) then return util.EVENT_REPLY_UNHANDLED end
	if(util.is_valid(lastActorsClicked[action])) then
		local actor = lastActorsClicked[action]
		lastActorsClicked[action] = nil
		local handled = actor:BroadcastEvent(ents.ClickComponent.EVENT_ON_CLICK,{action,false})
		if(pressed == false and handled == util.EVENT_REPLY_HANDLED) then return handled end
	end
	if(pressed == false) then return util.EVENT_REPLY_UNHANDLED end
	local clickActor,hitPos,startPos = ents.ClickComponent.find_actor_under_cursor(filter)
	local clickC = (clickActor ~= nil) and clickActor:GetComponent(ents.COMPONENT_CLICK) or nil
	if(clickC ~= nil) then
		lastActorsClicked[action] = clickC
		return (clickC:BroadcastEvent(ents.ClickComponent.EVENT_ON_CLICK,{action,true,hitPos}) or util.EVENT_REPLY_UNHANDLED),clickActor,hitPos,startPos
	end
	return util.EVENT_REPLY_UNHANDLED,clickActor,hitPos,startPos
end
function ents.ClickComponent.get_ray_data()
	local vpData = get_viewport_data()

	local cam = vpData.camera
	if(util.is_valid(cam) == false) then return end
	local dir = cam:CalcRayDirection(Vector2(vpData.cursorPos.x /vpData.width,vpData.cursorPos.y /vpData.height))
	local entCam = cam:GetEntity()
	return entCam:GetPos() +entCam:GetForward() *cam:GetNearZ(),dir
end
function ents.ClickComponent.get_camera()
	local vpData = get_viewport_data()
	return vpData.camera
end
function ents.ClickComponent.get_viewport_data() return get_viewport_data() end
function ents.ClickComponent.raycast(pos,dir,filter)
	local pl = ents.get_local_player()
	if(pl == nil) then return end
	local entPl = pl:GetEntity()
	--[[local drawInfo = debug.DrawInfo()
	drawInfo:SetDuration(12)
	drawInfo:SetColor(Color.Aqua)
	debug.draw_line(pos,pos +dir *1000,drawInfo)]]
	local distClosest = math.huge
	local actorClosest = nil
	local hitPos
	for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_CLICK),ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER)}) do
		local mdl = ent:GetModel()
		local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
		if(mdl ~= nil and ent ~= entPl and renderC ~= nil and renderC:GetSceneRenderPass() ~= game.SCENE_RENDER_PASS_VIEW and renderC:GetSceneRenderPass() ~= game.SCENE_RENDER_PASS_NONE and (filter == nil or filter(ent,renderC) == true)) then
			local r,hitData = renderC:CalcRayIntersection(pos,dir *32768,false)
			-- print("Intersection with ",ent,": ",r)
			-- Note: Distance of 0 usually means we're inside the object, in which case we probably don't intend to select it
			if(r == intersect.RESULT_INTERSECT and hitData.distance < distClosest) then -- and hitData.distance > 0.0) then
				-- print("Clicked actor: ",ent)
				distClosest = hitData.distance
				hitPos = hitData.position
				actorClosest = ent
			end
		end
	end
	return actorClosest,hitPos,pos
end
function ents.ClickComponent.find_actor_under_cursor(filter)
	local pos,dir = ents.ClickComponent.get_ray_data()
	if(pos == nil) then return end
	return ents.ClickComponent.raycast(pos,dir,filter)
end
function ents.ClickComponent:OnRemove()
	numClickComponents = numClickComponents -1
	if(numClickComponents == 0) then
		if(util.is_valid(cbClick)) then cbClick:Remove() end
	end
end
ents.COMPONENT_CLICK = ents.register_component("click",ents.ClickComponent)
ents.ClickComponent.EVENT_ON_CLICK = ents.register_component_event(ents.COMPONENT_CLICK,"on_clicked")
