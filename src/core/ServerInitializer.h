#ifndef __SERVER_INITIALIZER_H__
#define __SERVER_INITIALIZER_H__

#include "../logger/logger.hpp"
#include "ServerConfig.h"

namespace yoyo {

/**
 * 服务器初始化器 - 封装所有初始化逻辑
 * 可以在任何地方调用，不必放在main中
 */
class ServerInitializer {
public:
    // 方案1：使用配置对象初始化
    static void initialize(const ServerConfig& config) {
        initLogger(config.log);
    }

    // 方案2：使用默认配置初始化
    static void initializeDefault() {
        auto config = ServerConfig::forProduction();
        initialize(config);
    }

    // 方案3：延迟初始化 - 第一次使用时自动初始化
    static void ensureInitialized() {
        static bool initialized = false;
        if (!initialized) {
            initializeDefault();
            initialized = true;
        }
    }

private:
    static void initLogger(const ServerConfig::LogConfig& logConfig) {
        Logger::getInstance()
            ->setPrefixPath(logConfig.logPrefixPath)
            .setLogDirName(logConfig.logDir)
            .setLogFileName(logConfig.logFileName)
            .setConsle(logConfig.enableConsole)
            .setColor(logConfig.enableColor)
            .setWritefile(logConfig.enableFile)
            .setRotate(logConfig.enableRotate)
            .setFileMaxSize(logConfig.fileMaxSize);

        // 注意：缓冲区大小需要在logger.hpp中暴露setter接口
        // 或者在初始化时直接设置
    }
};

} // namespace yoyo

#endif
