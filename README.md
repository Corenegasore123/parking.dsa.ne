# Kigali Smart Parking Management System

A console-based **Smart Parking Management System** for Kigali City, built in **C++** using **in-memory data structures only** (no database). The program helps parking attendants manage slots, vehicle entry/exit, fee calculation, and operational reports in real time.

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Default Parking Tariffs](#default-parking-tariffs)
4. [Data Structures Used](#data-structures-used)
5. [How to Compile and Run](#how-to-compile-and-run)
6. [Menu Options](#menu-options)
7. [Input Formats and Validation](#input-formats-and-validation)
8. [Quick Test Walkthrough](#quick-test-walkthrough)
9. [Project Structure](#project-structure)
10. [Design Notes](#design-notes)

---

## Overview

Kigali City is replacing manual paper-ticket parking with a digital system that:

- Configures and tracks parking slots by vehicle type and zone
- Registers vehicle entry and auto-allocates suitable slots
- Calculates parking fees at exit using hourly tariffs
- Stores transaction history and generates revenue reports

---

## Features

| Task | Description |
|------|-------------|
| **Task 1** | Configure parking slots (Slot ID, vehicle type, zone, status) |
| **Task 2** | Register vehicle entry with automatic slot allocation |
| **Task 3** | Calculate fees with ceiling-hour billing; update prices at runtime |
| **Task 4** | Process vehicle exit, release slot, print receipt, save transaction |
| **Task 5** | Reports: available slots, parked vehicles, history, daily revenue |

### Billing Rules

- Fees are calculated **only when a vehicle exits**
- Partial hours are rounded **up** (15 min → 1 hour, 1h 20m → 2 hours)
- Price updates affect **future exits only** — completed history is unchanged

---

## Default Parking Tariffs

| Vehicle Type | Rate (RWF/hour) | Notes |
|--------------|-----------------|-------|
| Motorcycle   | 500             | As per assignment brief |
| Car          | 1,000           | As per assignment brief |
| Truck        | 2,000           | Required by Task 2; uses truck-only slots |

**Truck handling:** Trucks can only park in slots configured for `Truck`. The demo data includes slot `T-C1` in the Industrial zone. Billing uses the same ceiling-hour rule as other vehicle types.

---

## Data Structures Used

| Structure | Key | Purpose |
|-----------|-----|---------|
| `unordered_map<string, ParkingSlot>` | Slot ID | Fast slot lookup, insert, and status update |
| `unordered_map<string, VehicleEntry>` | Plate number | Prevent duplicate active parking; fast exit lookup |
| `unordered_map<VehicleType, int>` | Vehicle type | Store and update current hourly tariffs |
| `vector<ParkingTransaction>` | — | Append completed sessions; traverse for reports |

---

## How to Compile and Run

### Option A — Dev C++ (Recommended for assessment)

1. Open **Dev C++**
2. Go to **File → New → Project → Console Application (C++)**
3. Save the project, then replace the generated `.cpp` content with `main.cpp`
4. Press **F11** (Compile & Run)

### Option B — Command Line (g++)

**Windows:**

```bash
g++ -std=c++11 -o parking.exe main.cpp
parking.exe
```

**Linux / macOS:**

```bash
g++ -std=c++11 -o parking main.cpp
./parking
```

> **Note:** No internet connection is required. No external libraries are needed.

---

## Menu Options

```
 1. Add Parking Slot
 2. View All Parking Slots
 3. View Available Slots
 4. Register Vehicle Entry
 5. Process Vehicle Exit
 6. View Parked Vehicles
 7. Update Parking Price
 8. View Current Tariffs
 9. Vehicle Parking History
10. All Transaction History
11. Daily Revenue Report
12. Load Demo Data
 0. Exit
```

| Option | What it does |
|--------|--------------|
| 1 | Add a new slot (unique ID, vehicle type, zone) |
| 2 | List all slots with status (Available / Occupied) |
| 3 | List only free slots |
| 4 | Register a vehicle; system assigns a matching free slot |
| 5 | Exit a vehicle; calculates fee and prints receipt |
| 6 | Show all currently parked vehicles |
| 7 | Change hourly rate for Motorcycle, Car, or Truck |
| 8 | Display current tariffs |
| 9 | Show parking history for one plate number |
| 10 | Show all completed transactions |
| 11 | Show total revenue from completed sessions |
| 12 | Load sample slots for testing |
| 0 | Exit the program safely |

---

## Input Formats and Validation

The program **never crashes** on bad input. Invalid entries show an error and the menu is shown again.

### Menu

- Only whole numbers **0–12** are accepted
- `09`, `01`, letters, symbols, and decimals are **rejected**
- Only **`0`** exits the program

### Slot ID

- 2–15 characters
- Must start with a letter
- Letters, digits, and hyphens only (e.g. `C-A1`, `M-A2`)

### Rwandan Plate Number

Format: **`RA` + letter + 3 digits + letter**

| Part | Rule | Example |
|------|------|---------|
| Prefix | Must be `RA` | RA |
| Letter | One letter (A–Z) | B |
| Digits | Exactly 3 digits (0–9) | 123 |
| Letter | One ending letter (A–Z) | A |

**Valid examples:** `RAB123A`, `RA B 123 A`, `RA-B-123-A` (spaces/hyphens optional)

**Invalid examples:** `RAB12A`, `RC12345B`, `RAB123`, `rab123a` is accepted and stored as `RAB123A`

### Zone Name

- 2–30 characters
- Letters, spaces, and hyphens only (e.g. `Downtown`, `New York`)

### Date and Time (Entry / Exit)

- Format: **`DD-MM-YYYY HH:MM`** (24-hour clock)
- Example: `10-06-2026 14:30`
- **Future dates and times are not allowed**
- Exit cannot be earlier than entry

### Parking Price

- Must be a positive whole number (RWF per hour)
- Zero, negative values, and non-numeric input are rejected

### Vehicle Type

- `1` = Motorcycle
- `2` = Car
- `3` = Truck

---

## Quick Test Walkthrough

Follow these steps to verify all main features:

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Run program, choose **12** | Demo slots loaded (Motorcycle, Car, Truck) |
| 2 | Choose **3** | Lists available slots |
| 3 | Choose **4** → plate `RAB123A`, type `2`, entry `10-06-2026 08:00`* | Vehicle parked, slot assigned |
| 4 | Choose **6** | `RAB123A` appears in parked list |
| 5 | Choose **5** → plate `RAB123A`, exit `10-06-2026 09:20`* | Receipt: 2 hours × 1,000 = **2,000 RWF** |
| 6 | Choose **7** → type `2`, new price `1500` | Car rate updated |
| 7 | Choose **9** → plate `RAB123A` | History shows old rate (1,000) for first exit |
| 8 | Choose **11** | Daily revenue shows 2,000 RWF |
| 9 | At menu, type `09` | Error shown; program **does not exit** |
| 10 | Choose **0** | Program exits with goodbye message |

\* Use **today's date** and a time **not in the future** when testing.

---

## Project Structure

```
dsa/
├── main.cpp          # Complete application (all logic in one file)
├── README.md         # This file
├── .gitignore        # Git ignore rules for build artifacts
└── diagrams/         # Optional: architecture diagrams (HTML)
    └── all-diagrams.html
```

---

## Design Notes

### Object-Oriented Programming

- **Encapsulation:** `ParkingSystem` manages all data and operations
- **Abstraction:** Tariff rules hidden behind `TariffPolicy` interface
- **Inheritance & Polymorphism:** `MotorcycleTariff`, `CarTariff`, `TruckTariff` extend `TariffPolicy`

### Error Handling

- `try-catch` blocks protect the main loop and menu handlers
- Invalid `cin` input is cleared and the user is re-prompted
- Empty strings and whitespace-only input are rejected

### System Stability

The program runs continuously until the user explicitly selects **0 (Exit)**. Invalid input never terminates the application.

---

## License

Academic project for Data Structures and Algorithms (DSA) coursework.
