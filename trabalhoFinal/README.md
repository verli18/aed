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
# Normal mode (launches UI)
./build/library_manager [path/to/database.db]

# Test mode (exercises database layer)
./build/library_manager --test [path/to/database.db]

# Help
./build/library_manager --help
```

Without an explicit path the application creates/opens `library_manager.db` in the current directory and ensures the base schema exists.

## Project Layout

- `src/` – application sources
  - `app/` – database layer and business logic
    - `db.cpp` – SQLite RAII wrappers (Database, Statement, Transaction)
    - `schema.cpp` – migration system with versioned schema updates
    - `db_test.cpp` – comprehensive database tests
    - `repos/` – data access layer (Student, Book, Loan repositories)
  - `ui/` – terminal user interface components
  - `main.cpp` – application entry point
- `include/` – project headers
  - `app/` – database and domain model headers
    - `db.hpp` – database abstraction layer
    - `schema.hpp` – schema management
    - `models.hpp` – domain models (Student, Book, Loan, LoanSummary)
    - `repos/` – repository interfaces
- `lib/chrmaTUI/` – bundled terminal UI library
- `docs/` – documentation
  - `library_manager_design.md` – product & implementation design
  - `IMPLEMENTATION_STATUS.md` – current progress tracking
  - `database_guide.md` – database layer usage guide
  - `chrmaTUI.md` – UI toolkit documentation
  - `modal_forms_guide.md` – UI patterns and examples

## Database Features

✅ **Fully Implemented** (M1 Complete)

- **RAII-based SQLite wrapper** with automatic resource management
- **Migration system** with version tracking
- **Complete schema** with students, books, and loans tables
- **Automatic triggers** for book availability management
- **Three repositories** with full CRUD operations:
  - `StudentRepository` – student management + search
  - `BookRepository` – book catalogue + availability tracking
  - `LoanRepository` – loan lifecycle + overdue tracking
- **Transaction support** with automatic rollback on exceptions
- **Comprehensive tests** verifying all database operations

Run `./build/library_manager --test` to see the database layer in action!

## Next steps

- ✅ ~~Flesh out the data access layer (CRUD for patrons, loans, etc.)~~
- 🚧 Integrate `chrmaTUI` views with real repository data
- ⏳ Add service layer with business logic validation
- ⏳ Implement complete UI workflows (registration, loans, returns)
- ⏳ Add automated tests once the domain logic takes shape
