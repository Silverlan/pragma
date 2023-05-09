--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("util.ImageRecorder")
function util.ImageRecorder:__init(img)
	self.m_image = img
	self.m_frameIndex = 0
	self.m_frameRate = 60
	self.m_fileName = ""
	self.m_recording = false
	self.m_threadPool = util.ThreadPool(10, "img_recorder")
end
function util.ImageRecorder:__finalize()
	self:StopRecording()
end
function util.ImageRecorder:IsRecording()
	return self.m_recording
end
function util.ImageRecorder:GetFrameDeltaTime()
	return (1.0 / self.m_frameRate)
end
function util.ImageRecorder:SetFrameRate(frameRate)
	self.m_frameRate = frameRate
end
function util.ImageRecorder:SetImage(img)
	self.m_image = img
end
function util.ImageRecorder:RenderNextFrame()
	local img = self.m_image
	local frameRate = self.m_frameRate
	if self:GoToTimeOffset(self.m_frameIndex, self.m_frameIndex * (1.0 / frameRate)) == false then
		self:StopRecording()
		return
	end

	local c = 0
	self.m_cbTick = game.add_callback("Tick", function()
		if c == 0 then
			c = c + 1
			return
		end
		self.m_cbTick:Remove()
		self.m_cbRender = game.add_callback("PostRender", function()
			file.create_path(file.get_file_path(self.m_fileName))
			local fileName = self.m_fileName .. string.fill_zeroes(tostring(self.m_frameIndex + 1), 4) .. ".png"
			self:Log("Saving frame '" .. fileName .. "'...")

			local buf = img:ToImageBuffer(false, false)
			-- buf = buf:ApplyToneMapping(util.ImageBuffer.TONE_MAPPING_ACES)
			self.m_threadPool:WaitForPendingCount(15)
			local r = util.save_image(buf, fileName, util.IMAGE_FORMAT_PNG, 0.0, self.m_threadPool)
			if r ~= true then
				self:Log("Unable to save frame '" .. fileName .. "'!", true)
			end

			self.m_cbRender:Remove()
			self.m_frameIndex = self.m_frameIndex + 1
			self:RenderNextFrame()
		end)
	end)
end
function util.ImageRecorder:StartRecording(fileName)
	if self:IsRecording() then
		self:Log("Unable to start recording: Recording already in progress!", true)
		return false
	end
	self:Log("Starting video recording '" .. fileName .. "'...")
	self.m_recording = true
	self.m_fileName = fileName
	self.m_startTime = time.time_since_epoch()
	self:RenderNextFrame()
	return true
end
function util.ImageRecorder:StopRecording()
	self.m_recording = false
	util.remove(self.m_cbTick)
	util.remove(self.m_cbRender)

	local dt = time.time_since_epoch() - self.m_startTime
	local t = dt / 1000000000.0
	self:Log("Recording complete! Recording took " .. t .. " seconds!")
	self:OnComplete()
end
function util.ImageRecorder:OnComplete() end
function util.ImageRecorder:GoToTimeOffset(frameIndex, t)
	return true
end
function util.ImageRecorder:Log(msg, isWarning)
	if isWarning then
		console.print_warning(msg)
		return
	end
	print(msg)
end
