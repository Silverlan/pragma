--- @meta
--- 
--- @class curl
curl = {}


--- 
--- @class curl.Instance
curl.Instance = {}

--- 
--- @return bool ret0
--- @overload fun(): bool
function curl.Instance:CancelDownload() end

--- 
--- @param url string
function curl.Instance:AddRequest(url) end

--- 
function curl.Instance:StartDownload() end


