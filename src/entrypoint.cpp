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
SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK1_void(IServerGameDLL, ServerHibernationUpdate, SH_NOATTRIB, 0, bool);

HTTPExtension g_Ext;
CUtlVector<FuncHookBase *> g_vecHooks;

ISource2Server* server = nullptr;

CSteamGameServerAPIContext g_SteamAPI;
ISteamHTTP* g_http = nullptr;
HTTPManager* g_httpManager = nullptr;
HTTPServerManager* g_httpServerManager = nullptr;

//////////////////////////////////////////////////////////////
/////////////////          Core Class          //////////////
////////////////////////////////////////////////////////////

EXT_EXPOSE(g_Ext);
bool HTTPExtension::Load(std::string& error, SourceHook::ISourceHook *SHPtr, ISmmAPI* ismm, bool late)
{
    SAVE_GLOBALVARS();
    if(!InitializeHooks()) {
        error = "Failed to initialize hooks.";
        return false;
    }

    GET_IFACE_ANY(GetServerFactory, server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);

    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &HTTPExtension::Hook_GameFrame, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIActivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIActivated, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIDeactivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIDeactivated, false);
    SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerHibernationUpdate, server, this, &HTTPExtension::Hook_ServerHibernationUpdate, true);

    g_httpManager = new HTTPManager();
    g_httpServerManager = new HTTPServerManager();

    if(late)
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

void HTTPExtension::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
    while (!m_nextFrame.empty())
    {
        auto pair = m_nextFrame.front();
        pair.first(pair.second);
        m_nextFrame.pop_front();
    }
}

bool isServerHibernating = true;

void HTTPExtension::Hook_ServerHibernationUpdate(bool bHibernation)
{
    isServerHibernating = bHibernation;
}

void HTTPExtension::NextFrame(std::function<void(std::vector<std::any>)> fn, std::vector<std::any> param)
{
    if (isServerHibernating)
        fn(param);
    else
        m_nextFrame.push_back({ fn, param });
}

bool HTTPExtension::Unload(std::string& error)
{
    delete g_httpServerManager;
    delete g_httpManager;

    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &HTTPExtension::Hook_GameFrame, true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIActivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIActivated, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameServerSteamAPIDeactivated, server, this, &HTTPExtension::Hook_GameServerSteamAPIDeactivated, false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerHibernationUpdate, server, this, &HTTPExtension::Hook_ServerHibernationUpdate, true);

    UnloadHooks();
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
    EContext* state = (EContext*)pluginState;

    BeginClass<PluginHTTP>("HTTP", state)
        .addConstructor<std::string>()
        .addFunction("PerformHTTP", &PluginHTTP::PerformHTTP)
        .addFunction("Listen", &PluginHTTP::Listen)
    .endClass();
        
    BeginClass<PluginHTTPRequest>("HTTPRequest", state)
        .addProperty("path", &PluginHTTPRequest::m_path, false)
        .addProperty("method", &PluginHTTPRequest::m_method, false)
        .addProperty("body", &PluginHTTPRequest::m_body, false)
        .addProperty("files", &PluginHTTPRequest::m_files, false)
        .addProperty("headers", &PluginHTTPRequest::m_headers, false)
        .addProperty("params", &PluginHTTPRequest::m_params, false)
    .endClass();
        
    BeginClass<PluginHTTPResponse>("HTTPResponse", state)
        .addFunction("WriteBody", &PluginHTTPResponse::WriteBody)
        .addFunction("GetHeaders", &PluginHTTPResponse::GetHeaders)
        .addFunction("GetHeader", &PluginHTTPResponse::GetHeader)
        .addFunction("SetHeader", &PluginHTTPResponse::SetHeader)
        .addFunction("Send", &PluginHTTPResponse::Send)
        .addFunction("IsCompleted", &PluginHTTPResponse::IsCompleted)
    .endClass();

    GetGlobalNamespace(state).addConstant("http", PluginHTTP(pluginName));

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
    return "https://swiftlycs2.net/";
}
