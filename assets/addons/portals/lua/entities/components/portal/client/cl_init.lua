--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/shaders/reflection/reflection.lua")
include("../shared.lua")

local Component = ents.PortalComponent
function Component:SetResolution(w,h) self.m_resolution = Vector2i(w,h) end
function Component:GetResolution() return self.m_resolution end
function Component:GetReflectionRenderer() return self.m_renderer end
function Component:GetReflectionScene() return self.m_scene end
function Component:GetReflectionCamera() return self.m_camera end

function Component:SetDebugWireframeCameraEnabled(enabled)
	if(util.is_valid(self.m_camera) == false) then return end
	if(enabled) then self.m_debugWireframeCamera = self.m_camera:GetEntity():AddComponent("wireframe_camera")
	else self.m_camera:GetEntity():RemoveComponent("wireframe_camera") end
end

function Component:SetDebugOverlayEnabled(enabled)
	if(enabled) then self:InitializeDebugOverlay()
	else util.remove(self.m_dbgElTex) end
end

function Component:InitializeReflectionScene()
	self:CreateScene(self.m_resolution.x,self.m_resolution.y)

	if(util.is_valid(self.m_cbRenderScenes) == false) then
		self.m_cbRenderScenes = game.add_callback("RenderScenes",function(drawSceneInfo)
			self:InvokeEventCallbacks(Component.EVENT_PRE_RENDER_SCENE,{drawSceneInfo})
			self:UpdateCamera()
			game.queue_scene_for_rendering(self.m_drawSceneInfo)
		end)
	end

	self:BroadcastEvent(Component.EVENT_ON_RENDER_SCENE_INITIALIZED)
end

function Component:InitializeDebugOverlay()
	if(util.is_valid(self.m_dbgElTex)) then return end
	local elTex = gui.create("WITexturedRect")
	elTex:SetSize(512,512)
	elTex:SetTexture(self.m_renderer:GetPresentationTexture())
	self.m_dbgElTex = elTex
end

function Component:SetDebugCameraFrozen(frozen)
	if(frozen == false) then
		self.m_frozenCamData = nil
		return
	end
	local gameScene = game.get_scene()
	local gameCam = gameScene:GetActiveCamera()
	self.m_frozenCamData = {
		viewMatrix = gameCam:GetViewMatrix(),
		pose = gameCam:GetEntity():GetPose()
	}
end

function Component:UpdateCamera()
	local n = self.m_plane:GetNormal()
	local d = self.m_plane:GetDistance()
	local matReflect = matrix.create_reflection(n,0.0)

	local gameScene = game.get_scene()
	local gameCam = gameScene:GetActiveCamera()
	local vm
	local camPose
	if(self.m_frozenCamData ~= nil) then
		vm = self.m_frozenCamData.viewMatrix:Copy()
		camPose = self.m_frozenCamData.pose:Copy()
	else
		vm = gameCam:GetViewMatrix()
		camPose = gameCam:GetEntity():GetPose()
	end

	local srcPos = self:GetSurfacePose():GetOrigin()
	local targetPos = self:GetTargetPose():GetOrigin()
	local srcRot = self:GetSurfacePose():GetRotation()
	local targetRot = self:GetTargetPose():GetRotation()

	local targetMatrix = Mat4(1.0)
	local mirror = self:IsMirrored()

	local offset = targetPos -srcPos
	if(mirror) then offset.x = -offset.x
	else offset = -offset end
	targetMatrix:Translate(offset)
	local targetRotMat = Mat4(1.0)

	targetRotMat:Rotate(srcRot:GetInverse() *targetRot)
	vm = vm
	vm:Translate(srcPos)

	local tmp = Vector(vm:Get(3,0),vm:Get(3,1),vm:Get(3,2))
	vm:Set(3,0,0); vm:Set(3,1,0); vm:Set(3,1,0) -- Temporarily disable translation, since we don't want to affect it
	vm = vm *targetRotMat
	vm:Set(3,0,tmp.x); vm:Set(3,1,tmp.y); vm:Set(3,2,tmp.z)

	vm = vm *targetMatrix
	if(mirror) then vm = vm *matReflect end
	vm:Translate(-srcPos)

	local cam = self.m_camera
	cam:SetViewMatrix(vm)

	local newCamPose = self:ProjectPoseToTarget(camPose)
	cam:GetEntity():SetPose(newCamPose)
end

function Component:ClearScene()
	for _,c in ipairs({self.m_renderer,self.m_scene,self.m_camera}) do
		if(c:IsValid()) then util.remove(c:GetEntity()) end
	end
end

function Component:CreateScene(w,h)
	self:ClearScene()
	local sceneCreateInfo = ents.SceneComponent.CreateInfo()
	sceneCreateInfo.sampleCount = prosper.SAMPLE_COUNT_1_BIT
	local gameScene = game.get_scene()
	local scene = ents.create_scene(sceneCreateInfo,gameScene)
	scene:Link(gameScene)
	scene:GetEntity():SetName("scene_reflection")

	local entRenderer = ents.create("rasterization_renderer")
	local renderer = entRenderer:GetComponent(ents.COMPONENT_RENDERER)
	local rasterizer = entRenderer:GetComponent(ents.COMPONENT_RASTERIZATION_RENDERER)
	rasterizer:SetSSAOEnabled(true)
	renderer:InitializeRenderTarget(scene,w,h)

	self.m_drawSceneInfo.scene = scene
	self.m_drawSceneInfo.outputImage = renderer:GetPresentationTexture():GetImage()

	scene:SetRenderer(renderer)

	local gameScene = game.get_scene()
	local gameCam = gameScene:GetActiveCamera()
	local cam = ents.create_camera(gameCam:GetAspectRatio(),gameCam:GetFOV(),gameCam:GetNearZ(),gameCam:GetFarZ())
	scene:SetActiveCamera(cam)

	self.m_renderer = renderer
	self.m_scene = scene
	self.m_camera = cam

	if(util.is_valid(self.m_dbgElTex)) then
		self:SetDebugOverlayEnabled(false)
		self:SetDebugOverlayEnabled(true)
	end
	if(util.is_valid(self.m_debugWireframeCamera)) then
		self:SetDebugWireframeCameraEnabled(false)
		self:SetDebugWireframeCameraEnabled(true)
	end
end
Component.EVENT_ON_RENDER_SCENE_INITIALIZED = ents.register_component_event(ents.COMPONENT_PORTAL,"on_render_scene_initialized")
Component.EVENT_PRE_RENDER_SCENE = ents.register_component_event(ents.COMPONENT_PORTAL,"pre_render_scene")
