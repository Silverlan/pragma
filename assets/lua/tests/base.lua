tests = tests or {}

local LOGGER = log.register_logger("tests")

util.register_class("tests.TestManager", util.CallbackHandler)
function tests.TestManager:__init()
	util.CallbackHandler.__init(self)
	self.m_testQueue = {}
	self.m_scripts = {}
	self.m_resultData = {}
	self.m_resultAssetPaths = {}
	file.delete_directory("temp/tests")
	file.create_path("temp/tests")
end

function tests.TestManager:__finalize()
	self:Clear()
end

function tests.TestManager:AddTest(name)
	local scriptData = self.m_scripts[name]
	if scriptData == nil then
		return
	end
	if scriptData.scriptFile ~= nil then
		table.insert(self.m_testQueue, name)
	end
	if scriptData.children ~= nil then
		for _, child in ipairs(scriptData.children) do
			self:AddTest(child)
		end
	end
end

function tests.TestManager:Clear()
	util.remove(self.m_cbThink)
end

function tests.TestManager:IsTestActive()
	return self:GetActiveTest() ~= nil
end

function tests.TestManager:GetActiveTest()
	return self.m_currentTest
end

function tests.TestManager:CompleteTest(success, resultData)
	resultData = resultData or {}
	if resultData.screenshot then
		LOGGER:Info("Taking screenshot...")
		resultData["assets"] = resultData["assets"] or {}
		-- Wait a few frames before taking the screenshot
		game.wait_for_frames(5, function()
			local screenshotPath = util.screenshot()
			if screenshotPath ~= nil then
				table.insert(resultData["assets"], screenshotPath)
			else
				LOGGER:Warn("Failed to take screenshot!")
			end
			resultData.screenshot = false
			tests.manager:CompleteTest(success, resultData)
		end)
		return
	end

	if type(resultData) == "string" then
		resultData = {
			message = resultData,
		}
	else
		resultData = resultData or {}
	end
	local curTest = self.m_currentTest
	self.m_currentTest = nil
	if success == false then
		local msg = resultData.message or "Unknown"
		msg = "Test '" .. self.m_currentTest .. "' failed: " .. tostring(msg)
		LOGGER:Err(msg)
		error(msg)
	end

	LOGGER:Info("Test '{}' has completed!", curTest)
	self:CallCallbacks("OnTestComplete", success, resultData)

	self.m_resultData[curTest] = resultData

	if resultData.assets ~= nil then
		LOGGER:Info("Copying {} tests result assets...", #resultData.assets)
		for _, fileName in ipairs(resultData.assets) do
			local path = "temp/tests/" .. curTest
			file.create_path(path)
			local dstFileName = path .. "/" .. file.get_file_name(fileName)
			file.copy("log.txt", path .. "/log.txt")
			local success = file.move(fileName, dstFileName)
			if success == false then
				LOGGER:Warn("Failed to copy test asset file '{}' to '{}'!", fileName, dstFileName)
			else
				table.insert(self.m_resultAssetPaths, dstFileName)
			end
		end
	end

	if #self.m_testQueue == 0 then
		LOGGER:Info("All tests have been completed!")
		self:CallCallbacks("OnAllTestsComplete")
	end
end

function tests.TestManager:GetResultData()
	return self.m_resultData
end

function tests.TestManager:GetResultAssetPaths()
	return self.m_resultAssetPaths
end

function tests.TestManager:StartTest(test)
	LOGGER:Info("Starting test '{}'...", test)
	self.m_currentTest = test
	self:CallCallbacks("OnTestStart", test)

	local scriptData = self.m_scripts[test]
	assert(scriptData ~= nil)
	local retVals = { include(scriptData.scriptFile, true) }
	if retVals[1] ~= nil then
		self:CompleteTest(retVals[1], retVals[2])
	end
end

function tests.TestManager:Start()
	if util.is_valid(self.m_cbThink) then
		return
	end
	self.m_cbThink = game.add_callback("Think", function()
		if self:IsTestActive() == false then
			self:RunNextTest()
		end
	end)
end

function tests.TestManager:RunNextTest()
	if #self.m_testQueue == 0 then
		-- self:Clear()
		return
	end
	local test = self.m_testQueue[1]
	table.remove(self.m_testQueue, 1)
	self:StartTest(test)
end

function tests.TestManager:LoadTests(scriptFileName)
	local udmData, err = udm.load("scripts/" .. scriptFileName)
	if udmData == false then
		LOGGER:Err("Failed to load tests file '{}': {}", scriptFileName, err)
		return
	end
	LOGGER:Info("Loading tests file '{}'...", scriptFileName)
	local assetData = udmData:GetAssetData():GetData()
	for name, child in pairs(assetData:GetChildren()) do
		local scriptData = {}
		scriptData.scriptFile = child:GetValue("scriptFile", udm.TYPE_STRING)
		scriptData.children = child:GetArrayValues("children", udm.TYPE_STRING)
		self.m_scripts[name] = scriptData
	end
end

tests.manager = tests.TestManager()
--tests.manager = tests.manager or tests.TestManager()
tests.add_event_listener = function(...)
	return tests.manager:AddCallback(...)
end
tests.is_test_running = function()
	return tests.manager:IsTestActive()
end
tests.queue = function(name)
	tests.manager:AddTest(name)
end
tests.run = function()
	tests.manager:Start()
end
tests.complete = function(success, resultData)
	tests.manager:CompleteTest(success, resultData)
end
tests.load = function(fileName)
	tests.manager:LoadTests(fileName)
end
tests.log_debug = function(...)
	LOGGER:Debug(...)
end
tests.log_info = function(...)
	LOGGER:Info(...)
end
tests.log_err = function(...)
	LOGGER:Err(...)
end
tests.log_warn = function(...)
	LOGGER:Warn(...)
end
