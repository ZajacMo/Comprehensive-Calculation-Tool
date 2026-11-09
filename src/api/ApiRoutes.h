// REST API 层：把 src/core 的算法能力映射为 JSON 接口。
#pragma once

#include <string>

namespace calc {
namespace http {

class Server;

// 在服务器上注册全部 API 路由。
// 前端静态资源目录为 webRoot，用于服务 / 下的页面与脚本。
void registerApiRoutes(Server& server, const std::string& webRoot);

}  // namespace http
}  // namespace calc
