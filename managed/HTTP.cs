using SwiftlyS2.Internal_API;
using SwiftlyS2.API.Scripting;
using System.Text.Json;

namespace SwiftlyS2.API.Extensions
{
    public class HTTP
    {
        private static IntPtr _ctx = IntPtr.Zero;
        private static Dictionary<string, Action<int, string, string[], string>> _clientcallbacks = [];
        private static Dictionary<string, Action<HTTPRequest, HTTPResponse>> _servercallbacks = [];

        private static void InitializeContext()
        {
            if (_ctx != IntPtr.Zero) return;
            _ctx = Invoker.CallNative<IntPtr>("_G", "CreateHTTPInstance", CallKind.Function, Generic.GetCurrentPluginName());

            Scripting.Events.AddEventHandler("OnHTTPActionPerformed", (Scripting.Events.Event @e, int status, string body, string headers, string err, string requestID) =>
            {
                if (_clientcallbacks.TryGetValue(requestID, out Action<int, string, string[], string>? value))
                {
                    ulong id = Generic.RegisterCallstack(Generic.GetCurrentPluginName(), "HTTPClientCallback");
                    string[] parsedHeaders = JsonSerializer.Deserialize<string[]>(headers) ?? [];
                    value(status, body, parsedHeaders, err);
                    _clientcallbacks.Remove(requestID);
                    Generic.UnregisterCallstack(Generic.GetCurrentPluginName(), id);
                }
            });

            Scripting.Events.AddEventHandler("OnHTTPServerActionPerformed", (Scripting.Events.Event @e, string requestID, HTTPRequest req, HTTPResponse res) =>
            {
                if (_servercallbacks.TryGetValue(requestID, out Action<HTTPRequest, HTTPResponse>? value))
                {
                    ulong id = Generic.RegisterCallstack(Generic.GetCurrentPluginName(), "HTTPServerCallback");
                    value(req, res);
                    _servercallbacks.Remove(requestID);
                    Generic.UnregisterCallstack(Generic.GetCurrentPluginName(), id);
                }
            });
        }

        public static void PerformHTTPRequest(string url, Action<int, string, string[], string> callback, string? method, string? data, Dictionary<string, string>? headers, Dictionary<string, string>? files)
        {
            InitializeContext();

            Dictionary<string, object> sendData = new()
            {
                { "url", url },
                { "method", method ?? "GET" },
                { "data", data ?? "" },
                { "headers", headers ?? [] },
                { "files", files ?? [] },
            };

            string outreq = JsonSerializer.Serialize(sendData);
            string reqid = Invoker.CallNative<string>("HTTP", "PerformHTTP", CallKind.ClassFunction, _ctx, outreq) ?? "00000000-0000-0000-0000-000000000000";

            if(reqid == "00000000-0000-0000-0000-000000000000")
            {
                callback(0, "", [], "Failed to create HTTP Request");
            }
            else
            {
                _clientcallbacks.Add(reqid, callback);
            }
        }

        public static void Listen(string ip, int port, Action<HTTPRequest, HTTPResponse> callback)
        {
            InitializeContext();

            var cbuuid = Guid.NewGuid().ToString();
            _servercallbacks.Add(cbuuid, callback);

            Invoker.CallNative("HTTP", "Listen", CallKind.ClassFunction, _ctx, ip, port, cbuuid);
        }
    }

    public class HTTPRequest: ClassData
    {
        HTTPRequest() {}

        public string path
        {
            get { return Invoker.CallNative<string>("HTTPRequest", "path", CallKind.ClassMember, m_classData) ?? ""; }
        }
        public string method
        {
            get { return Invoker.CallNative<string>("HTTPRequest", "method", CallKind.ClassMember, m_classData) ?? ""; }
        }
        public string body
        {
            get { return Invoker.CallNative<string>("HTTPRequest", "body", CallKind.ClassMember, m_classData) ?? ""; }
        }
        public Dictionary<string, Dictionary<string, string>> files
        {
            get { return Invoker.CallNative<Dictionary<string, Dictionary<string, string>>>("HTTPRequest", "files", CallKind.ClassMember, m_classData) ?? []; }
        }
        public Dictionary<string, string> headers
        {
            get { return Invoker.CallNative<Dictionary<string, string>>("HTTPRequest", "headers", CallKind.ClassMember, m_classData) ?? []; }
        }
        public Dictionary<string, string> parameters
        {
            get { return Invoker.CallNative<Dictionary<string, string>>("HTTPRequest", "params", CallKind.ClassMember, m_classData) ?? []; }
        }
    }

    public class HTTPResponse : ClassData
    {
        HTTPResponse() { }

        public void WriteBody(string body)
        {
            Invoker.CallNative("HTTPResponse", "WriteBody", CallKind.ClassFunction, this.m_classData, body);
        }
        public Dictionary<string, string> GetHeaders()
        {
            return Invoker.CallNative<Dictionary<string, string>>("HTTPResponse", "GetHeaders", CallKind.ClassFunction, this.m_classData) ?? [];
        }
        public string GetHeader(string headername)
        {
            return Invoker.CallNative<string>("HTTPResponse", "GetHeader", CallKind.ClassFunction, this.m_classData, headername) ?? "";
        }
        public void SetHeader(string headername, string value)
        {
            Invoker.CallNative("HTTPResponse", "SetHeader", CallKind.ClassFunction, this.m_classData, headername, value);
        }
        public void Send(int responsecode)
        {
            Invoker.CallNative("HTTPResponse", "Send", CallKind.ClassFunction, this.m_classData, responsecode);
        }
        public bool IsCompleted()
        {
            return Invoker.CallNative<bool>("HTTPResponse", "IsCompleted", CallKind.ClassFunction, this.m_classData);
        }
    }
}
