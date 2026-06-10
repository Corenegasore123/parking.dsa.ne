/*
 * Kigali Smart Parking Management System
 * Dev C++ / C++11 single-file version
 * Compile: g++ -std=c++11 -o parking main.cpp
 */

#include <algorithm>      // std:: algorithms if needed
#include <cctype>         // isalpha, isdigit, isalnum
#include <cstdio>           // snprintf for formatting
#include <ctime>            // time, localtime for date validation
#include <iomanip>          // setw, left for table output
#include <iostream>         // cin, cout
#include <limits>           // numeric_limits for stream ignore
#include <sstream>          // stringstream for parsing
#include <stdexcept>        // runtime_error for exceptions
#include <string>           // string type
#include <unordered_map>    // hash maps for O(1) lookup
#include <vector>           // vector for transaction history

// Enumeration for supported vehicle categories
enum class VehicleType { MOTORCYCLE, CAR, TRUCK };

// Enumeration for parking slot occupancy state
enum class SlotStatus { AVAILABLE, OCCUPIED };

// Converts VehicleType enum to display text
std::string vehicleTypeToString(VehicleType type) {
    switch (type) {                                    // branch on type
        case VehicleType::MOTORCYCLE: return "Motorcycle"; // motorcycle label
        case VehicleType::CAR:        return "Car";        // car label
        case VehicleType::TRUCK:      return "Truck";      // truck label
        default:                      return "Unknown";    // safety fallback
    }
}

// Converts SlotStatus enum to display text
std::string slotStatusToString(SlotStatus status) {
    switch (status) {                                  // branch on status
        case SlotStatus::AVAILABLE: return "Available"; // free slot
        case SlotStatus::OCCUPIED:  return "Occupied";  // used slot
        default:                    return "Unknown";   // safety fallback
    }
}

// Clears cin error state and discards leftover input characters
void clearInputStream() {
    std::cin.clear();                                  // reset fail bits
    std::cin.ignore(10000, '\n');                      // discard bad input line
}

// Trims leading and trailing whitespace from a string
std::string trim(const std::string& text) {
    size_t start = 0;                                  // first non-space index
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;                                       // skip leading spaces
    }
    size_t end = text.size();                          // end index
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;                                         // skip trailing spaces
    }
    return text.substr(start, end - start);            // return trimmed substring
}

// Reads one full input line after showing a prompt
std::string readLine(const std::string& prompt) {
    std::cout << prompt;                               // show prompt text
    std::string line;                                  // buffer for input
    std::getline(std::cin, line);                      // read until newline
    return line;                                       // return raw line
}

// Returns true when text is empty or only whitespace
bool isEmptyOrWhitespace(const std::string& value) {
    return trim(value).empty();                        // empty after trim
}

// Validates slot IDs and plate numbers: letter first, alphanumeric + hyphen, 2-15 chars
bool isValidIdentifier(const std::string& value) {
    std::string v = trim(value);                       // normalize input
    if (v.empty() || v.length() > 15) return false;    // length rule
    if (!std::isalpha(static_cast<unsigned char>(v[0]))) return false; // must start with letter
    for (size_t i = 0; i < v.size(); ++i) {            // scan each character
        char ch = v[i];                                // current char
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '-')) return false;
    }
    return true;                                       // valid identifier
}

// Validates zone names: letters, spaces, hyphens, 2-30 chars
bool isValidZoneName(const std::string& value) {
    std::string v = trim(value);                       // normalize input
    if (v.empty() || v.length() > 30) return false;    // length rule
    for (size_t i = 0; i < v.size(); ++i) {            // scan each character
        char ch = v[i];                                // current char
        if (!(std::isalpha(static_cast<unsigned char>(ch)) || ch == '-' || ch == ' ')) return false;
    }
    return true;                                       // valid zone
}

// Checks whether a year is a leap year
bool isLeapYear(int year) {
    if (year % 400 == 0) return true;                  // divisible by 400
    if (year % 100 == 0) return false;                 // century rule
    return (year % 4 == 0);                            // divisible by 4
}

// Returns number of days in a given month/year
int daysInMonth(int month, int year) {
    static const int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31}; // month lengths
    if (month < 1 || month > 12) return 0;             // invalid month
    if (month == 2 && isLeapYear(year)) return 29;    // February leap year
    return days[month];                                // normal month length
}

// Stores a full calendar date and clock time
struct DateTime {
    int day;                                           // day of month 1-31
    int month;                                         // month 1-12
    int year;                                          // four-digit year
    int hour;                                          // hour 0-23
    int minute;                                        // minute 0-59

    DateTime() : day(1), month(1), year(2000), hour(0), minute(0) {} // default ctor

    // Converts this DateTime to comparable seconds since Unix epoch (local time)
    bool toTimeT(std::time_t& out) const {
        std::tm tmVal;                                 // broken-down time struct
        tmVal.tm_sec  = 0;                             // seconds set to zero
        tmVal.tm_min  = minute;                        // copy minute
        tmVal.tm_hour = hour;                          // copy hour
        tmVal.tm_mday = day;                           // copy day
        tmVal.tm_mon  = month - 1;                     // tm month is 0-based
        tmVal.tm_year = year - 1900;                   // tm year offset
        tmVal.tm_isdst = -1;                           // let library detect DST
        out = std::mktime(&tmVal);                     // convert to time_t
        return out != static_cast<std::time_t>(-1);    // false if conversion failed
    }

    // Builds DateTime from current system clock
    static DateTime now() {
        std::time_t t = std::time(NULL);               // current timestamp
        std::tm* lt = std::localtime(&t);              // local time breakdown
        DateTime dt;                                   // output object
        dt.day = lt->tm_mday;                          // current day
        dt.month = lt->tm_mon + 1;                     // current month
        dt.year = lt->tm_year + 1900;                  // current year
        dt.hour = lt->tm_hour;                         // current hour
        dt.minute = lt->tm_min;                        // current minute
        return dt;                                     // return now
    }
};

// Formats DateTime as DD-MM-YYYY HH:MM
std::string formatDateTime(const DateTime& dt) {
    char buffer[20];                                   // format buffer
    snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d %02d:%02d",
             dt.day, dt.month, dt.year, dt.hour, dt.minute); // build text
    return std::string(buffer);                        // return as string
}

// Validates calendar fields and rejects impossible dates
bool isValidCalendarDate(int day, int month, int year) {
    if (year < 2000 || year > 2100) return false;      // reasonable year range
    if (month < 1 || month > 12) return false;       // month range
    int dim = daysInMonth(month, year);                // days in month
    if (day < 1 || day > dim) return false;            // day range
    return true;                                       // date is valid
}

// Returns true if dt is later than current system date/time (future not allowed)
bool isFutureDateTime(const DateTime& dt) {
    std::time_t inputTime;                             // input as time_t
    if (!dt.toTimeT(inputTime)) return true;           // invalid counts as reject
    std::time_t nowTime = std::time(NULL);             // current time
    return inputTime > nowTime;                        // future if greater than now
}

// Parses DD-MM-YYYY HH:MM string into DateTime
bool parseDateTime(const std::string& input, DateTime& out) {
    std::stringstream ss(input);                       // stream for parsing
    char d1, d2, c1, c2;                               // separators
    if (!(ss >> out.day >> d1 >> out.month >> d2 >> out.year >> out.hour >> c2 >> out.minute)) {
        return false;                                  // parse failed
    }
    if (d1 != '-' || d2 != '-' || c2 != ':') return false; // bad separators
    if (out.hour < 0 || out.hour > 23 || out.minute < 0 || out.minute > 59) return false;
    if (!isValidCalendarDate(out.day, out.month, out.year)) return false; // bad date
    return true;                                       // parsed successfully
}

// Computes parking duration in minutes between entry and exit
long long durationMinutes(const DateTime& entry, const DateTime& exitDt) {
    std::time_t tEntry, tExit;                         // epoch times
    if (!entry.toTimeT(tEntry) || !exitDt.toTimeT(tExit)) return -1; // conversion error
    long long diff = static_cast<long long>(tExit - tEntry); // seconds difference
    return diff / 60;                                  // convert to minutes
}

// Ceiling billing: partial hours count as full hours
int calculateBilledHours(long long durationMinutes) {
    if (durationMinutes <= 0) return 1;                // minimum one hour
    return static_cast<int>((durationMinutes + 59) / 60); // round up
}

// Parses strict positive integer text without leading zeros (except "0" alone)
bool parseStrictInteger(const std::string& text, int& outValue, int minVal, int maxVal) {
    std::string v = trim(text);                        // trim spaces
    if (v.empty()) return false;                       // empty invalid
    if (v.size() > 1 && v[0] == '0') return false;     // reject 09, 01, etc.
    for (size_t i = 0; i < v.size(); ++i) {            // every char must be digit
        if (!std::isdigit(static_cast<unsigned char>(v[i]))) return false;
    }
    std::stringstream ss(v);                           // parse digits
    int value = 0;                                     // parsed value
    if (!(ss >> value)) return false;                  // parse failed
    if (!ss.eof()) return false;                       // reject trailing junk
    if (value < minVal || value > maxVal) return false; // range check
    outValue = value;                                  // store result
    return true;                                       // success
}

// --- OOP: abstract tariff policy (polymorphism for default rates) ---
class TariffPolicy {
public:
    virtual ~TariffPolicy() {}                         // virtual destructor
    virtual VehicleType getType() const = 0;           // pure virtual type
    virtual int getDefaultRate() const = 0;            // pure virtual default RWF/hour
    virtual std::string getName() const = 0;           // pure virtual label
};

// Motorcycle tariff policy: 500 RWF/hour default
class MotorcycleTariff : public TariffPolicy {
public:
    VehicleType getType() const { return VehicleType::MOTORCYCLE; } // motorcycle type
    int getDefaultRate() const { return 500; }         // assignment default rate
    std::string getName() const { return "Motorcycle"; } // display name
};

// Car tariff policy: 1000 RWF/hour default
class CarTariff : public TariffPolicy {
public:
    VehicleType getType() const { return VehicleType::CAR; } // car type
    int getDefaultRate() const { return 1000; }        // assignment default rate
    std::string getName() const { return "Car"; }      // display name
};

// Truck tariff policy: 2000 RWF/hour default (Task 2 requires Truck support)
class TruckTariff : public TariffPolicy {
public:
    VehicleType getType() const { return VehicleType::TRUCK; } // truck type
    int getDefaultRate() const { return 2000; }        // extension default for trucks
    std::string getName() const { return "Truck"; }    // display name
};

// Parking slot model
class ParkingSlot {
public:
    std::string slotId;                                // unique slot identifier
    VehicleType vehicleType;                           // supported vehicle type
    std::string zone;                                  // location zone name
    SlotStatus status;                                 // available or occupied

    ParkingSlot()
        : slotId(""), vehicleType(VehicleType::MOTORCYCLE), zone(""), status(SlotStatus::AVAILABLE) {}

    ParkingSlot(const std::string& id, VehicleType type, const std::string& zoneName, SlotStatus slotStatus)
        : slotId(id), vehicleType(type), zone(zoneName), status(slotStatus) {}
};

// Active parked vehicle record
class VehicleEntry {
public:
    std::string plateNumber;                           // unique plate while parked
    VehicleType vehicleType;                           // vehicle category
    DateTime entryDateTime;                            // entry date and time
    std::string slotId;                                // allocated slot id

    VehicleEntry()
        : plateNumber(""), vehicleType(VehicleType::CAR), slotId("") {}

    VehicleEntry(const std::string& plate, VehicleType type, const DateTime& dt, const std::string& slot)
        : plateNumber(plate), vehicleType(type), entryDateTime(dt), slotId(slot) {}
};

// Completed parking transaction stored in history
class ParkingTransaction {
public:
    std::string plateNumber;                           // vehicle plate
    VehicleType vehicleType;                           // vehicle category
    std::string slotId;                                // used slot
    std::string zone;                                  // slot zone
    DateTime entryDateTime;                            // entry timestamp
    DateTime exitDateTime;                             // exit timestamp
    long long durationMinutes;                         // actual minutes parked
    int billedHours;                                   // hours charged
    int ratePerHour;                                   // rate used at exit
    int totalFee;                                      // total RWF fee

    ParkingTransaction()
        : plateNumber(""), vehicleType(VehicleType::CAR), slotId(""), zone(""),
          durationMinutes(0), billedHours(0), ratePerHour(0), totalFee(0) {}
};

// Core parking system using in-memory data structures only
class ParkingSystem {
public:
    // Constructor loads default tariffs via polymorphic TariffPolicy objects
    ParkingSystem() {
        try {                                          // protect initialization
            tariffPolicies_.push_back(new MotorcycleTariff()); // motorcycle policy
            tariffPolicies_.push_back(new CarTariff());        // car policy
            tariffPolicies_.push_back(new TruckTariff());      // truck policy
            for (size_t i = 0; i < tariffPolicies_.size(); ++i) { // foreach policy
                TariffPolicy* policy = tariffPolicies_[i];      // current policy
                currentPrices_[policy->getType()] = policy->getDefaultRate(); // set default rate
            }
        } catch (const std::exception& ex) {           // catch init errors
            std::cout << "Error initializing tariffs: " << ex.what() << "\n";
        }
    }

    // Destructor frees polymorphic tariff policy objects
    ~ParkingSystem() {
        for (size_t i = 0; i < tariffPolicies_.size(); ++i) { // foreach policy
            delete tariffPolicies_[i];                 // free heap object
        }
        tariffPolicies_.clear();                       // clear pointer vector
    }

    // Task 1: add a uniquely identified parking slot
    bool addParkingSlot(const std::string& slotId, VehicleType type, const std::string& zone) {
        if (isEmptyOrWhitespace(slotId)) {               // empty slot id check
            std::cout << "Error: Slot ID cannot be empty.\n";
            return false;                                // reject empty
        }
        if (!isValidIdentifier(slotId)) {                // format validation
            std::cout << "Error: Invalid Slot ID. Start with a letter; use letters, digits, hyphens (2-15 chars).\n";
            return false;                                // reject bad format
        }
        if (isEmptyOrWhitespace(zone)) {               // empty zone check
            std::cout << "Error: Zone name cannot be empty.\n";
            return false;                                // reject empty zone
        }
        if (!isValidZoneName(zone)) {                    // zone format validation
            std::cout << "Error: Invalid zone name. Use letters, spaces, hyphens only (2-30 chars).\n";
            return false;                                // reject bad zone
        }
        if (slots_.find(slotId) != slots_.end()) {      // duplicate slot id check
            std::cout << "Error: Slot ID '" << slotId << "' already exists.\n";
            return false;                                // reject duplicate
        }
        slots_.emplace(slotId, ParkingSlot(slotId, type, zone, SlotStatus::AVAILABLE)); // insert slot
        std::cout << "Success: Slot '" << slotId << "' added for "
                  << vehicleTypeToString(type) << " in zone '" << zone << "'.\n";
        return true;                                     // success
    }

    // Task 1 report: show all slots
    void displayAllSlots() const {
        if (slots_.empty()) {                            // no slots configured
            std::cout << "No parking slots configured yet.\n";
            return;                                      // nothing to show
        }
        std::cout << "\n--- All Parking Slots ---\n";
        std::cout << std::left << std::setw(10) << "Slot ID" << std::setw(14) << "Vehicle Type"
                  << std::setw(18) << "Zone" << "Status\n" << std::string(52, '-') << "\n";
        for (std::unordered_map<std::string, ParkingSlot>::const_iterator it = slots_.begin();
             it != slots_.end(); ++it) {                // traverse slot map
            const ParkingSlot& slot = it->second;        // current slot
            std::cout << std::left << std::setw(10) << slot.slotId
                      << std::setw(14) << vehicleTypeToString(slot.vehicleType)
                      << std::setw(18) << slot.zone << slotStatusToString(slot.status) << "\n";
        }
        std::cout << "Total slots: " << slots_.size() << "\n\n";
    }

    // Task 5 report: available slots only
    void displayAvailableSlots() const {
        int count = 0;                                   // available counter
        std::cout << "\n--- Available Parking Slots ---\n";
        std::cout << std::left << std::setw(10) << "Slot ID" << std::setw(14) << "Vehicle Type"
                  << "Zone\n" << std::string(36, '-') << "\n";
        for (std::unordered_map<std::string, ParkingSlot>::const_iterator it = slots_.begin();
             it != slots_.end(); ++it) {                // traverse slots
            const ParkingSlot& slot = it->second;        // current slot
            if (slot.status == SlotStatus::AVAILABLE) {  // only available
                std::cout << std::left << std::setw(10) << slot.slotId
                          << std::setw(14) << vehicleTypeToString(slot.vehicleType)
                          << slot.zone << "\n";
                ++count;                                 // increment count
            }
        }
        if (count == 0) std::cout << "(No available slots)\n";
        std::cout << "Available count: " << count << "\n\n";
    }

    // Task 2: register vehicle entry with slot allocation
    bool registerVehicleEntry(const std::string& plate, VehicleType type, const DateTime& entryDt) {
        if (isEmptyOrWhitespace(plate)) {                // empty plate check
            std::cout << "Error: Plate number cannot be empty.\n";
            return false;                                // reject empty
        }
        if (!isValidIdentifier(plate)) {                 // plate format validation
            std::cout << "Error: Invalid plate number format.\n";
            return false;                                // reject bad format
        }
        if (activeVehicles_.find(plate) != activeVehicles_.end()) { // duplicate active plate
            std::cout << "Error: Vehicle '" << plate << "' is already parked.\n";
            return false;                                // prevent double parking
        }
        if (isFutureDateTime(entryDt)) {                 // future date/time not allowed
            std::cout << "Error: Future entry date/time is not allowed.\n";
            return false;                                // reject future entry
        }
        std::string slotId;                              // slot to allocate
        if (!findAvailableSlot(type, slotId)) {          // find matching free slot
            std::cout << "Error: No available " << vehicleTypeToString(type)
                      << " slot. Please try again later.\n";
            return false;                                // graceful no-slot message
        }
        if (!setSlotStatus(slotId, SlotStatus::OCCUPIED)) { // mark slot occupied
            std::cout << "Error: Failed to allocate slot '" << slotId << "'.\n";
            return false;                                // allocation failure
        }
        activeVehicles_.emplace(plate, VehicleEntry(plate, type, entryDt, slotId)); // store active vehicle
        ParkingSlot slotInfo;                            // slot details for message
        std::cout << "Success: '" << plate << "' entered at " << formatDateTime(entryDt)
                  << ". Slot: " << slotId;
        if (getSlot(slotId, slotInfo)) std::cout << " (Zone: " << slotInfo.zone << ")";
        std::cout << ".\n";
        return true;                                     // entry success
    }

    // Task 5 report: currently parked vehicles
    void displayParkedVehicles() const {
        if (activeVehicles_.empty()) {                   // no active vehicles
            std::cout << "No vehicles currently parked.\n";
            return;                                      // nothing to show
        }
        std::cout << "\n--- Currently Parked Vehicles ---\n";
        std::cout << std::left << std::setw(12) << "Plate" << std::setw(14) << "Vehicle Type"
                  << std::setw(10) << "Slot ID" << std::setw(20) << "Entry" << "Zone\n"
                  << std::string(66, '-') << "\n";
        for (std::unordered_map<std::string, VehicleEntry>::const_iterator it = activeVehicles_.begin();
             it != activeVehicles_.end(); ++it) {       // traverse active map
            const VehicleEntry& entry = it->second;      // current entry
            std::string zone = "N/A";                    // default zone label
            std::unordered_map<std::string, ParkingSlot>::const_iterator sit = slots_.find(entry.slotId);
            if (sit != slots_.end()) zone = sit->second.zone; // lookup zone
            std::cout << std::left << std::setw(12) << entry.plateNumber
                      << std::setw(14) << vehicleTypeToString(entry.vehicleType)
                      << std::setw(10) << entry.slotId
                      << std::setw(20) << formatDateTime(entry.entryDateTime)
                      << zone << "\n";
        }
        std::cout << "Parked count: " << activeVehicles_.size() << "\n\n";
    }

    // Task 3: update active hourly price (history unchanged)
    bool updateParkingPrice(VehicleType type, int newPrice) {
        if (newPrice <= 0) {                             // must be positive
            std::cout << "Error: Price must be a positive integer (RWF/hour).\n";
            return false;                                // reject zero/negative
        }
        if (newPrice > 1000000) {                        // upper sanity limit
            std::cout << "Error: Price exceeds maximum allowed (1,000,000 RWF/hour).\n";
            return false;                                // reject too large
        }
        int oldPrice = currentPrices_[type];             // previous active rate
        currentPrices_[type] = newPrice;                 // apply new active rate
        std::cout << "Success: " << vehicleTypeToString(type) << " rate changed from "
                  << oldPrice << " to " << newPrice << " RWF/hour.\n";
        std::cout << "Note: Past completed transactions keep their original rates.\n";
        return true;                                     // update success
    }

    // Task 3: show all current tariffs including Truck
    void displayCurrentPrices() const {
        std::cout << "\n--- Current Parking Tariffs (RWF/hour) ---\n";
        std::cout << "Motorcycle: " << currentPrices_.at(VehicleType::MOTORCYCLE) << "\n";
        std::cout << "Car:        " << currentPrices_.at(VehicleType::CAR) << "\n";
        std::cout << "Truck:      " << currentPrices_.at(VehicleType::TRUCK)
                  << "  (Truck uses same billing rules; default 2000 RWF/hour)\n\n";
    }

    // Task 4: process exit, free slot, calculate fee, store history
    bool processVehicleExit(const std::string& plate, const DateTime& exitDt, ParkingTransaction& outTx) {
        if (isEmptyOrWhitespace(plate)) {                // empty plate check
            std::cout << "Error: Plate number cannot be empty.\n";
            return false;                                // reject empty
        }
        if (!isValidIdentifier(plate)) {                 // plate format validation
            std::cout << "Error: Invalid plate number format.\n";
            return false;                                // reject bad format
        }
        std::unordered_map<std::string, VehicleEntry>::iterator vit = activeVehicles_.find(plate);
        if (vit == activeVehicles_.end()) {              // vehicle not parked
            std::cout << "Error: Vehicle '" << plate << "' is not currently parked.\n";
            return false;                                // prevent double exit
        }
        if (isFutureDateTime(exitDt)) {                  // future date/time not allowed
            std::cout << "Error: Future exit date/time is not allowed.\n";
            return false;                                // reject future exit
        }
        const VehicleEntry& entry = vit->second;         // active entry record
        long long mins = durationMinutes(entry.entryDateTime, exitDt); // duration in minutes
        if (mins < 0) {                                  // exit before entry
            std::cout << "Error: Exit time cannot be earlier than entry time ("
                      << formatDateTime(entry.entryDateTime) << ").\n";
            return false;                                // invalid time order
        }
        int billed = calculateBilledHours(mins);           // ceiling hour billing
        int rate = currentPrices_.at(entry.vehicleType); // active rate at exit (Motorcycle/Car/Truck)
        int fee = billed * rate;                         // total parking fee
        std::string zone = "N/A";                        // default zone
        std::unordered_map<std::string, ParkingSlot>::iterator sit = slots_.find(entry.slotId);
        if (sit != slots_.end()) zone = sit->second.zone; // read slot zone

        outTx.plateNumber = entry.plateNumber;           // copy plate to history
        outTx.vehicleType = entry.vehicleType;           // copy type
        outTx.slotId = entry.slotId;                     // copy slot id
        outTx.zone = zone;                               // copy zone
        outTx.entryDateTime = entry.entryDateTime;       // copy entry time
        outTx.exitDateTime = exitDt;                     // copy exit time
        outTx.durationMinutes = mins;                    // store duration
        outTx.billedHours = billed;                      // store billed hours
        outTx.ratePerHour = rate;                        // store rate used (immutable in history)
        outTx.totalFee = fee;                            // store total fee

        transactionHistory_.push_back(outTx);            // append to history vector
        setSlotStatus(entry.slotId, SlotStatus::AVAILABLE); // release slot
        activeVehicles_.erase(vit);                        // remove active record

        std::cout << "\n--- Parking Exit Receipt ---\n";
        std::cout << "Plate:        " << outTx.plateNumber << "\n";
        std::cout << "Vehicle Type: " << vehicleTypeToString(outTx.vehicleType) << "\n";
        std::cout << "Slot / Zone:  " << outTx.slotId << " / " << outTx.zone << "\n";
        std::cout << "Entry:        " << formatDateTime(outTx.entryDateTime) << "\n";
        std::cout << "Exit:         " << formatDateTime(outTx.exitDateTime) << "\n";
        std::cout << "Duration:     " << outTx.durationMinutes << " minutes\n";
        std::cout << "Billed Hours: " << outTx.billedHours << " (partial hours rounded up)\n";
        std::cout << "Rate Applied: " << outTx.ratePerHour << " RWF/hour\n";
        std::cout << "Total Fee:    " << outTx.totalFee << " RWF\n";
        std::cout << "----------------------------\n";
        return true;                                     // exit success
    }

    // Task 5: vehicle history by plate
    void displayVehicleHistory(const std::string& plate) const {
        if (isEmptyOrWhitespace(plate)) {                // empty plate check
            std::cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stop report
        }
        bool found = false;                              // match flag
        std::cout << "\n--- Parking History for '" << plate << "' ---\n";
        for (size_t i = 0; i < transactionHistory_.size(); ++i) { // scan history
            const ParkingTransaction& tx = transactionHistory_[i]; // current record
            if (tx.plateNumber == plate) {               // plate match
                found = true;                            // mark found
                std::cout << formatDateTime(tx.entryDateTime) << " -> "
                          << formatDateTime(tx.exitDateTime) << " | "
                          << vehicleTypeToString(tx.vehicleType) << " | Slot: " << tx.slotId
                          << " | Billed: " << tx.billedHours << "h | Rate: " << tx.ratePerHour
                          << " RWF/h | Fee: " << tx.totalFee << " RWF\n";
            }
        }
        if (!found) std::cout << "No completed records found for this plate.\n";
        std::cout << "\n";
    }

    // Task 5: all transaction history
    void displayAllHistory() const {
        if (transactionHistory_.empty()) {               // empty history
            std::cout << "No completed parking transactions yet.\n";
            return;                                      // nothing to show
        }
        std::cout << "\n--- All Transaction History ---\n";
        for (size_t i = 0; i < transactionHistory_.size(); ++i) { // traverse history
            const ParkingTransaction& tx = transactionHistory_[i]; // current tx
            std::cout << (i + 1) << ". " << tx.plateNumber << " | "
                      << vehicleTypeToString(tx.vehicleType) << " | Slot: " << tx.slotId
                      << " | " << formatDateTime(tx.entryDateTime) << " -> "
                      << formatDateTime(tx.exitDateTime) << " | Fee: " << tx.totalFee
                      << " RWF (rate: " << tx.ratePerHour << ")\n";
        }
        std::cout << "Total transactions: " << transactionHistory_.size() << "\n\n";
    }

    // Task 5: daily revenue report
    void displayDailyRevenue() const {
        long long total = 0;                             // revenue accumulator
        for (size_t i = 0; i < transactionHistory_.size(); ++i) { // sum fees
            total += transactionHistory_[i].totalFee;    // add each fee
        }
        std::cout << "\n--- Daily Revenue Report ---\n";
        std::cout << "Completed transactions: " << transactionHistory_.size() << "\n";
        std::cout << "Total revenue:          " << total << " RWF\n\n";
    }

    // Load demo slots for testing (includes Truck slot T-C1)
    void loadDemoData() {
        if (!slots_.empty()) {                           // avoid overwrite
            std::cout << "Demo data skipped: slots already exist.\n";
            return;                                      // stop loading
        }
        addParkingSlot("M-A1", VehicleType::MOTORCYCLE, "Downtown"); // motorcycle slot
        addParkingSlot("M-A2", VehicleType::MOTORCYCLE, "Downtown"); // motorcycle slot
        addParkingSlot("C-B1", VehicleType::CAR, "Airport");         // car slot
        addParkingSlot("C-B2", VehicleType::CAR, "Airport");         // car slot
        addParkingSlot("T-C1", VehicleType::TRUCK, "Industrial");    // truck slot
        std::cout << "Demo slots loaded (Motorcycle, Car, Truck).\n";
    }

private:
    std::unordered_map<std::string, ParkingSlot> slots_;              // slot id -> slot
    std::unordered_map<std::string, VehicleEntry> activeVehicles_;   // plate -> active entry
    std::vector<ParkingTransaction> transactionHistory_;               // completed sessions
    std::unordered_map<VehicleType, int> currentPrices_;             // type -> active rate
    std::vector<TariffPolicy*> tariffPolicies_;                      // polymorphic tariff objects

    bool findAvailableSlot(VehicleType type, std::string& outSlotId) const {
        for (std::unordered_map<std::string, ParkingSlot>::const_iterator it = slots_.begin();
             it != slots_.end(); ++it) {                // scan all slots
            const ParkingSlot& slot = it->second;        // current slot
            if (slot.status == SlotStatus::AVAILABLE && slot.vehicleType == type) { // match
                outSlotId = slot.slotId;                 // return slot id
                return true;                             // found
            }
        }
        outSlotId.clear();                               // clear output
        return false;                                    // not found
    }

    bool setSlotStatus(const std::string& slotId, SlotStatus status) {
        std::unordered_map<std::string, ParkingSlot>::iterator it = slots_.find(slotId);
        if (it == slots_.end()) return false;            // slot missing
        it->second.status = status;                      // update status
        return true;                                     // success
    }

    bool getSlot(const std::string& slotId, ParkingSlot& outSlot) const {
        std::unordered_map<std::string, ParkingSlot>::const_iterator it = slots_.find(slotId);
        if (it == slots_.end()) return false;            // slot missing
        outSlot = it->second;                            // copy slot data
        return true;                                     // success
    }
};

// Shows welcome banner at program start
void displayWelcome() {
    std::cout << "\n================================================\n";
    std::cout << "     KIGALI SMART PARKING MANAGEMENT SYSTEM     \n";
    std::cout << "================================================\n";
    std::cout << " Default Rates: Motorcycle 500 | Car 1000 | Truck 2000 RWF/hr\n";
    std::cout << " Future dates/times are NOT allowed.\n";
    std::cout << " Tip: Use option 12 to load demo slots.\n";
    std::cout << "================================================\n";
}

// Shows main menu; returns false only on read failure (should not exit program)
void displayMenu() {
    std::cout << "\n=================== MAIN MENU ===================\n";
    std::cout << " 1. Add Parking Slot\n";
    std::cout << " 2. View All Parking Slots\n";
    std::cout << " 3. View Available Slots\n";
    std::cout << " 4. Register Vehicle Entry\n";
    std::cout << " 5. Process Vehicle Exit\n";
    std::cout << " 6. View Parked Vehicles\n";
    std::cout << " 7. Update Parking Price\n";
    std::cout << " 8. View Current Tariffs\n";
    std::cout << " 9. Vehicle Parking History\n";
    std::cout << "10. All Transaction History\n";
    std::cout << "11. Daily Revenue Report\n";
    std::cout << "12. Load Demo Data\n";
    std::cout << " 0. Exit\n";
    std::cout << "=================================================\n";
}

// Reads menu choice safely; rejects 09, letters, decimals; ONLY exact 0 exits
bool readMenuChoice(int& choice) {
  displayMenu();                                         // show menu first
  std::string line = readLine("Enter Choice: ");         // read full line as text
  line = trim(line);                                     // trim spaces
  if (line.empty()) {                                    // empty input
    std::cout << "Invalid input! Please enter a valid choice (0-12).\n";
    return false;                                        // do not exit program
  }
  if (!parseStrictInteger(line, choice, 0, 12)) {        // strict parse 0-12
    std::cout << "Invalid input! Please enter a valid choice (0-12).\n";
    return false;                                        // reject 09, abc, 1.5, etc.
  }
  return true;                                           // valid menu choice
}

// Prompts vehicle type 1-3 with strict validation
bool promptVehicleType(VehicleType& outType) {
    std::cout << "Vehicle type: 1=Motorcycle  2=Car  3=Truck\n";
    std::string line = readLine("Enter type (1-3): ");   // read as text
    int typeChoice = 0;                                  // parsed choice
    if (!parseStrictInteger(trim(line), typeChoice, 1, 3)) { // strict 1-3
        std::cout << "Invalid input! Please enter 1, 2, or 3.\n";
        return false;                                    // invalid type input
    }
    if (typeChoice == 1) outType = VehicleType::MOTORCYCLE; // map to enum
    else if (typeChoice == 2) outType = VehicleType::CAR;   // map to enum
    else outType = VehicleType::TRUCK;                      // map to enum
    return true;                                         // success
}

// Prompts date and time; rejects future and invalid formats
bool promptDateTime(const std::string& label, DateTime& outDt) {
    std::string line = trim(readLine(label));            // read user line
    if (line.empty()) {                                  // empty check
        std::cout << "Error: Date/time cannot be empty.\n";
        return false;                                    // reject empty
    }
    if (!parseDateTime(line, outDt)) {                   // parse DD-MM-YYYY HH:MM
        std::cout << "Invalid format! Use DD-MM-YYYY HH:MM (e.g. 10-06-2026 14:30).\n";
        return false;                                    // bad format
    }
    if (isFutureDateTime(outDt)) {                       // future not allowed
        std::cout << "Error: Future date/time is not allowed.\n";
        return false;                                    // reject future
    }
    return true;                                         // valid datetime
}

// Menu handler: add parking slot
void handleAddSlot(ParkingSystem& system) {
    try {                                                // exception guard
        std::string slotId = trim(readLine("Enter Slot ID (e.g. C-A1): "));
        if (isEmptyOrWhitespace(slotId)) {               // empty validation
            std::cout << "Error: Slot ID cannot be empty.\n";
            return;                                      // stay in program
        }
        VehicleType type;                                // selected type
        if (!promptVehicleType(type)) return;            // invalid type
        std::string zone = trim(readLine("Enter Zone (e.g. Downtown): "));
        if (isEmptyOrWhitespace(zone)) {                 // empty zone
            std::cout << "Error: Zone name cannot be empty.\n";
            return;                                      // stay in program
        }
        system.addParkingSlot(slotId, type, zone);       // delegate to system
    } catch (const std::exception& ex) {                 // catch runtime errors
        std::cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: vehicle entry
void handleVehicleEntry(ParkingSystem& system) {
    try {                                                // exception guard
        std::string plate = trim(readLine("Enter plate number: "));
        if (isEmptyOrWhitespace(plate)) {                // empty plate
            std::cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stay in program
        }
        VehicleType type;                                // selected type
        if (!promptVehicleType(type)) return;            // invalid type
        DateTime entryDt;                                // entry datetime
        if (!promptDateTime("Enter entry (DD-MM-YYYY HH:MM): ", entryDt)) return;
        system.registerVehicleEntry(plate, type, entryDt); // register entry
    } catch (const std::exception& ex) {                 // catch runtime errors
        std::cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: vehicle exit
void handleVehicleExit(ParkingSystem& system) {
    try {                                                // exception guard
        std::string plate = trim(readLine("Enter plate number: "));
        if (isEmptyOrWhitespace(plate)) {                // empty plate
            std::cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stay in program
        }
        DateTime exitDt;                                 // exit datetime
        if (!promptDateTime("Enter exit (DD-MM-YYYY HH:MM): ", exitDt)) return;
        ParkingTransaction receipt;                      // output receipt
        system.processVehicleExit(plate, exitDt, receipt); // process exit
    } catch (const std::exception& ex) {                 // catch runtime errors
        std::cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: update parking price
void handlePriceUpdate(ParkingSystem& system) {
    try {                                                // exception guard
        VehicleType type;                                // type to update
        if (!promptVehicleType(type)) return;            // invalid type
        std::string line = readLine("Enter new hourly price (RWF): "); // read price text
        int newPrice = 0;                                // parsed price
        if (!parseStrictInteger(trim(line), newPrice, 1, 1000000)) { // positive int
            std::cout << "Invalid input! Enter a positive whole number.\n";
            return;                                      // stay in program
        }
        system.updateParkingPrice(type, newPrice);       // apply update
    } catch (const std::exception& ex) {                 // catch runtime errors
        std::cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: vehicle history search
void handleVehicleHistory(ParkingSystem& system) {
    try {                                                // exception guard
        std::string plate = trim(readLine("Enter plate to search: "));
        if (isEmptyOrWhitespace(plate)) {                // empty plate
            std::cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stay in program
        }
        system.displayVehicleHistory(plate);             // show history
    } catch (const std::exception& ex) {                 // catch runtime errors
        std::cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Application entry point
int main() {
    ParkingSystem system;                                // create parking system
    displayWelcome();                                    // show welcome screen
    int choice = -1;                                     // menu choice variable

    while (true) {                                       // main loop until explicit exit
        try {                                            // protect each menu cycle
            if (!readMenuChoice(choice)) {               // invalid menu input
                continue;                                // re-show menu, DO NOT exit
            }
            if (choice == 0) {                           // ONLY exact valid 0 exits
                std::cout << "Thank you for using Smart Parking System.\n";
                break;                                   // leave loop intentionally
            }
            switch (choice) {                            // dispatch valid options
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
                default:
                    std::cout << "Invalid input! Please enter a valid choice (0-12).\n";
                    break;                               // continue program
            }
        } catch (const std::exception& ex) {           // catch unexpected errors
            std::cout << "System error: " << ex.what() << "\n";
        } catch (...) {                                  // catch unknown errors
            std::cout << "Unknown error occurred. Program continues.\n";
        }
    }
    return 0;                                            // normal termination
}
