#ifndef _pluginhttp_h
#define _pluginhttp_h

#include <string>
#include <vector>
#include <any>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
#include <LuaBridge/LuaBridge.h>
#include <cpp-httplib/httplib.h>

class HTTPHeader
{
public:
    HTTPHeader(std::string strName, std::string strValue)
    {
        m_strName = strName;
        m_strValue = strValue;
    }
    const char* GetName() { return m_strName.c_str(); }
    const char* GetValue() { return m_strValue.c_str(); }
private:
    std::string m_strName;
    std::string m_strValue;
};

class PluginHTTP
{
private:
    std::string plugin_name;
    std::vector<std::vector<std::any>> toDelete;

public:
    PluginHTTP(std::string m_plugin_name);
    ~PluginHTTP();

    std::string PerformHTTP(std::string receivedData);
    std::string PerformHTTPWithRequestID(std::string receivedData, std::string requestID);

    void ListenLua(std::string ip_addr, uint16_t port, luabridge::LuaRef cb);
};

class PluginHTTPRequest
{
public:
    PluginHTTPRequest(std::string path, std::string method, std::string body, std::map<std::string, std::map<std::string, std::string>> files, std::map<std::string, std::string> headers, std::map<std::string, std::string> params);

    std::string m_path;
    std::string m_method;
    std::string m_body;
    std::map<std::string, std::map<std::string, std::string>> m_files;
    std::map<std::string, std::string> m_headers;
    std::map<std::string, std::string> m_params;
};

class PluginHTTPResponse
{
private:
    httplib::Response* m_res;
    bool completed = false;
    uint64_t started = 0;
public:
    PluginHTTPResponse(httplib::Response* res);

    void WriteBody(std::string content);
    std::map<std::string, std::string> GetHeaders();
    std::string GetHeader(std::string key);
    void SetHeader(std::string key, std::string val);
    void Send(uint16_t responseCode);
    bool IsCompleted();
};

#endif