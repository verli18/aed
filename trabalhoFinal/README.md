# Library Manager

Modern C++ terminal application for managing a library catalogue, powered by SQLite and the in-house `chrmaTUI` toolkit.

## Requirements

- CMake 3.20+
- A C++20 capable compiler (GCC 11+, Clang 12+, MSVC 19.30+)
- SQLite development package (`libsqlite3-dev` on Debian/Ubuntu, `sqlite` on Arch, `sqlite-devel` on Fedora)

## Configure & Build

```bash
cmake -S . -B build
cmake --build build
```

The resulting executable `library_manager` is placed in `build/`.

## Run

```bash
./build/library_manager [path/to/database.db]
```

Without an explicit path the application creates/opens `library_manager.db` in the project directory and ensures the base schema exists.

## Project Layout

- `src/` – application sources
- `include/` – project headers (reserved for upcoming modules)
- `lib/chrmaTUI/` – bundled terminal UI library

## Next steps

- Flesh out the data access layer (CRUD for patrons, loans, etc.)
- Integrate `chrmaTUI` views for listing and editing records
- Add automated tests once the domain logic takes shape
