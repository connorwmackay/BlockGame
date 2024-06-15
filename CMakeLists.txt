cmake_minimum_required(VERSION 3.20)
project(BlockGame)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GLFW Library
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)                

# Fetch GLAD Library
FetchContent_Declare(glad2
    GIT_REPOSITORY https://github.com/Dav1dde/glad
    GIT_TAG v2.0.6
    GIT_SHALLOW		TRUE
	SOURCE_SUBDIR	cmake    
)
FetchContent_MakeAvailable(glad2)
glad_add_library(glad_gl_core_45 STATIC API gl:core=4.5)

# Fetch GLM Library
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glm)

# Fetch FastNoise 2
FetchContent_Declare(fastnoise2
    GIT_REPOSITORY https://github.com/Auburn/FastNoise2/
    GIT_TAG v0.10.0-alpha)
set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "Build Noise Tool" FORCE)
FetchContent_MakeAvailable(fastnoise2)

# Set ImGui Files to Compile
set(IMGUI_FILES
   "${PROJECT_SOURCE_DIR}/Libraries/imgui/imgui.cpp"
   "${PROJECT_SOURCE_DIR}/Libraries/imgui/imgui_draw.cpp"
   "${PROJECT_SOURCE_DIR}/Libraries/imgui/backends/imgui_impl_opengl3.cpp"
   "${PROJECT_SOURCE_DIR}/Libraries/imgui/backends/imgui_impl_glfw.cpp"
   "${PROJECT_SOURCE_DIR}/Libraries/imgui/imgui_tables.cpp"
   "${PROJECT_SOURCE_DIR}/Libraries/imgui/imgui_widgets.cpp"
)

# Find the project's .cpp and .h files in the src folder
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.h)

# Build an executable using the project's files
add_executable(BlockGame
	${SRC_FILES}
	${IMGUI_FILES}
    "${PROJECT_SOURCE_DIR}/Libraries/stb/stb_image.c"
)

# Setup Header Includes
target_include_directories(BlockGame PUBLIC
    ${PROJECT_SOURCE_DIR}/Libraries/imgui
    ${PROJECT_SOURCE_DIR}/Libraries/stb
)

# Setup the library dependencies
target_link_libraries(BlockGame PUBLIC
    opengl32 glad_gl_core_45 glfw FastNoise glm::glm
)