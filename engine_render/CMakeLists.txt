# engine_render 子目录的 CMake：将渲染模块构建为独立库（支持头文件仅库）
# 约定：engine_render/include 放公共头，engine_render/src 放实现文件

# 搜集实现文件（如果没有实现文件，会退化为 INTERFACE 库，仅提供头）
file(GLOB_RECURSE ENGINE_RENDER_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# 搜集头文件（用于在IDE中显示，不影响构建）
file(GLOB_RECURSE ENGINE_RENDER_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

# 判断是否为 header-only（没有实现文件）
set(ENGINE_RENDER_IS_HEADER_ONLY FALSE)
if (ENGINE_RENDER_SOURCES STREQUAL "")
  set(ENGINE_RENDER_IS_HEADER_ONLY TRUE)
endif()

if (ENGINE_RENDER_IS_HEADER_ONLY)
  # 创建INTERFACE库，但使用FILE_SET确保头文件在IDE中显示
  add_library(engine_render INTERFACE)
  
  # 使用FILE_SET将头文件附加到目标（CMake 3.23+）
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.23")
    target_sources(engine_render INTERFACE FILE_SET HEADERS TYPE HEADERS
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
      FILES ${ENGINE_RENDER_HEADERS}
    )
  else()
    # CMake 3.23之前，使用INTERFACE源文件
    target_sources(engine_render INTERFACE ${ENGINE_RENDER_HEADERS})
  endif()
  
  # 暴露 include 目录给使用者
  target_include_directories(engine_render INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
else()
  add_library(engine_render ${ENGINE_RENDER_SOURCES})
  
  # 将头文件添加到目标（在IDE中显示）
  target_sources(engine_render PUBLIC ${ENGINE_RENDER_HEADERS})
  
  target_include_directories(engine_render PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  
  # 对于非 header-only 库，确保源文件所在目录在 IDE 中可见
  target_sources(engine_render PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  )
endif()
    
# 要求 C++23（较新 CMake 的推荐写法），针对 INTERFACE 或 实体库设置正确的作用域
if (ENGINE_RENDER_IS_HEADER_ONLY)
  target_compile_features(engine_render INTERFACE cxx_std_23)
else()
  target_compile_features(engine_render PUBLIC cxx_std_23)
endif()

# 对于静态库启用 PIC（在需要时）
if (NOT BUILD_SHARED_LIBS AND NOT ENGINE_RENDER_IS_HEADER_ONLY)
  set_target_properties(engine_render PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# 推荐的警告级别（可改），为 INTERFACE 库使用 INTERFACE 作用域，为普通库使用 PRIVATE
if (ENGINE_RENDER_IS_HEADER_ONLY)
  target_compile_options(engine_render INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
  )
else()
  target_compile_options(engine_render PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
  )
endif()

# ============================================================================
# 测试程序
# ============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/render_graph_test.cpp")
  add_executable(render_graph_test examples/render_graph_test.cpp)
  target_link_libraries(render_graph_test PRIVATE engine_render)
  target_include_directories(render_graph_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  target_compile_features(render_graph_test PRIVATE cxx_std_23)
endif()