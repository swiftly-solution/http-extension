function SetupHTTP(global) {
    global.httpRequestsQueue = {}
    global.httpServerCallbacks = {}

    global.PerformHTTPRequest = (url, callback, method, data, headers, files) => {
        let sendData = {
            url: url,
            method: method || 'GET',
            data: data || '',
            headers: headers || {},
            files: files || {}
        }

        const httpRequestID = ihttp.PerformHTTP(JSON.stringify(sendData))

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
        } catch (err) {
            console.log("An error has been occured while trying to execute a HTTP Callback.\nError: " + err)
        }

        delete global.httpRequestsQueue[httpRequestID];
        UnregisterCallstack(GetCurrentPluginName(), stackid)

        return EventResult.Stop
    })

    AddEventHandler("OnHTTPServerActionPerformed", (event, callback_id, req, res) => {
        if (!global.httpServerCallbacks[callback_id]) return EventResult.Continue

        try {
            httpServerCallbacks[callback_id](req, res)
        } catch (err) {
            console.log("An error has been occured while trying to execute a HTTP Server Callback.\nError: " + err)
        }
        return EventResult.Stop
    })

    global.http = {
        Listen: (ip, port, callback) => {
            if (typeof callback != "function") return;

            let callback_uuid = uuid()
            httpServerCallbacks[callback_uuid] = callback

            ihttp.Listen(ip, port, callback_uuid)
        }
    }
}

SetupHTTP(globalThis)