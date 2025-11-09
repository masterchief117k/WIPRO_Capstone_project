# Console File Explorer

A lightweight, console-based file explorer written in **C++17** for Linux.  
It provides interactive directory navigation, file manipulation, search, and permission management using `std::filesystem`.

---

##  Features

- **Interactive CLI** with simple command dispatcher and help.
- **Navigation**
  - `cd`, `pwd`, `ls`
  - History navigation: `back`, `forward`
  - Shortcuts: `cd -` (previous dir), `cd` (home)
- **Bookmarks**
  - `bookmark add|go|rm|list`
- **File operations**
  - `touch`, `mkdir`, `rm`, `cp`, `mv`
- **Search**
  - Substring, glob, or regex search with optional recursion
- **Permissions**
  - View and set permissions with octal or symbolic modes (`perm`, `chmod`)
- **Display settings**
  - Show hidden files toggle
  - Sort by name, size, or time

---


---

##  Build Instructions (CMake)

**Prerequisites:**
- C++17-capable compiler (`g++` or `clang++`)
- CMake 3.16 or newer

**Steps:**

# 1. Configure the project
cmake -S . -B build

# 2. Build the project
cmake --build build --config Release

# 3. Run the executable
./build/file_explorer


