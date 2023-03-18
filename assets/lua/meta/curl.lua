--- @meta
--- 
--- @class curl
curl = {}


--- 
--- @class curl.Instance
curl.Instance = {}

--- 
--- @param url string
function curl.Instance:AddRequest(url) end

--- 
function curl.Instance:StartDownload() end

--- 
--- @return bool ret0
--- @overload fun(): bool
function curl.Instance:CancelDownload() end


