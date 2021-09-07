cmake_minimum_required(VERSION 3.16)
project(abu_feed)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  abu_debug 
  GIT_REPOSITORY https://github.com/FrancoisChabot/abu-debug.git
  GIT_TAG        v1.0-alpha
)

FetchContent_MakeAvailable(abu_debug )


add_library(abu_feed INTERFACE)
target_compile_features(abu_feed INTERFACE cxx_std_20)
target_include_directories(abu_feed INTERFACE include)
target_link_libraries(abu_feed INTERFACE abu::debug)

add_library(abu::feed ALIAS abu_feed)
add_subdirectory(tests)