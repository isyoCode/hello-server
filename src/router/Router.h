#ifndef yoyo_ROUTER_H_
#define yoyo_ROUTER_H_

#include <string>
#include <map>
#include <functional>
#include <vector>

#include "../http/request.h"
#include "../http/response.h"

namespace yoyo {

namespace router {


using HttpHandler = std::function<void(const http::HttpRequest&, http::HttpResponse&)>;

class Router {
public:
    Router();

    void addRouter(const std::string& sMethod, const std::string& sPath, HttpHandler handler);

    bool route(const http::HttpRequest& req, http::HttpResponse& res) const;

private:
    // 路由表 method -> path -> handler
    std::map<std::string, std::map<std::string, HttpHandler>> routes_;


};

} // end of namespace router

} // end of namespace yoyo

#endif