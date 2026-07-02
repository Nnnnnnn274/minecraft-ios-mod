cmake_minimum_required(VERSION 3.20)
project(MinecraftClientLoader
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
endif()

set(SOURCES
    src/AntiPiracyCheck.mm
    src/HookInit.cpp
    src/NetworkGuard.cpp
    src/ClientUIManager.cpp
    src/OffsetConfig.cpp
    src/fishhook.c
    src/XboxAuthFix.cpp
    src/api/Paperweight.cpp
    src/api/Hardweight.cpp
    src/api/APIScreen.mm
)

set(HEADERS
    src/NetworkGuard.hpp
    src/HookInit.hpp
    src/ClientUIManager.hpp
    src/VerificationBridge.h
    src/OffsetConfig.h
    src/fishhook.h
    src/stb_image.h
    src/XboxAuthFix.hpp
    src/api/Paperweight.h
    src/api/Hardweight.h
    src/api/APIScreen.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -fobjc-arc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    "-framework UIKit"
    "-framework Foundation"
    "-framework OpenGLES"
    "-framework Security"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".dylib"
    PREFIX ""
    MACOSX_RPATH ON
)
