cmake_minimum_required(VERSION 3.10)
project(gtk_hello_app)

set(CMAKE_CXX_STANDARD 17)

# Enable compiler warnings
add_compile_options(-Wall -Wextra)

# ─── Define SOURCE_DIR macro to access source path from C++ ──────────────
add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

# ─── GTKMM ───────────────────────────────────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
include_directories(${GTKMM_INCLUDE_DIRS})
add_definitions(${GTKMM_CFLAGS_OTHER})

# ─── WebKitGTK ──────────────────────────────────────
pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
include_directories(${WEBKIT2GTK_INCLUDE_DIRS})
add_definitions(${WEBKIT2GTK_CFLAGS_OTHER})
include_directories(/usr/include/webkitgtk-4.1) # Manual fallback include

# ─── JsonCpp ────────────────────────────────────────
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})
add_definitions(${JSONCPP_CFLAGS_OTHER})

# ─── Project includes ───────────────────────────────
include_directories(${PROJECT_SOURCE_DIR})
include_directories(
    ${CMAKE_SOURCE_DIR}/discord-rpc/include
    ${CMAKE_SOURCE_DIR}/bcrypt
    ${CMAKE_SOURCE_DIR}/bcrypt/crypt_blowfish
    ${CMAKE_SOURCE_DIR}/db
    /usr/include/mariadb
)

# ─── Source files ───────────────────────────────────
set(SOURCES
    main.cpp
    Session/Session.cpp
    db/database.cpp
    welcome.cpp
    env.cpp
    dashboard/dashboard.cpp
    dashboard/chart/chart.cpp
    dashboard/chart/longChart.cpp
    dashboard/wallet/wallet.cpp
    dashboard/settings/settings.cpp
    login/login_page.cpp
    register/register_page.cpp
    discord-activity/discord_integration.cpp
    dashboard/Transaction/transaction.cpp

    # bcrypt
    bcrypt/bcrypt.c
    bcrypt/crypt_blowfish/crypt_blowfish.c
    bcrypt/crypt_blowfish/crypt_gensalt.c
    bcrypt/crypt_blowfish/wrapper.c

    # Discord RPC
    discord-rpc/src/discord_rpc.cpp
    discord-rpc/src/rpc_connection.cpp
    discord-rpc/src/serialization.cpp
    discord-rpc/src/connection_unix.cpp
    discord-rpc/src/discord_register_linux.cpp
)

# ─── OpenSSL ────────────────────────────────────────
find_package(OpenSSL REQUIRED)

# ─── Executable ─────────────────────────────────────
add_executable(gtk_hello ${SOURCES})

# ─── Link Libraries ─────────────────────────────────
target_link_libraries(gtk_hello
    ${GTKMM_LIBRARIES}
    ${WEBKIT2GTK_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    mariadb
    OpenSSL::Crypto
)
