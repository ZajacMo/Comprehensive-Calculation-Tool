// Web 服务器可执行文件：启动内置 HTTP 服务并托管 Web 前端。
//
// 用法：
//   calc_server [--port 8080] [--host 127.0.0.1] [--no-browser]
//
// 默认只监听回环地址，不会暴露到局域网。
// getenv 在 MSVC 下会触发弃用警告，这里显式关闭该检查。
// 该宏必须定义在包含任何头文件之前。
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "api/ApiRoutes.h"
#include "core/HttpServer.h"

namespace {

// 尝试打开默认浏览器。失败不影响服务本身。
void openBrowser(const std::string& url) {
#if defined(_WIN32)
    const std::string command = "start \"\" \"" + url + "\"";
    std::system(command.c_str());
#elif defined(__APPLE__)
    const std::string command = "open \"" + url + "\"";
    std::system(command.c_str());
#else
    const std::string command = "xdg-open \"" + url + "\"";
    std::system(command.c_str());
#endif
}

// 取得当前可执行文件路径。失败时返回空字符串。
std::string executablePath() {
#if defined(_WIN32)
    char buffer[MAX_PATH] = {0};
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::string(buffer, length);
#else
    char buffer[4096] = {0};
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    return std::string(buffer, static_cast<size_t>(length > 0 ? length : 0));
#endif
}

// 定位前端资源目录。优先用命令行参数，其次用环境变量，最后用可执行文件所在目录。
std::string resolveWebRoot(const std::string& explicitPath) {
    if (!explicitPath.empty()) return explicitPath;

    if (const char* fromEnv = std::getenv("CALC_WEB_ROOT")) {
        if (*fromEnv != '\0') return fromEnv;
    }

    // 可执行文件同级的 web 目录。
    const std::string exePath = executablePath();
    const size_t slash = exePath.find_last_of("/\\");
    if (slash != std::string::npos) {
        const std::string candidate = exePath.substr(0, slash) + "/web";
        std::ifstream probe(candidate + "/index.html");
        if (probe) return candidate;
    }
    return "web";
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    int port = 8080;
    std::string webRoot;
    bool openBrowserAutomatically = true;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--port" && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        } else if (argument == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (argument == "--web-root" && i + 1 < argc) {
            webRoot = argv[++i];
        } else if (argument == "--no-browser") {
            openBrowserAutomatically = false;
        } else if (argument == "--help" || argument == "-h") {
            std::cout << "用法: calc_server [--host 127.0.0.1] [--port 8080] "
                         "[--web-root <目录>] [--no-browser]\n";
            return 0;
        }
    }

    webRoot = resolveWebRoot(webRoot);

    calc::http::Server server;
    calc::http::registerApiRoutes(server, webRoot);

    std::cout << "综合计算工具 · Web 服务\n";
    std::cout << "前端目录: " << webRoot << "\n";
    std::cout << "正在启动服务..." << std::endl;

    if (!server.start(host, port)) {
        std::cerr << "启动失败：无法绑定 " << host << ":" << port << "\n"
                  << "端口可能已被占用，可换一个端口：calc_server --port 8081\n";
        return 1;
    }

    const std::string url = "http://" + host + ":" + std::to_string(server.boundPort()) + "/";
    std::cout << "服务已就绪: " << url << "\n";
    std::cout << "按 Ctrl+C 停止。" << std::endl;

    if (openBrowserAutomatically) {
        openBrowser(url);
    }

    // 等待服务线程结束。
    while (server.running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return 0;
}
