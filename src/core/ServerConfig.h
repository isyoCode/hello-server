#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <string>
#include <thread>
#include <cstddef>

namespace yoyo {

struct ServerConfig {
    // 服务器基础配置
    std::string listenIP = "0.0.0.0";
    int listenPort = 8888;
    std::string staticFilesRoot = "./resource";

    // 线程池配置
    size_t threadPoolSize = 0;  // 0表示自动检测CPU核心数

    // 日志配置
    struct LogConfig {
        bool enableConsole = true;
        bool enableFile = true;
        bool enableColor = true;
        bool enableRotate = true;
        std::string logDir = "server_logs";
        std::string logFileName = "tcp_server";
        std::string logPrefixPath = ".";
        size_t fileMaxSize = 10 * 1024 * 1024;  // 10MB
        size_t fileBufferSize = 64 * 1024;      // 64KB for production (better performance)
        size_t consoleBufferSize = 32 * 1024;   // 32KB for console
    } log;

    // HTTP配置
    struct HttpConfig {
        size_t maxHeaderSize = 8 * 1024;        // 8KB
        size_t maxBodySize = 1 * 1024 * 1024;   // 1MB
        size_t sendfileThreshold = 64 * 1024;   // 大于64KB的文件使用sendfile
        size_t sendfileChunkSize = 256 * 1024;  // sendfile每次发送256KB
    } http;

    // 连接配置
    struct ConnectionConfig {
        size_t maxConnections = 10000;
        int keepAliveTimeout = 60;  // 秒
        bool tcpNoDelay = true;     // 禁用Nagle算法，减少延迟
    } connection;

    // 性能优化配置
    struct PerformanceConfig {
        bool enableFileCache = true;            // 启用小文件内存缓存
        size_t fileCacheMaxSize = 10 * 1024;    // 只缓存小于10KB的文件
        size_t fileCacheMaxEntries = 100;       // 最多缓存100个文件
        bool useEpollET = false;                // 是否使用ET模式（默认LT更稳定）
    } performance;

    // 根据环境自动优化配置
    void autoOptimize() {
        if (threadPoolSize == 0) {
            unsigned int cores = std::thread::hardware_concurrency();
            threadPoolSize = cores > 0 ? cores : 4;
        }
    }

    // 为开发环境设置（更多日志，更小的缓冲区便于调试）
    static ServerConfig forDevelopment() {
        ServerConfig config;
        config.log.fileBufferSize = 1 * 1024;   // 1KB - 快速刷新方便调试
        config.log.consoleBufferSize = 1 * 1024;
        config.performance.enableFileCache = false;  // 禁用缓存，总能看到最新文件
        return config;
    }

    // 为生产环境设置（高性能）
    static ServerConfig forProduction() {
        ServerConfig config;
        config.log.enableConsole = false;           // 生产环境不输出到控制台
        config.log.fileBufferSize = 256 * 1024;     // 256KB - 减少刷盘次数
        config.log.consoleBufferSize = 128 * 1024;
        config.performance.enableFileCache = true;   // 启用文件缓存
        config.http.sendfileChunkSize = 1024 * 1024; // 1MB chunks for better throughput
        config.autoOptimize();
        return config;
    }
};

} // namespace yoyo

#endif
