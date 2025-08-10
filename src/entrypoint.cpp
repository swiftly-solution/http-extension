#include "entrypoint.h"
#include "Manager.h"
#include "ServerManager.h"
#include "PluginHTTP.h"

//////////////////////////////////////////////////////////////
/////////////////        Core Variables        //////////////
////////////////////////////////////////////////////////////

CREATE_GLOBALVARS();

SH_DECL_HOOK0_void(IServerGameDLL, GameServerSteamAPIActivated, SH_NOATTRIB, 0);
SH_DECL_HOOK0_void(IServerGameDLL, GameServerSteamAPIDeactivated, SH_NOATTRIB, 0);
SH_DECL_HOOK1_void(IServerGameDLL, PreWorldUpdate, SH_NOATTRIB, 0, bool);

HTTPExtension g_Ext;

ISource2Server* server = nullptr;

CSteamGameServerAPIContext g_SteamAPI;
ISteamHTTP* g_http = nullptr;
HTTPManager* g_httpManager = nullptr;
HTTPServerManager* g_httpServerManager = nullptr;

//////////////////////////////////////////////////////////////
/////////////////          Core Class          //////////////
////////////////////////////////////////////////////////////

EXT_EXPOSE(g_Ext);
bool HTTPExtension::Load(std::string& error, SourceHook::ISourceHook* SHPtr, ISmmAPI* ismm, bool late)
{
    SAVE_GLOBALVARS();

    GET_IFACE_ANY(GetServerFactory, server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);

    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, PreWorldUpdate, server, this, &HTTPExtension::Hook_PreWorldUpdate, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIActivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIActivated, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIDeactivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIDeactivated, false);

    g_httpManager = new HTTPManager();
    g_httpServerManager = new HTTPServerManager();

    if (late)
    {
        g_SteamAPI.Init();
        g_http = g_SteamAPI.SteamHTTP();
        g_httpManager->ProcessPendingHTTPRequests();
    }

    return true;
}

void HTTPExtension::Hook_GameServerSteamAPIActivated()
{
    if (!CommandLine()->HasParm("-dedicated") || g_SteamAPI.SteamUGC())
        return;

    g_SteamAPI.Init();
    g_http = g_SteamAPI.SteamHTTP();
    g_httpManager->ProcessPendingHTTPRequests();

    RETURN_META(MRES_IGNORED);
}

void HTTPExtension::Hook_GameServerSteamAPIDeactivated()
{
    g_http = nullptr;

    RETURN_META(MRES_IGNORED);
}

void HTTPExtension::Hook_PreWorldUpdate(bool simulating)
{
    while (!m_nextFrame.empty())
    {
        auto pair = m_nextFrame.front();
        pair.first(pair.second);
        m_nextFrame.pop_front();
    }
}

void HTTPExtension::NextFrame(std::function<void(std::vector<std::any>)> fn, std::vector<std::any> param)
{
    m_nextFrame.push_back({ fn, param });
}

bool HTTPExtension::Unload(std::string& error)
{
    delete g_httpServerManager;
    delete g_httpManager;

    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, PreWorldUpdate, server, this, &HTTPExtension::Hook_PreWorldUpdate, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIActivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIActivated, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIDeactivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIDeactivated, false);

    return true;
}

void HTTPExtension::AllExtensionsLoaded()
{

}

void HTTPExtension::AllPluginsLoaded()
{

}

bool HTTPExtension::OnPluginLoad(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error)
{
    EContext* ctx = (EContext*)pluginState;
    typedef std::map<std::string, std::string> StringMap;
    typedef std::map<std::string, StringMap> MapStringMap;

    ADD_CLASS("HTTP");

    ADD_CLASS_FUNCTION("HTTP", "~HTTP", [](FunctionContext* context, ClassData* data) -> void {
        delete data->GetData<PluginHTTP*>("phttp");
        });

    ADD_CLASS_FUNCTION("HTTP", "PerformHTTP", [](FunctionContext* context, ClassData* data) -> void {
        std::string receivedData = context->GetArgumentOr<std::string>(0, "{}");
        context->SetReturn(data->GetData<PluginHTTP*>("phttp")->PerformHTTP(receivedData));
        });

    ADD_CLASS_FUNCTION("HTTP", "Listen", [](FunctionContext* context, ClassData* data) -> void {
        std::string ip = context->GetArgumentOr<std::string>(0, "0.0.0.0");
        int port = context->GetArgumentOr<int>(1, 3000);
        std::string callback_id = context->GetArgument<std::string>(2);

        data->GetData<PluginHTTP*>("phttp")->Listen(ip, port, callback_id);
        });

    ADD_CLASS("HTTPRequest");

    ADD_CLASS_MEMBER_READONLY("HTTPRequest", "path", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPRequest*>("preq")->m_path);
        });

    ADD_CLASS_MEMBER_READONLY("HTTPRequest", "method", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPRequest*>("preq")->m_method);
        });

    ADD_CLASS_MEMBER_READONLY("HTTPRequest", "body", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPRequest*>("preq")->m_body);
        });

    ADD_CLASS_MEMBER_READONLY("HTTPRequest", "files", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPRequest*>("preq")->m_files);
        });

    ADD_CLASS_MEMBER_READONLY("HTTPRequest", "headers", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPRequest*>("preq")->m_headers);
        });

    ADD_CLASS_MEMBER_READONLY("HTTPRequest", "params", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPRequest*>("preq")->m_params);
        });

    ADD_CLASS("HTTPResponse");

    ADD_CLASS_FUNCTION("HTTPResponse", "WriteBody", [](FunctionContext* context, ClassData* data) -> void {
        std::string body = context->GetArgumentOr<std::string>(0, "");
        data->GetData<PluginHTTPResponse*>("pres")->WriteBody(body);
        });

    ADD_CLASS_FUNCTION("HTTPResponse", "GetHeaders", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPResponse*>("pres")->GetHeaders());
        });

    ADD_CLASS_FUNCTION("HTTPResponse", "GetHeader", [](FunctionContext* context, ClassData* data) -> void {
        std::string key = context->GetArgumentOr<std::string>(0, "");
        context->SetReturn(data->GetData<PluginHTTPResponse*>("pres")->GetHeader(key));
        });

    ADD_CLASS_FUNCTION("HTTPResponse", "SetHeader", [](FunctionContext* context, ClassData* data) -> void {
        std::string key = context->GetArgumentOr<std::string>(0, "");
        std::string val = context->GetArgumentOr<std::string>(1, "");
        data->GetData<PluginHTTPResponse*>("pres")->SetHeader(key, val);
        });

    ADD_CLASS_FUNCTION("HTTPResponse", "Send", [](FunctionContext* context, ClassData* data) -> void {
        int rescode = context->GetArgumentOr<int>(0, 200);
        data->GetData<PluginHTTPResponse*>("pres")->Send(rescode);
        });

    ADD_CLASS_FUNCTION("HTTPResponse", "IsCompleted", [](FunctionContext* context, ClassData* data) -> void {
        context->SetReturn(data->GetData<PluginHTTPResponse*>("pres")->IsCompleted());
        });

    ADD_FUNCTION("CreateHTTPInstance", [](FunctionContext* context) -> void {
        context->SetReturn(MAKE_CLASS_INSTANCE("HTTP", { { "phttp", new PluginHTTP(context->GetArgument<std::string>(0)) } }));
        });

    ADD_VARIABLE("_G", "ihttp", MAKE_CLASS_INSTANCE_CTX(ctx, "HTTP", { { "phttp", new PluginHTTP(pluginName) } }));

    return true;
}

bool HTTPExtension::OnPluginUnload(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error)
{
    return true;
}

const char* HTTPExtension::GetAuthor()
{
    return "Swiftly Development Team";
}

const char* HTTPExtension::GetName()
{
    return "HTTP Extension";
}

const char* HTTPExtension::GetVersion()
{
#ifndef VERSION
    return "Local";
#else
    return VERSION;
#endif
}

const char* HTTPExtension::GetWebsite()
{
    return "https://swiftlys2.net/";
}
