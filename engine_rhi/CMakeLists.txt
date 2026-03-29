# engine_rhi 子目录的 CMake：渲染硬件接口层，封装 Vulkan

# 支持 Conan 依赖查找 - 尝试多个可能的路径
# 1. 当前构建目录的 Release/Debug 子目录
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/Release/generators")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/Debug/generators")
# 2. 项目根目录的 build 文件夹（Conan 默认输出位置）
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/Release/generators")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/Debug/generators")
# 3. CLion 默认构建目录
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/cmake-build-debug-visual-studio/Release/generators")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/cmake-build-debug-visual-studio/Debug/generators")

# 查找 Vulkan SDK（本地安装）
find_package(Vulkan REQUIRED)

# 查找 VMA（通过 Conan）- 优先使用 Conan 提供的查找
find_package(VulkanMemoryAllocator REQUIRED)

# 搜集实现文件
file(GLOB_RECURSE ENGINE_RHI_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# 搜集头文件（用于在IDE中显示）
file(GLOB_RECURSE ENGINE_RHI_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

# 判断是否为 header-only
set(ENGINE_RHI_IS_HEADER_ONLY FALSE)
if (ENGINE_RHI_SOURCES STREQUAL "")
  set(ENGINE_RHI_IS_HEADER_ONLY TRUE)
endif()

if (ENGINE_RHI_IS_HEADER_ONLY)
  # 创建 INTERFACE 库
  add_library(engine_rhi INTERFACE)

  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.23")
    target_sources(engine_rhi INTERFACE FILE_SET HEADERS TYPE HEADERS
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
      FILES ${ENGINE_RHI_HEADERS}
    )
  else()
    target_sources(engine_rhi INTERFACE ${ENGINE_RHI_HEADERS})
  endif()

  target_include_directories(engine_rhi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )

  # 链接 Vulkan 和 VMA
  target_link_libraries(engine_rhi INTERFACE Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
else()
  add_library(engine_rhi ${ENGINE_RHI_SOURCES})

  target_sources(engine_rhi PUBLIC ${ENGINE_RHI_HEADERS})

  target_include_directories(engine_rhi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )

  # VMA 头文件路径
  target_include_directories(engine_rhi PRIVATE
    ${VulkanMemoryAllocator_INCLUDE_DIRS}
  )

  target_sources(engine_rhi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  )

  # 链接 Vulkan 和 VMA
  target_link_libraries(engine_rhi PUBLIC Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
endif()

# C++23
target_compile_features(engine_rhi INTERFACE cxx_std_23)

# PIC
if (NOT BUILD_SHARED_LIBS AND NOT ENGINE_RHI_IS_HEADER_ONLY)
  set_target_properties(engine_rhi PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# 警告级别
if (ENGINE_RHI_IS_HEADER_ONLY)
  target_compile_options(engine_rhi INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
  )
else()
  target_compile_options(engine_rhi PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
  )
endif()
