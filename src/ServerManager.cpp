#include "ServerManager.h"

HTTPServerManager::~HTTPServerManager()
{
    for (auto it = httpServers.begin(); it != httpServers.end(); ++it) {
        it->second->stop();
        delete it->second;
    }
}

typedef void (*HTTPCallback)(const httplib::Request&, httplib::Response&, std::string);

void SetupServer(HTTPServerManager* _this, std::string ip_addr, uint16_t port)
{
    std::pair<std::string, uint16_t> key{ ip_addr, port };

    auto FunctionHTTPCallback = [&](const httplib::Request& req, httplib::Response& res) {
        auto listeners = _this->httpListeners[key];
        for (auto listener : listeners)
            reinterpret_cast<HTTPCallback>(listener.first)(req, res, listener.second);
        };

    _this->httpServers[key]->Options(R"((.*))", FunctionHTTPCallback);
    _this->httpServers[key]->Get(R"((.*))", FunctionHTTPCallback);
    _this->httpServers[key]->Post(R"((.*))", FunctionHTTPCallback);
    _this->httpServers[key]->Put(R"((.*))", FunctionHTTPCallback);
    _this->httpServers[key]->Patch("((.*))", FunctionHTTPCallback);
    _this->httpServers[key]->Delete(R"((.*))", FunctionHTTPCallback);

    _this->httpServers[key]->listen(ip_addr, port);
}

void HTTPServerManager::SetupHTTPServer(std::string ip_addr, uint16_t port)
{
    std::pair<std::string, uint16_t> key{ ip_addr, port };
    if (httpServers.find(key) != httpServers.end()) {
        httpServers[key]->stop();
        delete httpServers[key];
    }

    this->httpServers[key] = new httplib::Server();
    std::thread(SetupServer, this, ip_addr, port).detach();
}

void HTTPServerManager::RegisterHTTPServerListener(std::string ip_addr, uint16_t port, void* callback, std::string callback_id)
{
    std::pair<std::string, uint16_t> key{ ip_addr, port };
    if (httpServers.find(key) == httpServers.end()) SetupHTTPServer(ip_addr, port);

    httpListeners[key].push_back({ callback, callback_id });
}

void HTTPServerManager::UnregisterHTTPServerListener(std::string ip_addr, uint16_t port, std::string callback_id)
{
    std::pair<std::string, uint16_t> key{ ip_addr, port };
    if (httpServers.find(key) == httpServers.end()) SetupHTTPServer(ip_addr, port);

    for (auto it = httpListeners[key].begin(); it != httpListeners[key].end(); ++it)
        if (it->second == callback_id) {
            httpListeners[key].erase(it);
            break;
        }

    if (httpListeners[key].size() == 0) {
        httpServers[key]->stop();
        delete httpServers[key];
        httpServers.erase(key);
    }
}