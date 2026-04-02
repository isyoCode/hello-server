#include <algorithm>
#include <cctype>

#include "Router.h"

namespace yoyo {
namespace router {

Router::Router() {
  // init
  // TODO 提前添加一些自定义的处理方法
  const static std::string sDefaultPath = "/yoyo/default/test.html";
  const static std::string sDefaultMethod = "GET";

  addRouter(
      sDefaultMethod, sDefaultPath,
      [](const http::HttpRequest& req, http::HttpResponse& res) {
        // 处理默认路由的请求
        res.setVersion("HTTP/1.1");
        res.setStatusCode(200);
        res.setReason("OK");
        res.addHeader("Content-Type", "text/html");
        std::string sResBody =
            "<h1>Welcome to Yoyo Router!</h1><p>This is the default route.</p>";
        res.setBody(
            std::vector<unsigned char>(sResBody.begin(), sResBody.end()));
      });
}

void Router::addRouter(const std::string& sMethod, const std::string& sPath,
                       HttpHandler handler) {
  std::string lowerMethod = sMethod;
  // 统一化method小写
  std::transform(lowerMethod.begin(), lowerMethod.end(), lowerMethod.begin(),
                 ::tolower);
  routes_[lowerMethod][sPath] = std::move(handler);
#ifdef YOYOROUTERDEBUG
  std::cout << "Added route: " << lowerMethod << " " << sPath << std::endl;
#endif
}

bool Router::route(const http::HttpRequest& req,
                   http::HttpResponse& res) const {
  std::string lowerMethod = req.getMethod();
  // 统一化method小写
  std::transform(lowerMethod.begin(), lowerMethod.end(), lowerMethod.begin(),
                 ::tolower);
  auto methodIt = routes_.find(lowerMethod);
  if (methodIt != routes_.end()) {
    const auto& pathMap = methodIt->second;
    auto pathIt = pathMap.find(req.getPath());
    if (pathIt != pathMap.end()) {
      // 找到匹配的路由，调用处理函数
      pathIt->second(req, res);
      return true;
    }
  }

  // 没有找到匹配的路由
  // 未找到路由，返回 404
  res.setVersion("HTTP/1.1");
  res.setStatusCode(404);
  res.setReason("Not Found");
  res.addHeader("Content-Type", "text/html");
  std::string sResBody = "<h1>404 Not Found</h1><p>The requested URL " +
                         req.getUrl() + " was not found on this server.</p>";
  res.setBody(std::vector<unsigned char>(sResBody.begin(), sResBody.end()));
  return false;
}

}  // end of namespace router
}  // end of namespace yoyo