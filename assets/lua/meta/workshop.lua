--- @meta
--- 
--- @class steamworks.workshop
steamworks.workshop = {}

--- 
--- @param fileId int
--- @param oCallback any
--- @param language int
--- @overload fun(fileId: int, oCallback: any): 
function steamworks.workshop.get_item(fileId, oCallback, language) end

--- 
--- @param oCallback any
function steamworks.workshop.create_item(oCallback) end

--- 
--- @param itemId int
--- @param itemIdOther int
--- @param callback any
function steamworks.workshop.add_item_dependency(itemId, itemIdOther, callback) end

--- 
--- @param itemId int
--- @param appId int
--- @param callback any
function steamworks.workshop.add_item_app_dependency(itemId, appId, callback) end

--- 
--- @param itemId int
--- @param itemIdOther int
--- @param callback any
function steamworks.workshop.remove_item_dependency(itemId, itemIdOther, callback) end

--- 
--- @param itemId int
--- @param appId int
--- @param callback any
function steamworks.workshop.remove_item_app_dependency(itemId, appId, callback) end

--- 
--- @param itemId int
--- @param callback any
function steamworks.workshop.subscribe_item(itemId, callback) end

--- 
--- @param itemId int
--- @param callback any
function steamworks.workshop.unsubscribe_item(itemId, callback) end

--- 
function steamworks.workshop.get_subscribed_items() end

--- 
--- @param callbackId int
--- @param oCallback any
function steamworks.workshop.register_callback(callbackId, oCallback) end


--- 
--- @class steamworks.workshop.Item
steamworks.workshop.Item = {}

--- 
function steamworks.workshop.Item:GetUpVoteCount() end

--- 
--- @param imagePath string
function steamworks.workshop.Item:SetPreviewImage(imagePath) end

--- 
function steamworks.workshop.Item:GetDownVoteCount() end

--- 
function steamworks.workshop.Item:GetTags() end

--- 
function steamworks.workshop.Item:GetCategoryFlags() end

--- 
--- @param desc string
function steamworks.workshop.Item:SetDescription(desc) end

--- 
--- @param visibility int
function steamworks.workshop.Item:SetVisibility(visibility) end

--- 
--- @param categoryFlags int
function steamworks.workshop.Item:SetCategoryFlags(categoryFlags) end

--- 
function steamworks.workshop.Item:GetChildCount() end

--- 
function steamworks.workshop.Item:GetTimeAddedToUserList() end

--- 
--- @param tTags any
function steamworks.workshop.Item:SetTags(tTags) end

--- 
--- @param addonDirectory string
function steamworks.workshop.Item:SetAddonContent(addonDirectory) end

--- 
--- @param language int
function steamworks.workshop.Item:SetUpdateLanguage(language) end

--- 
function steamworks.workshop.Item:GetURL() end

--- 
--- @param callback any
--- @param changeLog string
--- @overload fun(callback: any): 
function steamworks.workshop.Item:PublishChanges(callback, changeLog) end

--- 
function steamworks.workshop.Item:GetStateFlags() end

--- 
--- @overload fun(highPriority: bool): 
function steamworks.workshop.Item:Download() end

--- 
function steamworks.workshop.Item:GetDownloadInfo() end

--- 
function steamworks.workshop.Item:GetScore() end

--- 
function steamworks.workshop.Item:GetTimeCreated() end

--- 
--- @param title string
function steamworks.workshop.Item:SetTitle(title) end

--- 
function steamworks.workshop.Item:GetTitle() end

--- 
function steamworks.workshop.Item:GetSteamIdOwner() end

--- 
function steamworks.workshop.Item:GetDescription() end

--- 
function steamworks.workshop.Item:GetInstallInfo() end

--- 
function steamworks.workshop.Item:IsValid() end

--- 
function steamworks.workshop.Item:GetFileSize() end

--- 
function steamworks.workshop.Item:GetId() end

--- 
function steamworks.workshop.Item:IsBanned() end

--- 
function steamworks.workshop.Item:AreTagsTruncated() end

--- 
function steamworks.workshop.Item:GetVisibility() end

--- 
function steamworks.workshop.Item:GetProgress() end

--- 
function steamworks.workshop.Item:GetPreviewFileSize() end

--- 
function steamworks.workshop.Item:GetType() end

--- 
function steamworks.workshop.Item:GetTimeUpdated() end


--- @enum Type
steamworks.workshop.Item = {
	TYPE_COMMUNITY = 0,
	TYPE_ART = 3,
	TYPE_STEAM_VIDEO = 14,
	TYPE_CONCEPT = 8,
	TYPE_MICROTRANSACTION = 1,
	TYPE_COLLECTION = 2,
	TYPE_GAME = 6,
	TYPE_CONTROLLER_BINDING = 12,
	TYPE_INTEGRATED_GUIDE = 10,
	TYPE_GAME_MANAGED_ITEM = 15,
	TYPE_MERCH = 11,
	TYPE_SCREENSHOT = 5,
	TYPE_SOFTWARE = 7,
	TYPE_STEAMWORKS_ACCESS_INVITE = 13,
	TYPE_VIDEO = 4,
	TYPE_WEB_GUIDE = 9,
}

--- @enum StateFlag
steamworks.workshop.Item = {
	STATE_FLAG_SUBSCRIBED_BIT = 1,
	STATE_FLAG_DOWNLOAD_PENDING_BIT = 32,
	STATE_FLAG_DOWNLOADING_BIT = 16,
	STATE_FLAG_NONE = 0,
	STATE_FLAG_INSTALLED_BIT = 4,
	STATE_FLAG_LEGACY_ITEM_BIT = 2,
	STATE_FLAG_NEEDS_UPDATE_BIT = 8,
}

