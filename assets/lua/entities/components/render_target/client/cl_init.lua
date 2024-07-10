--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("../shared.lua")

local Component = ents.RenderTargetComponent

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:SetResolution(1024, 1024)

	self.m_drawSceneInfo = game.DrawSceneInfo()
	self.m_drawSceneInfo.toneMapping = shader.TONE_MAPPING_NONE
end

function Component:OnRemove()
	util.remove({ self.m_dbgElTex, self.m_cbRenderScenes })
	self:ClearScene()
end

function Component:SetResolution(w, h)
	self.m_resolution = Vector2i(w, h)
end
function Component:GetResolution()
	return self.m_resolution
end
function Component:GetRenderer()
	return self.m_renderer
end
function Component:GetScene()
	return self.m_scene
end
function Component:GetCamera()
	return self.m_camera
end
function Component:GetDrawSceneInfo()
	return self.m_drawSceneInfo
end

function Component:SetCamera(cam)
	self.m_targetCamera = cam
end

function Component:SetDebugWireframeCameraEnabled(enabled)
	if util.is_valid(self.m_camera) == false then
		return
	end
	if enabled then
		self.m_debugWireframeCamera = self.m_camera:GetEntity():AddComponent("wireframe_camera")
	else
		self.m_camera:GetEntity():RemoveComponent("wireframe_camera")
	end
end

function Component:SetDebugOverlayEnabled(enabled)
	if enabled then
		self:InitializeDebugOverlay()
	else
		util.remove(self.m_dbgElTex)
	end
end

function Component:OnEntitySpawn()
	self:InitializeScene()
end

function Component:InitializeScene()
	self:CreateScene(self.m_resolution.x, self.m_resolution.y)

	if util.is_valid(self.m_cbRenderScenes) == false then
		self.m_cbRenderScenes = game.add_callback("RenderScenes", function(drawSceneInfo)
			self:InvokeEventCallbacks(Component.EVENT_PRE_RENDER_SCENE, { self.m_drawSceneInfo })
			game.queue_scene_for_rendering(self.m_drawSceneInfo)
		end)
	end

	self:BroadcastEvent(Component.EVENT_ON_RENDER_SCENE_INITIALIZED)
end

function Component:InitializeDebugOverlay()
	if util.is_valid(self.m_dbgElTex) then
		return
	end
	local elTex = gui.create("WITexturedRect")
	elTex:SetSize(512, 512)
	elTex:SetTexture(self.m_renderer:GetPresentationTexture())
	self.m_dbgElTex = elTex
end

function Component:ClearScene()
	for _, c in ipairs({ self.m_renderer, self.m_scene }) do
		if c:IsValid() then
			util.remove(c:GetEntity())
		end
	end
	if self.m_customCamera == true and util.is_valid(self.m_camera) then
		util.remove(self.m_camera:GetEntity())
	end
end

function Component:CreateScene(w, h)
	self:ClearScene()
	local sceneCreateInfo = ents.SceneComponent.CreateInfo()
	sceneCreateInfo.sampleCount = prosper.SAMPLE_COUNT_1_BIT
	local gameScene = game.get_scene()
	local scene = ents.create_scene(sceneCreateInfo, gameScene)
	scene:Link(gameScene)
	scene:GetEntity():SetName("scene_render_target")

	local entRenderer = self:GetEntity():CreateChild("rasterization_renderer")
	local renderer = entRenderer:GetComponent(ents.COMPONENT_RENDERER)
	local rasterizer = entRenderer:GetComponent(ents.COMPONENT_RASTERIZATION_RENDERER)
	rasterizer:SetSSAOEnabled(true)
	renderer:InitializeRenderTarget(scene, w, h)

	self.m_drawSceneInfo.scene = scene
	self.m_drawSceneInfo.renderFlags = bit.band(self.m_drawSceneInfo.renderFlags, bit.bnot(game.RENDER_FLAG_BIT_VIEW)) -- Don't render view models
	self.m_drawSceneInfo.outputImage = renderer:GetPresentationTexture():GetImage()

	scene:SetRenderer(renderer)

	local gameScene = game.get_scene()
	local gameCam = gameScene:GetActiveCamera()
	local cam
	if util.is_valid(self.m_targetCamera) then
		cam = self.m_targetCamera
		self.m_customCamera = false
	else
		cam = ents.create_camera(gameCam:GetAspectRatio(), gameCam:GetFOV(), gameCam:GetNearZ(), gameCam:GetFarZ())
		self.m_customCamera = true
	end
	scene:SetActiveCamera(cam)

	self.m_renderer = renderer
	self.m_scene = scene
	self.m_camera = cam

	if util.is_valid(self.m_dbgElTex) then
		self:SetDebugOverlayEnabled(false)
		self:SetDebugOverlayEnabled(true)
	end
	if util.is_valid(self.m_debugWireframeCamera) then
		self:SetDebugWireframeCameraEnabled(false)
		self:SetDebugWireframeCameraEnabled(true)
	end
end
Component.EVENT_ON_RENDER_SCENE_INITIALIZED =
	ents.register_component_event(ents.COMPONENT_RENDER_TARGET, "on_render_scene_initialized")
Component.EVENT_PRE_RENDER_SCENE = ents.register_component_event(ents.COMPONENT_RENDER_TARGET, "pre_render_scene")
