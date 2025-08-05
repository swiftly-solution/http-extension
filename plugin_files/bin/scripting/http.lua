local httpRequestsQueue = {}
local httpServerCallbacks = {}
local json_encode = json.encode
local json_decode = json.decode

function PerformHTTPRequest(url, callback, method, data, headers, files)
    local sendData = {
        url = url,
        method = method or 'GET',
        data = data or '',
        headers = headers or {},
        files = files or {}
    }

    local httpRequestID = ihttp:PerformHTTP(json_encode(sendData))

    if httpRequestID ~= "00000000-0000-0000-0000-000000000000" then
        httpRequestsQueue[httpRequestID] = callback
    else
        callback(0, nil, {}, 'Failed to create HTTP Request')
    end
end

AddEventHandler("OnHTTPActionPerformed", function(event, status, body, headers, err, httpRequestID)
    if not httpRequestsQueue[httpRequestID] then return EventResult.Continue end
    headers = json_decode(headers)

    httpRequestsQueue[httpRequestID](status, body, headers, err)
    httpRequestsQueue[httpRequestID] = nil

    return EventResult.Stop
end)

AddEventHandler("OnHTTPServerActionPerformed", function(event, callback_id, req, res)
    if not httpServerCallbacks[callback_id] then return EventResult.Continue end

    httpServerCallbacks[callback_id](req, res)
    return EventResult.Stop
end)

http = {
    Listen = function(ip, port, callback)
        if not callback then return end
        if type(callback) ~= "function" then return end

        local callback_uuid = uuid()
        httpServerCallbacks[callback_uuid] = callback
        
        ihttp:Listen(ip, port, callback_uuid)
    end
}