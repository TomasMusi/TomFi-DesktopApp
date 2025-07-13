# 💻 TomFi Desktop — Banking Client (C++ / GTK)

A native desktop version of the **TomFi** banking system — rebuilt in **C++** using **GTK** for UI.

> 🎯 This version brings the web app experience to desktop environments with a sleek, responsive interface.

---

![Screenshot from 2025-07-05 13-27-06](https://github.com/user-attachments/assets/c0baa42a-e0b8-4cb9-a9ef-6151c2dd2921)


## 🧩 About the Project

**TomFi Desktop** is a reimplementation of the original TomFi project with a native GUI built from scratch. It aims to bring desktop-native performance and deeper OS integration, while maintaining the core features of the web version.

Throughout this version, I'm exploring:

- 🧱 **GTKmm** — native desktop GUI with C++ bindings
- 🔐 **bcrypt** — secure password hashing
- 💬 **Discord Rich Presence** — live status while the app is running
- 🌐 **Environment variables** using `.env` loader
- ✨ Smooth UI with CSS-like styling

---

## ✨ Features

| Feature                     | Description                                                                 |
|---------------------------- |-----------------------------------------------------------------------------|
| 🖥️ **Modern UI**            | Built with GTKmm 3.0 and styled using external CSS                          |
| 🔒 **Secure Registration**  | Passwords hashed with bcrypt before storage                                 |
| 👤 **Login System**         | Form validation with string trimming, matching, and length checks           |
| 📱 **Discord Activity**     | Shows real-time presence while using the app                                |
| 🔐 **Validation Rules**     | Checks for empty fields, length, matching passwords, and forbidden spaces   |
| 🔧 **Configurable via .env**| Keep secrets out of code using a custom `.env` loader                       |
| 🧪 **Modular Structure**    | Separated into login, register, welcome, and main logic                     |
| 🌐 **Web Chart Integration**| Interactive charts embedded using WebKitWebView for HTML/JS rendering       |
 

---

## 🛠 Tech Stack

- **Language**: C++17
- **GUI**: GTKmm 3.0
- **Web Embedding**: WebKitGTK via `WebKitWebView`
- **Styling**: CSS-like `style.css` for GTK
- **Security**: bcrypt (via `bcrypt` C++ library)
- **Discord**: Discord Game SDK (Rich Presence)
- **Build Tool**: CMake
- **Env Loading**: Custom `.env` parser (header-only)


---

## 🖼️ Screenshots

![Screenshot from 2025-07-08 18-55-51](https://github.com/user-attachments/assets/5eeafb9c-b2cf-4188-8bd0-db3362650021)


---

## 🧠 What I Learned

This project taught me:

- Structuring a C++ app with real GUI
- Using signals, widgets, and style providers in GTKmm
- Proper string validation and error handling
- Integrating third-party SDKs like Discord Presence
- Making builds simpler using CMake
- Reading `.env` configs without exposing secrets

---

## 📌 Roadmap

- [x] **RSA Encryption** for secure PIN and key handling 
- [x] **Account syncing** with web version via shared DB schema 
- [x] **.env environment variable system** for secure configuration 
- [x] **SQLite & MySQL support** for local and remote storage 
- [x] **Secure user authentication** with password validation (bcrypt) 
- [x] **GUI creation** using GTK (login, registration, forms) 
- [x] **Full database integration** with prepared statements and transactions 
- [x] **Discord Rich Presence** for real-time session display 
- [X] **Full-featured dashboard** (account overview, transaction history) 
- [ ] **Transaction functionality** (send/receive, balance updates) 
- [X] **2FA (Two-Factor Authentication)** with TOTP setup 
- [X] **Analytics panel** with spending charts and category breakdowns 
- [X] **JWT-like token session handling** for secure client access 
---

## 📁 Directory Structure

```
├── backend-node/
│ ├── 2fa/
│ │ ├── 2fa-login
│ │ ├── Check
│ │ └── Create
│ ├── db.ts
│ ├── db_types.ts
│ ├── node_modules/
│ ├── package.json
│ ├── package-lock.json
│ └── tsconfig.json
├── bcrypt/
│ ├── bcrypt.c
│ ├── bcrypt.h
│ ├── COPYING
│ ├── crypt_blowfish/
│ │ ├── crypt.3
│ │ ├── crypt_blowfish.c
│ │ ├── crypt_blowfish.h
│ │ ├── crypt_gensalt.c
│ │ ├── crypt_gensalt.h
│ │ ├── crypt.h
│ │ ├── glibc-*.diff
│ │ ├── LINKS
│ │ ├── Makefile
│ │ ├── ow-crypt.h
│ │ ├── PERFORMANCE
│ │ ├── README
│ │ ├── wrapper.c
│ │ └── x86.S
│ ├── Makefile
│ └── README
├── build/
│ ├── CMakeCache.txt
│ ├── CMakeFiles/
│ ├── cmake_install.cmake
│ ├── gtk_hello
│ ├── Makefile
│ ├── README.md
│ └── style.css
├── dashboard/
│ ├── chart/
│ │ ├── chart.cpp
│ │ ├── chart.h
│ │ ├── chart.html
│ │ ├── longChart.cpp
│ │ ├── longChart.h
│ │ └── longChart.html
│ ├── dashboard.cpp
│ ├── dashboard.h
│ ├── settings/
│ │ ├── settings.cpp
│ │ └── settings.h
│ └── Transaction/
│ ├── transaction.cpp
│ └── transaction.h
├── db/
│ ├── database.cpp
│ └── database.h
├── discord-activity/
│ ├── discord_integration.cpp
│ └── discord_integration.h
├── discord-rpc/
│ ├── appveyor.yml
│ ├── build.py
│ ├── CMakeLists.txt
│ ├── documentation/
│ │ ├── hard-mode.md
│ │ └── images/
│ ├── examples/
│ │ ├── button-clicker
│ │ ├── send-presence
│ │ └── unrealstatus
│ ├── include/
│ │ ├── discord_register.h
│ │ └── discord_rpc.h
│ ├── LICENSE
│ ├── README.md
│ └── src/
│ ├── backoff.h
│ ├── CMakeLists.txt
│ ├── connection.h
│ ├── connection_unix.cpp
│ ├── connection_win.cpp
│ ├── discord_register_linux.cpp
│ ├── discord_register_osx.m
│ ├── discord_register_win.cpp
│ ├── discord_rpc.cpp
│ ├── dllmain.cpp
│ ├── msg_queue.h
│ ├── rpc_connection.cpp
│ ├── rpc_connection.h
│ ├── serialization.cpp
│ └── serialization.h
├── keys/
│ ├── private.pem
│ └── public.pem
├── login/
│ ├── login_page.cpp
│ └── login_page.h
├── register/
│ ├── register_page.cpp
│ └── register.h
├── Session/
│ ├── Session.cpp
│ └── Session.hpp
```


---

## 🚀 Building Locally

```bash
# Clone repository
git clone https://github.com/TomasMusi/TomFi-DesktopApp.git
cd TomFi-DesktopApp

# Create build folder
mkdir build && cd build

# Run CMake and build
cmake ..
make

# Run the app
./gtk_hello
