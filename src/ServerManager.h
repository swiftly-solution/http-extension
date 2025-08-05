#ifndef _http_servermanager_h
#define _http_servermanager_h

#include <map>
#include <vector>
#include <string>
#include <any>
#include <cpp-httplib/httplib.h>

class HTTPServerManager
{
private:
    void SetupHTTPServer(std::string ip_addr, uint16_t port);
public:
    std::map<std::pair<std::string, uint16_t>, std::vector<std::pair<void*, std::string>>> httpListeners;
    std::map<std::pair<std::string, uint16_t>, httplib::Server*> httpServers;

    ~HTTPServerManager();

    void RegisterHTTPServerListener(std::string ip_addr, uint16_t port, void* callback, std::string callback_id);
    void UnregisterHTTPServerListener(std::string ip_addr, uint16_t port, std::string callback);
};
extern HTTPServerManager* g_httpServerManager;

#endif