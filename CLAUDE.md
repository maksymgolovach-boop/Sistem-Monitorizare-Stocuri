# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## Build & Run

```bash
# Configure (from project root — Qt Creator sets this automatically)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Build application
cmake --build build

# Run application
./build/Sistem_Monitorizare_Stocuri        # Linux
build\Sistem_Monitorizare_Stocuri.exe      # Windows
```

## Tests

```bash
# Build + run all 6 test executables via CTest
cmake --build build && ctest --test-dir build --output-on-failure

# Run a single test suite (verbose)
./build/tst_produs -v2
./build/tst_warehousemanager -v2

# Headless Linux (tst_exportmanager uses QApplication)
QT_QPA_PLATFORM=offscreen ./build/tst_exportmanager -v2
# Note: CMakeLists already sets QT_QPA_PLATFORM=offscreen for ctest automatically
```

---

## Architecture

The project uses a strict 4-layer architecture. Each layer knows only about the layer below it — UI never touches storage directly.

```
LibrarieModele/     Pure C++ data models (no Qt, no I/O)
    Produs          Product entity with UUID, validation, JSON roundtrip
    Tranzactie<T>   Template transaction; always used as TranzactieProdus

ManagerStocare/     Storage abstraction
    IStocare        Pure-virtual interface (salveaza/incarca produse + tranzactii)
    JsonStorage     Concrete implementation using QSaveFile (atomic writes)

NivelStocareDate/   Business logic (no UI, no JSON)
    WarehouseManager(IStocare*)   CRUD + stock ops + alert detection
    TranzactieStorage<T>(IStocare*) Transaction history + filtering

NivelUI/            Qt dialogs only — no business logic
    mainwindow.cpp  Controller + layout (not in NivelUI/ but same role)
    exportmanager   Static utility: CSV/PDF from any QTableWidget
```

`IStocare*` is injected into both `WarehouseManager` and `TranzactieStorage` at construction — this is why tests can use `MockStocare` without touching the filesystem.

## Key Design Decisions

**Two separate JSON files** — products in `depozit.json`, transactions in `tranzactii.json`. Both paths default to `QStandardPaths::AppDataLocation + "/SistemMonitorizareStocuri/"` and are persisted in `QSettings` (org: `SistemMonitorizare`, app: `StocManager`).

**`UpdateUI()` pattern** — any write operation (add/edit/delete/transaction) calls `UpdateUI()` which refreshes all four pages. `produseSubPrag()` is expensive (O(n)); it is computed **once** inside `UpdateUI()` and passed as a `const std::vector<Produs>&` to `populateDashboard`, `populateAlertsTable`, and `updateHeaderBadges`. Do not add extra `produseSubPrag()` calls in populate methods.

**`TranzactieStorage::adauga()` does NOT auto-save.** Callers must call `istoric.salveaza()` explicitly. This was an intentional fix — auto-saving caused a full file write per push.

**Sidebar active state** — use `activateSidebarBtn(btn)` instead of `setProperty` alone. Qt does not re-apply QSS after `setProperty`; `unpolish/polish/update` must be called on all nav buttons.

**Dialog background scope** — dialog `setStyleSheet` must use `"QDialog { background-color: white; }"`, not `"background-color: white;"`. The latter applies `* { ... }` and overrides button hover states in the application stylesheet.

## QSS Conventions

All visual styling is in `styles/styles.qss`, loaded at startup via `resources.qrc`. Object names on widgets act as CSS IDs:
- `ModernInput` — text inputs and spinboxes
- `BtnFilter` / `BtnAdd` / `BtnDelete` / `BtnDialogSave` / `BtnDialogCancel` — button variants
- `ToggleBtn` with `setProperty("active", bool)` — the Achizitionare/Vanzare toggle

Dynamic property changes (e.g. `active`) require `style()->unpolish/polish` + `update()` to take effect — see `activateSidebarBtn()` and `TransactionDialog::updateToggleStyle()`.

## Data Flow for a Transaction

1. `onBtnSalesClicked()` opens `TransactionDialog(depozit, this)`
2. On accept: validates stock, calls `depozit.adaugaCantitate` or `scadeCantitate`
3. Builds `TranzactieProdus` with a **snapshot** of the product as payload (preserves price/name at time of transaction)
4. `istoric.adauga(t)` — memory only
5. `depozit.salveazaDate()` then `istoric.salveaza()` — two explicit saves
6. `stackedWidget->setCurrentIndex(3)` navigates to history page
7. `UpdateUI()` refreshes everything

## Adding a New Export

`ExportManager` is a static utility that works with **any** `QTableWidget`:
- `exportCSV(path, table)` — exports visible rows with UTF-8 BOM
- `exportPDF(path, table, title, infoLines)` — renders via `QTextDocument` → `QPrinter` (Landscape A4)
- `exportHTMLtoPDF(path, html, landscape)` — generic HTML → PDF used for invoices

Cell foreground colours set via `QTableWidgetItem::setForeground` are automatically preserved in the PDF output.

## Test Structure

`tests/mock_stocare.h` provides `MockStocare : IStocare` with call counters (`saveProduseCount`, `saveTranzactiiCount`) and configurable return data. All model/logic tests use it; no filesystem access needed.

- `tst_produs`, `tst_tranzactie`, `tst_warehousemanager`, `tst_tranzactiestorage`, `tst_jsonstorage` — use `QTEST_GUILESS_MAIN`
- `tst_exportmanager` — uses `QTEST_MAIN` (needs `QApplication` for `QTableWidget`)

`tst_jsonstorage` uses `QTemporaryDir` for all filesystem interaction; directories are auto-cleaned on test exit.
