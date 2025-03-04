function SetupHTTP(global) {
    global.httpRequestsQueue = {}
    
    global.PerformHTTPRequest = (url, callback, method, data, headers, files) => {
        let sendData = {
            url: url,
            method: method || 'GET',
            data: data || '',
            headers: headers || {},
            files: files || {}
        }
    
        const httpRequestID = http.PerformHTTP(JSON.stringify(sendData))
    
        if (httpRequestID != "00000000-0000-0000-0000-000000000000") {
            global.httpRequestsQueue[httpRequestID] = callback
        } else {
            callback(0, null, [], 'Failed to create HTTP Request')
        }
    }
    
    AddEventHandler("OnHTTPActionPerformed", (event, status, body, headers, err, httpRequestID) => {
        if (!global.httpRequestsQueue[httpRequestID]) return EventResult.Continue
        let stackid = RegisterCallstack(GetCurrentPluginName(), "HTTPCallback")

        headers = JSON.parse(headers)
    
        try {
            global.httpRequestsQueue[httpRequestID](status, body, headers, err)
        } catch(err) {
            console.log("An error has been occured while trying to execute a HTTP Callback.\nError: " + err)
        }
    
        delete global.httpRequestsQueue[httpRequestID];
        UnregisterCallstack(GetCurrentPluginName(), stackid)
    
        return EventResult.Stop
})
}

SetupHTTP(globalThis)