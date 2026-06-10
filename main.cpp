// Kigali Smart Parking Management System - single-file implementation
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// --- Enums ---
enum class VehicleType { MOTORCYCLE, CAR, TRUCK };
enum class SlotStatus { AVAILABLE, OCCUPIED };

inline std::string vehicleTypeToString(VehicleType type) {
    switch (type) {
        case VehicleType::MOTORCYCLE: return "Motorcycle";
        case VehicleType::CAR:        return "Car";
        case VehicleType::TRUCK:      return "Truck";
        default:                      return "Unknown";
    }
}

inline std::string slotStatusToString(SlotStatus status) {
    switch (status) {
        case SlotStatus::AVAILABLE: return "Available";
        case SlotStatus::OCCUPIED:  return "Occupied";
        default:                    return "Unknown";
    }
}

// --- Utilities ---
inline bool isValidIdentifier(const std::string& value) {
    if (value.empty() || value.length() > 15) return false;
    if (!std::isalpha(static_cast<unsigned char>(value[0]))) return false;
    for (char ch : value) {
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '-')) return false;
    }
    return true;
}

inline bool isValidZoneName(const std::string& value) {
    if (value.empty() || value.length() > 30) return false;
    for (char ch : value) {
        if (!(std::isalpha(static_cast<unsigned char>(ch)) || ch == '-' || ch == ' ')) return false;
    }
    return true;
}

inline std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

inline std::string trim(const std::string& text) {
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

inline void clearInputStream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

struct TimePoint {
    int hour;
    int minute;
    int toMinutes() const { return hour * 60 + minute; }
};

inline bool parseTime(const std::string& input, TimePoint& outTime) {
    std::stringstream ss(input);
    char colon;
    if (!(ss >> outTime.hour >> colon >> outTime.minute) || colon != ':') return false;
    if (outTime.hour < 0 || outTime.hour > 23 || outTime.minute < 0 || outTime.minute > 59) return false;
    return true;
}

inline std::string formatTime(const TimePoint& time) {
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", time.hour, time.minute);
    return std::string(buffer);
}

inline int calculateBilledHours(int durationMinutes) {
    if (durationMinutes <= 0) return 1;
    return (durationMinutes + 59) / 60;
}

// --- Models ---
class ParkingSlot {
public:
    std::string slotId;
    VehicleType vehicleType;
    std::string zone;
    SlotStatus status;
    ParkingSlot() : slotId(""), vehicleType(VehicleType::MOTORCYCLE), zone(""), status(SlotStatus::AVAILABLE) {}
    ParkingSlot(const std::string& id, VehicleType type, const std::string& zoneName, SlotStatus slotStatus)
        : slotId(id), vehicleType(type), zone(zoneName), status(slotStatus) {}
};

class VehicleEntry {
public:
    std::string plateNumber;
    VehicleType vehicleType;
    TimePoint entryTime;
    std::string slotId;
    VehicleEntry() : plateNumber(""), vehicleType(VehicleType::CAR), entryTime{0, 0}, slotId("") {}
    VehicleEntry(const std::string& plate, VehicleType type, const TimePoint& time, const std::string& slot)
        : plateNumber(plate), vehicleType(type), entryTime(time), slotId(slot) {}
};

class ParkingTransaction {
public:
    std::string plateNumber;
    VehicleType vehicleType;
    std::string slotId;
    std::string zone;
    TimePoint entryTime;
    TimePoint exitTime;
    int durationMinutes;
    int billedHours;
    int ratePerHour;
    int totalFee;
    ParkingTransaction()
        : plateNumber(""), vehicleType(VehicleType::CAR), slotId(""), zone(""),
          entryTime{0, 0}, exitTime{0, 0}, durationMinutes(0),
          billedHours(0), ratePerHour(0), totalFee(0) {}
};

// --- Parking System ---
class ParkingSystem {
public:
    ParkingSystem() {
        currentPrices_[VehicleType::MOTORCYCLE] = 500;
        currentPrices_[VehicleType::CAR]        = 1000;
        currentPrices_[VehicleType::TRUCK]      = 2000;
    }

    bool addParkingSlot(const std::string& slotId, VehicleType type, const std::string& zone) {
        if (!isValidIdentifier(slotId)) {
            std::cout << "Error: Invalid Slot ID. Use 2-15 characters; start with a letter; letters, digits, hyphens only.\n";
            return false;
        }
        if (!isValidZoneName(zone)) {
            std::cout << "Error: Invalid zone name. Use letters, spaces, and hyphens only (2-30 characters).\n";
            return false;
        }
        if (slots_.find(slotId) != slots_.end()) {
            std::cout << "Error: Slot ID '" << slotId << "' already exists. Slot IDs must be unique.\n";
            return false;
        }
        slots_.emplace(slotId, ParkingSlot(slotId, type, zone, SlotStatus::AVAILABLE));
        std::cout << "Success: Parking slot '" << slotId << "' configured in zone '" << zone << "'.\n";
        return true;
    }

    void displayAllSlots() const {
        if (slots_.empty()) { std::cout << "No parking slots configured yet.\n"; return; }
        std::cout << "\n--- All Parking Slots ---\n";
        std::cout << std::left << std::setw(10) << "Slot ID" << std::setw(14) << "Vehicle Type"
                  << std::setw(18) << "Zone" << "Status\n" << std::string(52, '-') << "\n";
        for (const auto& pair : slots_) {
            const ParkingSlot& slot = pair.second;
            std::cout << std::left << std::setw(10) << slot.slotId
                      << std::setw(14) << vehicleTypeToString(slot.vehicleType)
                      << std::setw(18) << slot.zone << slotStatusToString(slot.status) << "\n";
        }
        std::cout << "Total slots: " << slots_.size() << "\n\n";
    }

    void displayAvailableSlots() const {
        int count = 0;
        std::cout << "\n--- Available Parking Slots ---\n";
        std::cout << std::left << std::setw(10) << "Slot ID" << std::setw(14) << "Vehicle Type" << "Zone\n"
                  << std::string(36, '-') << "\n";
        for (const auto& pair : slots_) {
            const ParkingSlot& slot = pair.second;
            if (slot.status == SlotStatus::AVAILABLE) {
                std::cout << std::left << std::setw(10) << slot.slotId
                          << std::setw(14) << vehicleTypeToString(slot.vehicleType) << slot.zone << "\n";
                ++count;
            }
        }
        if (count == 0) std::cout << "(No available slots)\n";
        std::cout << "Available count: " << count << "\n\n";
    }

    bool registerVehicleEntry(const std::string& plate, VehicleType type, const TimePoint& entryTime) {
        if (!isValidIdentifier(plate)) {
            std::cout << "Error: Invalid plate number. Use 2-15 characters; start with a letter; letters, digits, hyphens only.\n";
            return false;
        }
        if (activeVehicles_.find(plate) != activeVehicles_.end()) {
            std::cout << "Error: Vehicle '" << plate << "' is already parked. A vehicle cannot be parked twice.\n";
            return false;
        }
        std::string slotId;
        if (!findAvailableSlot(type, slotId)) {
            std::cout << "Error: No available " << vehicleTypeToString(type) << " parking slot. Please try again later.\n";
            return false;
        }
        if (!setSlotStatus(slotId, SlotStatus::OCCUPIED)) {
            std::cout << "Error: Failed to allocate slot '" << slotId << "'.\n";
            return false;
        }
        activeVehicles_.emplace(plate, VehicleEntry(plate, type, entryTime, slotId));
        ParkingSlot slot;
        std::cout << "Success: Vehicle '" << plate << "' entered at " << formatTime(entryTime)
                  << ". Allocated slot: " << slotId;
        if (getSlot(slotId, slot)) std::cout << " (Zone: " << slot.zone << ")";
        std::cout << ".\n";
        return true;
    }

    void displayParkedVehicles() const {
        if (activeVehicles_.empty()) { std::cout << "No vehicles currently parked.\n"; return; }
        std::cout << "\n--- Currently Parked Vehicles ---\n";
        std::cout << std::left << std::setw(12) << "Plate" << std::setw(14) << "Vehicle Type"
                  << std::setw(10) << "Slot ID" << std::setw(10) << "Entry" << "Zone\n"
                  << std::string(56, '-') << "\n";
        for (const auto& pair : activeVehicles_) {
            const VehicleEntry& entry = pair.second;
            std::string zone = "N/A";
            auto slotIt = slots_.find(entry.slotId);
            if (slotIt != slots_.end()) zone = slotIt->second.zone;
            std::cout << std::left << std::setw(12) << entry.plateNumber
                      << std::setw(14) << vehicleTypeToString(entry.vehicleType)
                      << std::setw(10) << entry.slotId << std::setw(10) << formatTime(entry.entryTime) << zone << "\n";
        }
        std::cout << "Parked count: " << activeVehicles_.size() << "\n\n";
    }

    bool updateParkingPrice(VehicleType type, int newPrice) {
        if (newPrice <= 0) { std::cout << "Error: Price must be a positive integer (RWF per hour).\n"; return false; }
        if (newPrice > 1000000) { std::cout << "Error: Price exceeds maximum allowed value (1,000,000 RWF/hour).\n"; return false; }
        int oldPrice = currentPrices_[type];
        currentPrices_[type] = newPrice;
        std::cout << "Success: " << vehicleTypeToString(type) << " rate updated from " << oldPrice
                  << " to " << newPrice << " RWF/hour.\nNote: Completed transactions in history are not affected.\n";
        return true;
    }

    void displayCurrentPrices() const {
        std::cout << "\n--- Current Parking Tariffs (RWF/hour) ---\n";
        std::cout << "Motorcycle: " << currentPrices_.at(VehicleType::MOTORCYCLE) << "\n";
        std::cout << "Car:        " << currentPrices_.at(VehicleType::CAR) << "\n";
        std::cout << "Truck:      " << currentPrices_.at(VehicleType::TRUCK) << "\n\n";
    }

    bool processVehicleExit(const std::string& plate, const TimePoint& exitTime, ParkingTransaction& outTransaction) {
        if (!isValidIdentifier(plate)) { std::cout << "Error: Invalid plate number format.\n"; return false; }
        auto vehicleIt = activeVehicles_.find(plate);
        if (vehicleIt == activeVehicles_.end()) {
            std::cout << "Error: Vehicle '" << plate << "' is not currently parked.\n";
            return false;
        }
        const VehicleEntry& entry = vehicleIt->second;
        int durationMinutes = exitTime.toMinutes() - entry.entryTime.toMinutes();
        if (durationMinutes < 0) {
            std::cout << "Error: Exit time cannot be earlier than entry time (" << formatTime(entry.entryTime) << ").\n";
            return false;
        }
        int billedHours = calculateBilledHours(durationMinutes);
        int rate = currentPrices_.at(entry.vehicleType);
        int totalFee = billedHours * rate;
        std::string zone = "N/A";
        auto slotIt = slots_.find(entry.slotId);
        if (slotIt != slots_.end()) zone = slotIt->second.zone;

        outTransaction.plateNumber = entry.plateNumber;
        outTransaction.vehicleType = entry.vehicleType;
        outTransaction.slotId = entry.slotId;
        outTransaction.zone = zone;
        outTransaction.entryTime = entry.entryTime;
        outTransaction.exitTime = exitTime;
        outTransaction.durationMinutes = durationMinutes;
        outTransaction.billedHours = billedHours;
        outTransaction.ratePerHour = rate;
        outTransaction.totalFee = totalFee;

        transactionHistory_.push_back(outTransaction);
        setSlotStatus(entry.slotId, SlotStatus::AVAILABLE);
        activeVehicles_.erase(vehicleIt);

        std::cout << "\n--- Parking Exit Receipt ---\n";
        std::cout << "Plate:           " << outTransaction.plateNumber << "\n";
        std::cout << "Vehicle Type:    " << vehicleTypeToString(outTransaction.vehicleType) << "\n";
        std::cout << "Slot / Zone:     " << outTransaction.slotId << " / " << outTransaction.zone << "\n";
        std::cout << "Entry Time:      " << formatTime(outTransaction.entryTime) << "\n";
        std::cout << "Exit Time:       " << formatTime(outTransaction.exitTime) << "\n";
        std::cout << "Duration:        " << outTransaction.durationMinutes << " minutes\n";
        std::cout << "Billed Hours:    " << outTransaction.billedHours << " hour(s) (partial hours rounded up)\n";
        std::cout << "Rate Applied:    " << outTransaction.ratePerHour << " RWF/hour\n";
        std::cout << "Total Fee:       " << outTransaction.totalFee << " RWF\n----------------------------\n";
        return true;
    }

    void displayVehicleHistory(const std::string& plate) const {
        if (!isValidIdentifier(plate)) { std::cout << "Error: Invalid plate number format.\n"; return; }
        bool found = false;
        std::cout << "\n--- Parking History for '" << plate << "' ---\n";
        for (const ParkingTransaction& tx : transactionHistory_) {
            if (tx.plateNumber == plate) {
                found = true;
                std::cout << "Slot: " << tx.slotId << " | Zone: " << tx.zone
                          << " | Entry: " << formatTime(tx.entryTime) << " | Exit: " << formatTime(tx.exitTime)
                          << " | Billed: " << tx.billedHours << "h | Rate: " << tx.ratePerHour
                          << " RWF/h | Fee: " << tx.totalFee << " RWF\n";
            }
        }
        if (!found) std::cout << "No completed parking records found for this plate.\n";
        std::cout << "\n";
    }

    void displayAllHistory() const {
        if (transactionHistory_.empty()) { std::cout << "No completed parking transactions yet.\n"; return; }
        std::cout << "\n--- All Parking Transaction History ---\n";
        int index = 1;
        for (const ParkingTransaction& tx : transactionHistory_) {
            std::cout << index++ << ". Plate: " << tx.plateNumber << " | " << vehicleTypeToString(tx.vehicleType)
                      << " | Slot: " << tx.slotId << " | " << formatTime(tx.entryTime) << " -> "
                      << formatTime(tx.exitTime) << " | Fee: " << tx.totalFee << " RWF (rate: " << tx.ratePerHour << ")\n";
        }
        std::cout << "Total transactions: " << transactionHistory_.size() << "\n\n";
    }

    void displayDailyRevenue() const {
        long long totalRevenue = 0;
        for (const ParkingTransaction& tx : transactionHistory_) totalRevenue += tx.totalFee;
        std::cout << "\n--- Daily Revenue Report ---\n";
        std::cout << "Completed transactions: " << transactionHistory_.size() << "\n";
        std::cout << "Total revenue:          " << totalRevenue << " RWF\n\n";
    }

    void loadDemoData() {
        if (!slots_.empty()) { std::cout << "Demo data skipped: slots already exist.\n"; return; }
        addParkingSlot("M-A1", VehicleType::MOTORCYCLE, "Downtown");
        addParkingSlot("M-A2", VehicleType::MOTORCYCLE, "Downtown");
        addParkingSlot("C-B1", VehicleType::CAR, "Airport");
        addParkingSlot("C-B2", VehicleType::CAR, "Airport");
        addParkingSlot("T-C1", VehicleType::TRUCK, "Industrial");
        std::cout << "Demo parking slots loaded successfully.\n";
    }

private:
    std::unordered_map<std::string, ParkingSlot> slots_;
    std::unordered_map<std::string, VehicleEntry> activeVehicles_;
    std::vector<ParkingTransaction> transactionHistory_;
    std::unordered_map<VehicleType, int> currentPrices_;

    bool findAvailableSlot(VehicleType type, std::string& outSlotId) const {
        for (const auto& pair : slots_) {
            const ParkingSlot& slot = pair.second;
            if (slot.status == SlotStatus::AVAILABLE && slot.vehicleType == type) {
                outSlotId = slot.slotId;
                return true;
            }
        }
        outSlotId.clear();
        return false;
    }

    bool setSlotStatus(const std::string& slotId, SlotStatus status) {
        auto it = slots_.find(slotId);
        if (it == slots_.end()) return false;
        it->second.status = status;
        return true;
    }

    bool getSlot(const std::string& slotId, ParkingSlot& outSlot) const {
        auto it = slots_.find(slotId);
        if (it == slots_.end()) return false;
        outSlot = it->second;
        return true;
    }
};

// --- Menu / UI ---
void displayMenu() {
    std::cout << "\n========================================\n";
    std::cout << "  KIGALI SMART PARKING MANAGEMENT SYSTEM\n";
    std::cout << "========================================\n";
    std::cout << " 1. Add Parking Slot (Configure)\n";
    std::cout << " 2. View All Parking Slots\n";
    std::cout << " 3. View Available Slots\n";
    std::cout << " 4. Register Vehicle Entry\n";
    std::cout << " 5. Process Vehicle Exit\n";
    std::cout << " 6. View Parked Vehicles\n";
    std::cout << " 7. Update Parking Price\n";
    std::cout << " 8. View Current Tariffs\n";
    std::cout << " 9. Vehicle Parking History\n";
    std::cout << "10. View All Transaction History\n";
    std::cout << "11. Daily Revenue Report\n";
    std::cout << "12. Load Demo Data (Test Slots)\n";
    std::cout << " 0. Exit Program\n";
    std::cout << "========================================\nEnter your choice: ";
}

bool promptVehicleType(VehicleType& outType) {
    std::cout << "Select vehicle type:\n  1. Motorcycle\n  2. Car\n  3. Truck\nChoice (1-3): ";
    int typeChoice = 0;
    if (!(std::cin >> typeChoice)) { clearInputStream(); std::cout << "Invalid input! Please enter 1, 2, or 3.\n"; return false; }
    clearInputStream();
    switch (typeChoice) {
        case 1: outType = VehicleType::MOTORCYCLE; return true;
        case 2: outType = VehicleType::CAR;        return true;
        case 3: outType = VehicleType::TRUCK;      return true;
        default: std::cout << "Invalid choice! Please enter 1, 2, or 3.\n"; return false;
    }
}

bool promptTime(const std::string& label, TimePoint& outTime) {
    std::string timeStr = trim(readLine(label));
    if (!parseTime(timeStr, outTime)) {
        std::cout << "Invalid time format! Use HH:MM (24-hour), e.g. 09:30 or 14:15.\n";
        return false;
    }
    return true;
}

void handleAddSlot(ParkingSystem& system) {
    std::string slotId = trim(readLine("Enter Slot ID (e.g. C-A1): "));
    if (slotId.empty()) { std::cout << "Error: Slot ID cannot be empty.\n"; return; }
    VehicleType type;
    if (!promptVehicleType(type)) return;
    std::string zone = trim(readLine("Enter Zone name (e.g. Downtown): "));
    if (zone.empty()) { std::cout << "Error: Zone name cannot be empty.\n"; return; }
    system.addParkingSlot(slotId, type, zone);
}

void handleVehicleEntry(ParkingSystem& system) {
    std::string plate = trim(readLine("Enter vehicle plate number: "));
    if (plate.empty()) { std::cout << "Error: Plate number cannot be empty.\n"; return; }
    VehicleType type;
    if (!promptVehicleType(type)) return;
    TimePoint entryTime;
    if (!promptTime("Enter entry time (HH:MM): ", entryTime)) return;
    system.registerVehicleEntry(plate, type, entryTime);
}

void handleVehicleExit(ParkingSystem& system) {
    std::string plate = trim(readLine("Enter vehicle plate number: "));
    if (plate.empty()) { std::cout << "Error: Plate number cannot be empty.\n"; return; }
    TimePoint exitTime;
    if (!promptTime("Enter exit time (HH:MM): ", exitTime)) return;
    ParkingTransaction receipt;
    system.processVehicleExit(plate, exitTime, receipt);
}

void handlePriceUpdate(ParkingSystem& system) {
    VehicleType type;
    if (!promptVehicleType(type)) return;
    std::cout << "Enter new hourly price (RWF): ";
    int newPrice = 0;
    if (!(std::cin >> newPrice)) { clearInputStream(); std::cout << "Invalid input! Please enter a positive integer.\n"; return; }
    clearInputStream();
    system.updateParkingPrice(type, newPrice);
}

void handleVehicleHistory(ParkingSystem& system) {
    std::string plate = trim(readLine("Enter plate number to search: "));
    if (plate.empty()) { std::cout << "Error: Plate number cannot be empty.\n"; return; }
    system.displayVehicleHistory(plate);
}

int main() {
    ParkingSystem system;
    std::cout << "Welcome to Kigali Smart Parking Management System!\n";
    std::cout << "Tip: Choose option 12 to load demo slots for quick testing.\n";
    int choice = -1;
    while (true) {
        displayMenu();
        if (!(std::cin >> choice)) {
            clearInputStream();
            std::cout << "Invalid input! Please enter a valid choice (0-12).\n";
            continue;
        }
        clearInputStream();
        switch (choice) {
            case 1:  handleAddSlot(system); break;
            case 2:  system.displayAllSlots(); break;
            case 3:  system.displayAvailableSlots(); break;
            case 4:  handleVehicleEntry(system); break;
            case 5:  handleVehicleExit(system); break;
            case 6:  system.displayParkedVehicles(); break;
            case 7:  handlePriceUpdate(system); break;
            case 8:  system.displayCurrentPrices(); break;
            case 9:  handleVehicleHistory(system); break;
            case 10: system.displayAllHistory(); break;
            case 11: system.displayDailyRevenue(); break;
            case 12: system.loadDemoData(); break;
            case 0:
                std::cout << "Thank you for using Kigali Smart Parking System. Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid choice! Please enter a number between 0 and 12.\n";
                break;
        }
    }
    return 0;
}
