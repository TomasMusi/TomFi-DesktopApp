# 💻 TomFi Desktop — Banking Client (C++ / GTK)

A native desktop version of the **TomFi** banking system — rebuilt in **C++** using **GTK** for UI.

> 🎯 This version brings the web app experience to desktop environments with a sleek, responsive interface.

---

<img width="1037" height="627" alt="image" src="https://github.com/user-attachments/assets/11c24c6b-fd21-4878-809e-e6ceadda3ac0" />


## 🧩 About the Project

TomFi Desktop is a high-performance native banking client written in **C++17**, designed to mirror and extend the functionality of the TomFi web platform. Built using **GTKmm 3.0** for the GUI and **MySQL/MariaDB** as its persistent backend, the app is engineered with modern security, modular architecture, and direct system integration in mind.

> 🔒 Features include end-to-end encrypted PIN validation, secure user authentication, RSA key handling, live analytics, and cross-platform transaction syncing via a shared database schema.


- 🧱 **GTKmm** — native desktop GUI with C++ bindings
- 🔐 **bcrypt** — secure password hashing
- 💬 **Discord Rich Presence** — live status while the app is running
- 🌐 **Environment variables** using `.env` loader
- ✨ Smooth UI with CSS-like styling

---

## ✨ Features

| Feature                     | Description                                                                 |
|---------------------------- |-----------------------------------------------------------------------------|
| 🖥️ **Custom GTK UI**        | Fully designed from scratch with CSS-like theming and responsive layout     |
| 🔒 **RSA-secured PIN checks**| PINs are stored encrypted and decrypted using a private key on-the-fly     |
| 👤 **Login System**         | Form validation with string trimming, matching, and length checks           |
| 📱 **Discord Activity**     | Shows real-time presence while using the app                                |
| 🔐 **Validation Rules**     | Checks for empty fields, length, matching passwords, and forbidden spaces   |
| 🔧 **Configurable via .env**| Keep secrets out of code using a custom `.env` loader                       |
| 🧪 **Modular Structure**    | Separated into login, register, welcome, and main logic                     |
| 🌐 **Web Chart Integration**| Interactive charts embedded using WebKitWebView for HTML/JS rendering       |
| 📡 **Database Transactions**| Secure balance update & rollback logic via MySQL prepared statements        |
| 📱 **2FA with QR Support**    | User setup and TOTP validation with backend-issued secrets                |


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

<img width="1231" height="569" alt="image" src="https://github.com/user-attachments/assets/43e84839-2a24-4ed5-93bd-ebc7fcd1caad" />


---

## 🧠 What I Learned

This project taught me:

- Writing modular, maintainable C++ with proper class abstraction
- Implementing multi-layer security: bcrypt, RSA, session tokens
- Integrating external SDKs (Discord, TOTP) natively
- Embedding and interacting with web-rendered data visualizations (HTML/JS charts)
- Developed both frontend and backend in C++ while integrating with a Node.js backend through direct HTTP communication for seamless interoperability.- Practicing defensive programming: all DB operations are injection-resistant
- Mastering cross-language infrastructure via CMake

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
- [X] **Transaction functionality** (send/receive, balance updates) 
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
