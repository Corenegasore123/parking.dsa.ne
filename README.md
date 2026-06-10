# Kigali Smart Parking Management System

Single-file C++ console app for Dev C++ (C++11). No database, no internet required.

## Compile (Dev C++)

1. Open Dev C++ → **File → New → Project → Console Application (C++)**
2. Replace project source with `main.cpp` (or add `main.cpp` to project)
3. **Execute → Compile & Run** (F11)

Or command line:

```bash
g++ -std=c++11 -o parking.exe main.cpp
parking.exe
```

## Default Tariffs (RWF/hour)

| Vehicle   | Default Rate | Notes |
|-----------|-------------|-------|
| Motorcycle | 500        | Per assignment brief |
| Car        | 1,000      | Per assignment brief |
| Truck      | 2,000      | Task 2 requires Truck; same billing rules as other types |

Trucks use **Truck-only slots** (e.g. demo slot `T-C1`). Fee = billed hours × current truck rate at exit.

## Important Validation Rules

- **Invalid menu input (e.g. `09`, letters) does NOT exit** — shows error and re-prompts
- **Only `0` exits** the program (not `09`, `00`, etc.)
- **Future dates/times are NOT allowed** for entry or exit
- Date/time format: `DD-MM-YYYY HH:MM` (e.g. `10-06-2026 14:30`)

## Menu

| # | Action |
|---|--------|
| 1 | Add parking slot |
| 2 | View all slots |
| 3 | View available slots |
| 4 | Register vehicle entry |
| 5 | Process vehicle exit |
| 6 | View parked vehicles |
| 7 | Update parking price |
| 8 | View tariffs |
| 9 | Vehicle history |
| 10 | All transaction history |
| 11 | Daily revenue |
| 12 | Load demo data |
| 0 | Exit |

## Quick Test

1. Run program → option **12** (loads Motorcycle, Car, **Truck** slots)
2. Option **4** → plate `RAB-123`, type `2` (Car), entry `10-06-2026 08:00` (use today, not future)
3. Option **5** → same plate, exit `10-06-2026 09:20` → 2 hours × 1000 = **2000 RWF**
4. Type `09` at menu → error message, program **continues** (does not exit)

## Project Files

```
dsa/
├── main.cpp              # Complete application
├── README.md
└── diagrams/
    └── all-diagrams.html
```
