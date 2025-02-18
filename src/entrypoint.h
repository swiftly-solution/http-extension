#ifndef _entrypoint_h
#define _entrypoint_h

#include <string>
#include <steam/steam_api_common.h>
#include <steam/steam_gameserver.h>
#include <Embedder.h>

#include <deque>
#include <vector>
#include <any>
#include <functional>

#include <swiftly-ext/core.h>
#include <swiftly-ext/extension.h>
#include <swiftly-ext/hooks/NativeHooks.h>

class HTTPExtension : public SwiftlyExt
{
public:
    bool Load(std::string& error, SourceHook::ISourceHook *SHPtr, ISmmAPI* ismm, bool late);
    bool Unload(std::string& error);
    
    void AllExtensionsLoaded();
    void AllPluginsLoaded();

    bool OnPluginLoad(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error);
    bool OnPluginUnload(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error);

    void Hook_GameServerSteamAPIActivated();
    void Hook_GameServerSteamAPIDeactivated();
    void Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick);
    void Hook_ServerHibernationUpdate(bool bHibernation);

public:
    const char* GetAuthor();
    const char* GetName();
    const char* GetVersion();
    const char* GetWebsite();

    void NextFrame(std::function<void(std::vector<std::any>)> fn, std::vector<std::any> param);

private:
    std::deque<std::pair<std::function<void(std::vector<std::any>)>, std::vector<std::any>>> m_nextFrame;
};

extern HTTPExtension g_Ext;
extern CSteamGameServerAPIContext g_SteamAPI;
extern ISource2Server* server;
DECLARE_GLOBALVARS();

#endif