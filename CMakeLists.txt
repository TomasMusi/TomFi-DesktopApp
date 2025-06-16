cmake_minimum_required(VERSION 3.10)
project(gtk_hello_app)

set(CMAKE_CXX_STANDARD 17)

# Enable warnings
add_compile_options(-Wall -Wextra)

# Find GTKMM using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# Add GTKMM include directories and compiler flags
include_directories(${GTKMM_INCLUDE_DIRS})
add_definitions(${GTKMM_CFLAGS_OTHER})


# Add include directories for Discord RPC and cpp-dotenv
include_directories(
    ${CMAKE_SOURCE_DIR}/discord-rpc/include
)

# Source files (exactly as in your g++ command)
set(SOURCES
    main.cpp
    welcome.cpp
    env.cpp
    login/login_page.cpp
    register/register_page.cpp
    discord-activity/discord_integration.cpp
    discord-rpc/src/discord_rpc.cpp
    discord-rpc/src/rpc_connection.cpp
    discord-rpc/src/serialization.cpp
    discord-rpc/src/connection_unix.cpp
    discord-rpc/src/discord_register_linux.cpp
)

# Build executable
add_executable(gtk_hello ${SOURCES})



# Link GTKMM libraries
target_link_libraries(gtk_hello ${GTKMM_LIBRARIES})
