cmake_minimum_required(VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  if (MSVC)
    # 为 MSVC 在 Debug 配置启用 Edit-and-Continue（如果可用）
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>")
  endif()
endif()

project(engine VERSION 0.1 LANGUAGES CXX)

# 全局 C++ 设置
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 强制使用UTF-8编码（解决中文注释的警告C4819）
if(MSVC)
  add_compile_options(/utf-8)
endif()

# 可选：默认构建为静态库，可通过 -DBUILD_SHARED_LIBS=ON 切换为共享库
option(BUILD_SHARED_LIBS "Build libraries as shared libraries" OFF)

# 方便 IDE 组织
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 子目录（各独立库放在子目录下）
add_subdirectory(engine_rhi)
add_subdirectory(engine_render)

# 可执行目标（保留原有 engine.cpp / engine.h）
add_executable(engine engine.cpp engine.h)

# 链接子库（rhi 是基础层，render 依赖于 rhi）
target_link_libraries(engine PRIVATE engine_render engine_rhi)

# 如果需要，显式设置可执行文件的 CXX 标准属性（兼容旧 CMake）
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET engine PROPERTY CXX_STANDARD 23)
endif()