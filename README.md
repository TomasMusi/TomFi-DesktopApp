# 💻 TomFi Desktop — Banking Client (C++ / GTK)

A native desktop version of the **TomFi** banking system — rebuilt in **C++** using **GTK** for UI.

> 🎯 This version brings the web app experience to desktop environments with a sleek, responsive interface.

---

![Screenshot from 2025-06-16 18-25-28](https://github.com/user-attachments/assets/fecece48-bee5-41f7-b28c-d9e29db3d6c8)


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
|----------------------------|-----------------------------------------------------------------------------|
| 🖥️ **Modern UI**            | Built with GTKmm 3.0 and styled using external CSS                         |
| 🔒 **Secure Registration**  | Passwords hashed with bcrypt before storage                                |
| 👤 **Login System**         | Form validation with string trimming, matching, and length checks          |
| 📱 **Discord Activity**     | Shows real-time presence while using the app                               |
| 🔐 **Validation Rules**     | Checks for empty fields, length, matching passwords, and forbidden spaces |
| 🔧 **Configurable via .env**| Keep secrets out of code using a custom `.env` loader                      |
| 🧪 **Modular Structure**    | Separated into login, register, welcome, and main logic                    |

---

## 🛠 Tech Stack

- **Language**: C++17
- **GUI**: GTKmm 3.0
- **Styling**: CSS-like `style.css` for GTK
- **Security**: bcrypt (via `bcrypt` C++ library)
- **Discord**: Discord Game SDK (Rich Presence)
- **Build Tool**: CMake
- **Env Loading**: Custom `.env` parser (header-only)

---

## 🖼️ Screenshots

![Screenshot from 2025-06-16 18-25-39](https://github.com/user-attachments/assets/96399ebb-4b91-4484-875f-d8c6919972c0)


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

- [x] Welcome screen
- [x] Login form
- [x] Register form
- [x] Password validation
- [x] .env file support
- [x] Discord Rich Presence
- [ ] SQLite or server-based data storage
- [ ] Full dashboard with transaction views
- [ ] Account syncing with web version

---

## 📁 Directory Structure

```
├── main.cpp
├── welcome.cpp / welcome.h
├── login/
│   └── login_page.cpp / login_page.h
├── register/
│   └── register_page.cpp / register_page.h
├── discord-activity/
│   └── discord_integration.cpp
├── env.hpp / env.cpp
├── style.css
├── .env
├── CMakeLists.txt
└── .gitignore
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
