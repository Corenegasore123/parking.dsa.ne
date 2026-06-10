# Kigali Smart Parking Management System

An in-memory **C++** console application for managing public parking in Kigali City. The system demonstrates **Data Structures & Algorithms (DSA)** and **Object-Oriented Programming (OOP)** without using a database.

## Features

| Task | Description |
|------|-------------|
| **Task 1** | Configure parking slots (ID, vehicle type, zone, status) |
| **Task 2** | Register vehicle entry with automatic slot allocation |
| **Task 3** | Calculate parking fees with ceiling-hour billing and live price updates |
| **Task 4** | Process vehicle exit, release slots, and store transactions |
| **Task 5** | Operational reports: available slots, parked vehicles, history, daily revenue |

## Default Parking Tariffs (RWF/hour)

| Vehicle Type | Rate |
|--------------|------|
| Motorcycle   | 500  |
| Car          | 1,000 |
| Truck        | 2,000 |

> Truck rate is an extension (not specified in the brief). Partial hours are always rounded **up** (15 min → 1 hour, 1h 20m → 2 hours).

## Compile and Run

### Windows (MinGW / g++)

```bash
g++ -std=c++11 -o parking.exe main.cpp
parking.exe
```

### Linux / macOS

```bash
g++ -std=c++11 -o parking main.cpp
./parking
```

## Menu Options

| # | Action |
|---|--------|
| 1 | Add parking slot |
| 2 | View all slots |
| 3 | View available slots |
| 4 | Register vehicle entry |
| 5 | Process vehicle exit (fee calculated here) |
| 6 | View currently parked vehicles |
| 7 | Update parking price (does not change history) |
| 8 | View current tariffs |
| 9 | Vehicle parking history (by plate) |
| 10 | All transaction history |
| 11 | Daily revenue report |
| 12 | Load demo test slots |
| 0 | Exit |

## Quick Test Walkthrough

1. Run the program and choose **12** to load demo slots.
2. Choose **3** to see available slots.
3. Choose **4** — register plate `RAB-123`, type **Car**, entry time `08:00`.
4. Choose **6** to confirm the vehicle is parked.
5. Choose **5** — exit plate `RAB-123` at `09:20` → billed **2 hours** × 1,000 = **2,000 RWF**.
6. Choose **7** — update Car price to `1500`, then repeat entry/exit for another car.
7. Choose **9** or **10** to verify history keeps the old rate for the first transaction.
8. Choose **11** for daily revenue.

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│              Console Menu (main.cpp)                     │
└─────────────────────────┬───────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────┐
│              ParkingSystem (Facade / Controller)         │
├──────────────┬──────────────┬──────────────┬────────────┤
│ Slot Manager │Vehicle Mgr   │ Price Mgr    │ Report Mgr  │
│ unordered_map│unordered_map │unordered_map │ vector      │
│ <slotId,Slot>│<plate,Entry> │<type,rate>   │<Transaction>│
└──────────────┴──────────────┴──────────────┴────────────┘
```

### Data Structure Justification

| Structure | Key | Purpose | Why |
|-----------|-----|---------|-----|
| `unordered_map<string, ParkingSlot>` | Slot ID | Slot CRUD & status updates | O(1) average lookup by unique slot ID |
| `unordered_map<string, VehicleEntry>` | Plate number | Active parking records | O(1) duplicate-entry check & exit lookup |
| `unordered_map<VehicleType, int>` | Vehicle type | Current hourly tariffs | O(1) price read at exit time |
| `vector<ParkingTransaction>` | — | Completed session history | O(1) append on exit; sequential traversal for reports |

## Input Validation

- Menu choices: invalid/non-numeric input is rejected without crashing.
- Slot IDs & plate numbers: 2–15 chars, start with a letter, alphanumeric + hyphens.
- Zone names: letters, spaces, hyphens only (2–30 chars).
- Times: `HH:MM` 24-hour format; exit cannot be before entry.
- Prices: positive integers only; history records store the rate used at exit.

## Project Structure

```
dsa/
├── main.cpp                  # Full application (models + logic + menu)
├── README.md                 # Usage and documentation
└── diagrams/
    └── all-diagrams.html     # Architecture, DFD, class & sequence diagrams
```

Open `diagrams/all-diagrams.html` in a browser to view visual diagrams.
