#pragma once

#include <format>
#include <fstream>
#include <mutex>
#include "core/Types.h"

namespace render::utils
{

    /**
     * @brief 日志级别
     * @details 消息的重要程度分类
     */
    enum class LogLevel {
        Trace,      ///< 跟踪（最详细）
        Debug,      ///< 调试信息
        Info,       ///< 普通信息
        Warning,    ///< 警告
        Error,      ///< 错误
        Fatal       ///< 致命错误
    };

    /**
     * @brief 日志类别
     * @details 日志消息的来源模块
     */
    enum class LogCategory : u32 {
        General  = 0,       ///< 通用日志
        Render   = 1 << 0,  ///< 渲染相关
        Resource = 1 << 1,  ///< 资源管理
        Pipeline = 1 << 2,  ///< 管线管理
        Graph    = 1 << 3,  ///< 渲染图
        Device   = 1 << 4,  ///< 设备管理
        Memory   = 1 << 5,  ///< 内存管理
        All      = 0xFFFFFFFF  ///< 所有类别
    };

    inline LogCategory operator|(LogCategory a, LogCategory b) {
        return static_cast<LogCategory>(
            static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline LogCategory operator&(LogCategory a, LogCategory b) {
        return static_cast<LogCategory>(
            static_cast<u32>(a) & static_cast<u32>(b));
    }

    // 日志配置
    struct LogConfig {
        LogLevel    minLevel         = LogLevel::Debug;
        LogCategory categories       = LogCategory::All;
        bool        enableConsole    = true;
        bool        enableFile       = false;
        const char* logFile          = "render.log";
        bool        enableColors     = true;
        bool        includeTimestamp = true;
        bool        includeCategory  = true;
    };

    // 日志系统
    class Logger {
        public:
            static Logger& get();

            void initialize(const LogConfig& config);
            void shutdown();

            void log(LogLevel level, LogCategory category, const char* message);
            void log(LogLevel level, LogCategory category, const std::string& message);

            template<typename... Args>
            void log(LogLevel                    level, LogCategory category,
                     std::format_string<Args...> fmt, Args&&...     args) {
                if (shouldLog(level, category)) {
                    std::string message = std::format(fmt, std::forward<Args>(args)...);
                    logInternal(level, category, message.c_str());
                }
            }

            void setMinLevel(LogLevel level) { config.minLevel = level; }
            void setCategories(LogCategory categories) { config.categories = categories; }
            void enableCategory(LogCategory category) {
                config.categories = config.categories | category;
            }
            void disableCategory(LogCategory category) {
                config.categories = static_cast<LogCategory>(
                    static_cast<u32>(config.categories) & ~static_cast<u32>(category));
            }

            bool shouldLog(LogLevel level, LogCategory category) const;

        private:
            Logger()  = default;
            ~Logger() = default;

            void        logInternal(LogLevel level, LogCategory category, const char* message);
            const char* getLevelString(LogLevel level) const;
            const char* getCategoryString(LogCategory category) const;
            const char* getLevelColor(LogLevel level) const;

            LogConfig     config;
            std::mutex    mutex;
            std::ofstream fileStream;
            bool          initialized = false;
    };

    // 便捷宏
    #define LOG_TRACE(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Trace, \
        render::utils::LogCategory::General, fmt, ##__VA_ARGS__)
    #define LOG_DEBUG(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Debug, \
        render::utils::LogCategory::General, fmt, ##__VA_ARGS__)
    #define LOG_INFO(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Info, \
        render::utils::LogCategory::General, fmt, ##__VA_ARGS__)
    #define LOG_WARNING(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Warning, \
        render::utils::LogCategory::General, fmt, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Error, \
        render::utils::LogCategory::General, fmt, ##__VA_ARGS__)
    #define LOG_FATAL(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Fatal, \
        render::utils::LogCategory::General, fmt, ##__VA_ARGS__)

    // 分类日志宏
    #define LOG_RENDER_TRACE(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Trace, \
        render::utils::LogCategory::Render, fmt, ##__VA_ARGS__)
    #define LOG_RENDER_DEBUG(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Debug, \
        render::utils::LogCategory::Render, fmt, ##__VA_ARGS__)
    #define LOG_RENDER_INFO(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Info, \
        render::utils::LogCategory::Render, fmt, ##__VA_ARGS__)

    #define LOG_RESOURCE_TRACE(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Trace, \
        render::utils::LogCategory::Resource, fmt, ##__VA_ARGS__)
    #define LOG_RESOURCE_DEBUG(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Debug, \
        render::utils::LogCategory::Resource, fmt, ##__VA_ARGS__)

    #define LOG_GRAPH_TRACE(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Trace, \
        render::utils::LogCategory::Graph, fmt, ##__VA_ARGS__)
    #define LOG_GRAPH_DEBUG(fmt, ...) \
    render::utils::Logger::get().log(render::utils::LogLevel::Debug, \
        render::utils::LogCategory::Graph, fmt, ##__VA_ARGS__)

    // 断言
    #ifdef _DEBUG
    #define ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                LOG_FATAL("Assertion failed: {} at {}:{}", message, __FILE__, __LINE__); \
                __debugbreak(); \
            } \
        } while(0)
    #else
    #define ASSERT(condition, message) ((void)0)
    #endif

    #define ASSERT_NOT_NULL(ptr) ASSERT(ptr != nullptr, #ptr " is null")
    #define ASSERT_VALID_HANDLE(handle) ASSERT(handle.isValid(), #handle " is invalid")

}
