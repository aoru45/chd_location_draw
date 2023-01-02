cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(PROJECT_NAME test)
project(${PROJECT_NAME})
message(STATUS ${DXSDK_DIR})
include_directories(${CMAKE_SOURCE_DIR} ${DXSDK_DIR}/Include ${CMAKE_SOURCE_DIR}/imgui)
link_directories(${DXDK_DIR}/Lib/x86)
set(CHDAR 
    main.cpp
    hook.cpp
    memory.cpp
    game.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_dx9.cpp
    imgui/imgui_impl_win32.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)

add_library(chdar SHARED ${CHDAR})
# add_executable(chdar ${CHDAR})
add_executable(inject inject.cpp)
target_link_libraries(chdar 
dwmapi.lib
d3d9.lib
"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/d3dx9.lib"
)


