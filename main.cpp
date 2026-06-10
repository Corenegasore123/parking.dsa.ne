/*
 * Kigali Smart Parking Management System
 * Compile: g++ -o parking main.cpp
 */

#include <algorithm>      
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

using namespace std;        

// Enumeration for supported vehicle categories
enum class VehicleType { MOTORCYCLE, CAR, TRUCK };

// Enumeration for parking slot occupancy state
enum class SlotStatus { AVAILABLE, OCCUPIED };

// Converts VehicleType enum to display text
string vehicleTypeToString(VehicleType type) {
    switch (type) {                                    // branch on type
        case VehicleType::MOTORCYCLE: return "Motorcycle"; // motorcycle label
        case VehicleType::CAR:        return "Car";        // car label
        case VehicleType::TRUCK:      return "Truck";      // truck label
        default:                      return "Unknown";    // safety fallback
    }
}

// Converts SlotStatus enum to display text
string slotStatusToString(SlotStatus status) {
    switch (status) {                                  // branch on status
        case SlotStatus::AVAILABLE: return "Available"; // free slot
        case SlotStatus::OCCUPIED:  return "Occupied";  // used slot
        default:                    return "Unknown";   // safety fallback
    }
}

// Clears cin error state and discards leftover input characters
void clearInputStream() {
cin.clear();                                  // reset fail bits
cin.ignore(10000, '\n');                      // discard bad input line
}

// Trims leading and trailing whitespace from a string
string trim(const string& text) {
    size_t start = 0;                                  // first non-space index
    while (start < text.size() && isspace(static_cast<unsigned char>(text[start]))) {
        ++start;                                       // skip leading spaces
    }
    size_t end = text.size();                          // end index
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;                                         // skip trailing spaces
    }
    return text.substr(start, end - start);            // return trimmed substring
}

// Reads one full input line after showing a prompt
string readLine(const string& prompt) {
cout << prompt;                               // show prompt text
string line;                                  // buffer for input
getline(cin, line);                      // read until newline
    return line;                                       // return raw line
}

// Returns true when text is empty or only whitespace
bool isEmptyOrWhitespace(const string& value) {
    return trim(value).empty();                        // empty after trim
}

// Validates parking slot IDs: letter first, alphanumeric + hyphen, 2-15 chars
bool isValidSlotId(const string& value) {
    string v = trim(value);                            // normalize input
    if (v.empty() || v.length() > 15) return false;    // length rule
    if (!isalpha(static_cast<unsigned char>(v[0]))) return false; // must start with letter
    for (size_t i = 0; i < v.size(); ++i) {            // scan each character
        char ch = v[i];                                // current char
        if (!(isalnum(static_cast<unsigned char>(ch)) || ch == '-')) return false;
    }
    return true;                                       // valid slot id
}

// Validates plate: 6-8 characters, no spaces, no hyphens (Rwanda or foreign)
bool isValidPlateNumber(const string& plate) {
    if (plate.length() < 6 || plate.length() > 8) return false; // min 6, max 8 chars
    for (size_t i = 0; i < plate.size(); ++i) {            // scan each character
        char ch = plate[i];                              // current character
        if (isspace(static_cast<unsigned char>(ch)) || ch == '-') return false; // no spaces/hyphens
    }
    return true;                                         // plate accepted
}

// Trims and uppercases plate for storage; returns "" if invalid
string normalizePlateNumber(const string& input) {
    string plate = trim(input);                          // remove outer whitespace
    if (!isValidPlateNumber(plate)) return "";           // reject invalid plate
    for (size_t i = 0; i < plate.size(); ++i) {          // uppercase for consistency
        plate[i] = static_cast<char>(toupper(static_cast<unsigned char>(plate[i])));
    }
    return plate;                                        // return normalized plate
}

// Validates zone names: letters, spaces, hyphens, 2-30 chars
bool isValidZoneName(const string& value) {
string v = trim(value);                       // normalize input
    if (v.empty() || v.length() > 30) return false;    // length rule
    for (size_t i = 0; i < v.size(); ++i) {            // scan each character
        char ch = v[i];                                // current char
        if (!(isalpha(static_cast<unsigned char>(ch)) || ch == '-' || ch == ' ')) return false;
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
    bool toTimeT(time_t& out) const {
tm tmVal;                                 // broken-down time struct
        tmVal.tm_sec  = 0;                             // seconds set to zero
        tmVal.tm_min  = minute;                        // copy minute
        tmVal.tm_hour = hour;                          // copy hour
        tmVal.tm_mday = day;                           // copy day
        tmVal.tm_mon  = month - 1;                     // tm month is 0-based
        tmVal.tm_year = year - 1900;                   // tm year offset
        tmVal.tm_isdst = -1;                           // let library detect DST
        out = mktime(&tmVal);                     // convert to time_t
        return out != static_cast<time_t>(-1);    // false if conversion failed
    }

    // Builds DateTime from current system clock
    static DateTime now() {
time_t t = time(NULL);               // current timestamp
tm* lt = localtime(&t);              // local time breakdown
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
string formatDateTime(const DateTime& dt) {
    char buffer[20];                                   // format buffer
    snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d %02d:%02d",
             dt.day, dt.month, dt.year, dt.hour, dt.minute); // build text
    return string(buffer);                        // return as string
}

// Validates calendar fields and rejects impossible dates
bool isValidCalendarDate(int day, int month, int year) {
    if (year < 2000 || year > 2100) return false;      // reasonable year range
    if (month < 1 || month > 12) return false;       // month range
    int dim = daysInMonth(month, year);                // days in month
    if (day < 1 || day > dim) return false;            // day range
    return true;                                       // date is valid
}

// Returns true if the calendar date is before today (past dates not allowed)
bool isPastDate(const DateTime& dt) {
    DateTime today = DateTime::now();                    // current system date
    if (dt.year < today.year) return true;               // year in the past
    if (dt.year > today.year) return false;              // year in the future
    if (dt.month < today.month) return true;             // month in the past
    if (dt.month > today.month) return false;            // month in the future
    return dt.day < today.day;                           // day in the past
}

// Returns true if the calendar date is after today (future dates not allowed)
bool isFutureDate(const DateTime& dt) {
    DateTime today = DateTime::now();                    // current system date
    if (dt.year > today.year) return true;               // year in the future
    if (dt.year < today.year) return false;              // year in the past
    if (dt.month > today.month) return true;             // month in the future
    if (dt.month < today.month) return false;            // month in the past
    return dt.day > today.day;                           // day in the future
}

// Returns true if dt is later than current system clock (future time today not allowed)
bool isFutureDateTime(const DateTime& dt) {
    time_t inputTime;                                    // input as time_t
    if (!dt.toTimeT(inputTime)) return true;             // invalid counts as reject
    time_t nowTime = time(NULL);                         // current time
    return inputTime > nowTime;                          // future if greater than now
}

// Validates parking date/time: today only; past times OK; future time rejected
bool validateParkingDateTime(const DateTime& dt, string& errorMsg) {
    if (isPastDate(dt)) {                                // yesterday or earlier
        errorMsg = "Error: Past dates are not allowed. Use today's date only.";
        return false;                                    // reject past date
    }
    if (isFutureDate(dt)) {                               // tomorrow or later
        errorMsg = "Error: Future dates are not allowed. Use today's date only.";
        return false;                                    // reject future date
    }
    if (isFutureDateTime(dt)) {                            // later than now today
        errorMsg = "Error: Future time is not allowed. Past times today are accepted.";
        return false;                                    // reject future time
    }
    return true;                                         // valid for entry/exit
}

// Parses DD-MM-YYYY HH:MM string into DateTime
bool parseDateTime(const string& input, DateTime& out) {
stringstream ss(input);                       // stream for parsing
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
time_t tEntry, tExit;                         // epoch times
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
bool parseStrictInteger(const string& text, int& outValue, int minVal, int maxVal) {
string v = trim(text);                        // trim spaces
    if (v.empty()) return false;                       // empty invalid
    if (v.size() > 1 && v[0] == '0') return false;     // reject 09, 01, etc.
    for (size_t i = 0; i < v.size(); ++i) {            // every char must be digit
        if (!isdigit(static_cast<unsigned char>(v[i]))) return false;
    }
stringstream ss(v);                           // parse digits
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
    virtual string getName() const = 0;           // pure virtual label
};

// Motorcycle tariff policy: 500 RWF/hour default
class MotorcycleTariff : public TariffPolicy {
public:
    VehicleType getType() const { return VehicleType::MOTORCYCLE; } // motorcycle type
    int getDefaultRate() const { return 500; }         // assignment default rate
string getName() const { return "Motorcycle"; } // display name
};

// Car tariff policy: 1000 RWF/hour default
class CarTariff : public TariffPolicy {
public:
    VehicleType getType() const { return VehicleType::CAR; } // car type
    int getDefaultRate() const { return 1000; }        // assignment default rate
string getName() const { return "Car"; }      // display name
};

// Truck tariff policy: 2000 RWF/hour default (Task 2 requires Truck support)
class TruckTariff : public TariffPolicy {
public:
    VehicleType getType() const { return VehicleType::TRUCK; } // truck type
    int getDefaultRate() const { return 2000; }        // extension default for trucks
string getName() const { return "Truck"; }    // display name
};

// Parking slot model
class ParkingSlot {
public:
string slotId;                                // unique slot identifier
    VehicleType vehicleType;                           // supported vehicle type
string zone;                                  // location zone name
    SlotStatus status;                                 // available or occupied

    ParkingSlot()
        : slotId(""), vehicleType(VehicleType::MOTORCYCLE), zone(""), status(SlotStatus::AVAILABLE) {}

    ParkingSlot(const string& id, VehicleType type, const string& zoneName, SlotStatus slotStatus)
        : slotId(id), vehicleType(type), zone(zoneName), status(slotStatus) {}
};

// Active parked vehicle record
class VehicleEntry {
public:
string plateNumber;                           // unique plate while parked
    VehicleType vehicleType;                           // vehicle category
    DateTime entryDateTime;                            // entry date and time
string slotId;                                // allocated slot id

    VehicleEntry()
        : plateNumber(""), vehicleType(VehicleType::CAR), slotId("") {}

    VehicleEntry(const string& plate, VehicleType type, const DateTime& dt, const string& slot)
        : plateNumber(plate), vehicleType(type), entryDateTime(dt), slotId(slot) {}
};

// Completed parking transaction stored in history
class ParkingTransaction {
public:
string plateNumber;                           // vehicle plate
    VehicleType vehicleType;                           // vehicle category
string slotId;                                // used slot
string zone;                                  // slot zone
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
        } catch (const exception& ex) {           // catch init errors
cout << "Error initializing tariffs: " << ex.what() << "\n";
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
    bool addParkingSlot(const string& slotId, VehicleType type, const string& zone) {
        if (isEmptyOrWhitespace(slotId)) {               // empty slot id check
cout << "Error: Slot ID cannot be empty.\n";
            return false;                                // reject empty
        }
        if (!isValidSlotId(slotId)) {                    // slot id format validation
            cout << "Error: Invalid Slot ID. Start with a letter; use letters, digits, hyphens (2-15 chars).\n";
            return false;                                // reject bad format
        }
        if (isEmptyOrWhitespace(zone)) {               // empty zone check
cout << "Error: Zone name cannot be empty.\n";
            return false;                                // reject empty zone
        }
        if (!isValidZoneName(zone)) {                    // zone format validation
cout << "Error: Invalid zone name. Use letters, spaces, hyphens only (2-30 chars).\n";
            return false;                                // reject bad zone
        }
        if (slots_.find(slotId) != slots_.end()) {      // duplicate slot id check
cout << "Error: Slot ID '" << slotId << "' already exists.\n";
            return false;                                // reject duplicate
        }
        slots_.emplace(slotId, ParkingSlot(slotId, type, zone, SlotStatus::AVAILABLE)); // insert slot
cout << "Success: Slot '" << slotId << "' added for "
                  << vehicleTypeToString(type) << " in zone '" << zone << "'.\n";
        return true;                                     // success
    }

    // Task 1 report: show all slots
    void displayAllSlots() const {
        if (slots_.empty()) {                            // no slots configured
cout << "No parking slots configured yet.\n";
            return;                                      // nothing to show
        }
cout << "\n--- All Parking Slots ---\n";
cout << left << setw(10) << "Slot ID" << setw(14) << "Vehicle Type"
                  << setw(18) << "Zone" << "Status\n" << string(52, '-') << "\n";
        for (unordered_map<string, ParkingSlot>::const_iterator it = slots_.begin();
             it != slots_.end(); ++it) {                // traverse slot map
            const ParkingSlot& slot = it->second;        // current slot
cout << left << setw(10) << slot.slotId
                      << setw(14) << vehicleTypeToString(slot.vehicleType)
                      << setw(18) << slot.zone << slotStatusToString(slot.status) << "\n";
        }
cout << "Total slots: " << slots_.size() << "\n\n";
    }

    // Task 5 report: available slots only
    void displayAvailableSlots() const {
        int count = 0;                                   // available counter
cout << "\n--- Available Parking Slots ---\n";
cout << left << setw(10) << "Slot ID" << setw(14) << "Vehicle Type"
                  << "Zone\n" << string(36, '-') << "\n";
        for (unordered_map<string, ParkingSlot>::const_iterator it = slots_.begin();
             it != slots_.end(); ++it) {                // traverse slots
            const ParkingSlot& slot = it->second;        // current slot
            if (slot.status == SlotStatus::AVAILABLE) {  // only available
cout << left << setw(10) << slot.slotId
                          << setw(14) << vehicleTypeToString(slot.vehicleType)
                          << slot.zone << "\n";
                ++count;                                 // increment count
            }
        }
        if (count == 0) cout << "(No available slots)\n";
cout << "Available count: " << count << "\n\n";
    }

    // Task 2: register vehicle entry with slot allocation
    bool registerVehicleEntry(const string& plate, VehicleType type, const DateTime& entryDt) {
        if (isEmptyOrWhitespace(plate)) {                // empty plate check
            cout << "Error: Plate number cannot be empty.\n";
            return false;                                // reject empty
        }
        string normalizedPlate = normalizePlateNumber(plate); // 6-8 chars, no spaces/hyphens
        if (normalizedPlate.empty()) {                 // invalid plate format
            cout << "Error: Invalid plate. Use 6-8 characters, no spaces or hyphens (e.g. RAB123A, UG1234AB).\n";
            return false;                                // reject bad plate format
        }
        if (activeVehicles_.find(normalizedPlate) != activeVehicles_.end()) { // duplicate active plate
            cout << "Error: Vehicle '" << normalizedPlate << "' is already parked.\n";
            return false;                                // prevent double parking
        }
        string dateError;                                  // validation message buffer
        if (!validateParkingDateTime(entryDt, dateError)) { // today only; no future time
            cout << dateError << "\n";
            return false;                                // reject invalid entry time
        }
string slotId;                              // slot to allocate
        if (!findAvailableSlot(type, slotId)) {          // find matching free slot
cout << "Error: No available " << vehicleTypeToString(type)
                      << " slot. Please try again later.\n";
            return false;                                // graceful no-slot message
        }
        if (!setSlotStatus(slotId, SlotStatus::OCCUPIED)) { // mark slot occupied
cout << "Error: Failed to allocate slot '" << slotId << "'.\n";
            return false;                                // allocation failure
        }
        activeVehicles_.emplace(normalizedPlate, VehicleEntry(normalizedPlate, type, entryDt, slotId)); // store vehicle
        ParkingSlot slotInfo;                            // slot details for message
        cout << "Success: '" << normalizedPlate << "' entered at " << formatDateTime(entryDt)
             << ". Slot: " << slotId;
        if (getSlot(slotId, slotInfo)) cout << " (Zone: " << slotInfo.zone << ")";
cout << ".\n";
        return true;                                     // entry success
    }

    // Task 5 report: currently parked vehicles
    void displayParkedVehicles() const {
        if (activeVehicles_.empty()) {                   // no active vehicles
cout << "No vehicles currently parked.\n";
            return;                                      // nothing to show
        }
cout << "\n--- Currently Parked Vehicles ---\n";
cout << left << setw(12) << "Plate" << setw(14) << "Vehicle Type"
                  << setw(10) << "Slot ID" << setw(20) << "Entry" << "Zone\n"
                  << string(66, '-') << "\n";
        for (unordered_map<string, VehicleEntry>::const_iterator it = activeVehicles_.begin();
             it != activeVehicles_.end(); ++it) {       // traverse active map
            const VehicleEntry& entry = it->second;      // current entry
string zone = "N/A";                    // default zone label
unordered_map<string, ParkingSlot>::const_iterator sit = slots_.find(entry.slotId);
            if (sit != slots_.end()) zone = sit->second.zone; // lookup zone
cout << left << setw(12) << entry.plateNumber
                      << setw(14) << vehicleTypeToString(entry.vehicleType)
                      << setw(10) << entry.slotId
                      << setw(20) << formatDateTime(entry.entryDateTime)
                      << zone << "\n";
        }
cout << "Parked count: " << activeVehicles_.size() << "\n\n";
    }

    // Task 3: update active hourly price (history unchanged)
    bool updateParkingPrice(VehicleType type, int newPrice) {
        if (newPrice <= 0) {                             // must be positive
cout << "Error: Price must be a positive integer (RWF/hour).\n";
            return false;                                // reject zero/negative
        }
        if (newPrice > 1000000) {                        // upper sanity limit
cout << "Error: Price exceeds maximum allowed (1,000,000 RWF/hour).\n";
            return false;                                // reject too large
        }
        int oldPrice = currentPrices_[type];             // previous active rate
        currentPrices_[type] = newPrice;                 // apply new active rate
cout << "Success: " << vehicleTypeToString(type) << " rate changed from "
                  << oldPrice << " to " << newPrice << " RWF/hour.\n";
cout << "Note: Past completed transactions keep their original rates.\n";
        return true;                                     // update success
    }

    // Task 3: show all current tariffs including Truck
    void displayCurrentPrices() const {
cout << "\n--- Current Parking Tariffs (RWF/hour) ---\n";
cout << "Motorcycle: " << currentPrices_.at(VehicleType::MOTORCYCLE) << "\n";
cout << "Car:        " << currentPrices_.at(VehicleType::CAR) << "\n";
cout << "Truck:      " << currentPrices_.at(VehicleType::TRUCK)
                  << "\n";
    }

    // Task 4: process exit, free slot, calculate fee, store history
    bool processVehicleExit(const string& plate, const DateTime& exitDt, ParkingTransaction& outTx) {
        if (isEmptyOrWhitespace(plate)) {                // empty plate check
            cout << "Error: Plate number cannot be empty.\n";
            return false;                                // reject empty
        }
        string normalizedPlate = normalizePlateNumber(plate); // 6-8 chars, no spaces/hyphens
        if (normalizedPlate.empty()) {                 // invalid plate format
            cout << "Error: Invalid plate. Use 6-8 characters, no spaces or hyphens (e.g. RAB123A, UG1234AB).\n";
            return false;                                // reject bad plate format
        }
        unordered_map<string, VehicleEntry>::iterator vit = activeVehicles_.find(normalizedPlate);
        if (vit == activeVehicles_.end()) {              // vehicle not parked
            cout << "Error: Vehicle '" << normalizedPlate << "' is not currently parked.\n";
            return false;                                // prevent double exit
        }
        string dateError;                                  // validation message buffer
        if (!validateParkingDateTime(exitDt, dateError)) { // today only; no future time
            cout << dateError << "\n";
            return false;                                // reject invalid exit time
        }
        const VehicleEntry& entry = vit->second;         // active entry record
        long long mins = durationMinutes(entry.entryDateTime, exitDt); // duration in minutes
        if (mins < 0) {                                  // exit before entry
cout << "Error: Exit time cannot be earlier than entry time ("
                      << formatDateTime(entry.entryDateTime) << ").\n";
            return false;                                // invalid time order
        }
        int billed = calculateBilledHours(mins);           // ceiling hour billing
        int rate = currentPrices_.at(entry.vehicleType); // active rate at exit (Motorcycle/Car/Truck)
        int fee = billed * rate;                         // total parking fee
string zone = "N/A";                        // default zone
unordered_map<string, ParkingSlot>::iterator sit = slots_.find(entry.slotId);
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

cout << "\n--- Parking Exit Receipt ---\n";
cout << "Plate:        " << outTx.plateNumber << "\n";
cout << "Vehicle Type: " << vehicleTypeToString(outTx.vehicleType) << "\n";
cout << "Slot / Zone:  " << outTx.slotId << " / " << outTx.zone << "\n";
cout << "Entry:        " << formatDateTime(outTx.entryDateTime) << "\n";
cout << "Exit:         " << formatDateTime(outTx.exitDateTime) << "\n";
cout << "Duration:     " << outTx.durationMinutes << " minutes\n";
cout << "Billed Hours: " << outTx.billedHours << " (partial hours rounded up)\n";
cout << "Rate Applied: " << outTx.ratePerHour << " RWF/hour\n";
cout << "Total Fee:    " << outTx.totalFee << " RWF\n";
cout << "----------------------------\n";
        return true;                                     // exit success
    }

    // Task 5: vehicle history by plate
    void displayVehicleHistory(const string& plate) const {
        if (isEmptyOrWhitespace(plate)) {                // empty plate check
            cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stop report
        }
        string normalizedPlate = normalizePlateNumber(plate); // normalize search plate
        if (normalizedPlate.empty()) {                 // invalid plate format
            cout << "Error: Invalid plate. Use 6-8 characters, no spaces or hyphens.\n";
            return;                                      // stop report
        }
        bool found = false;                              // match flag
        cout << "\n--- Parking History for '" << normalizedPlate << "' ---\n";
        for (size_t i = 0; i < transactionHistory_.size(); ++i) { // scan history
            const ParkingTransaction& tx = transactionHistory_[i]; // current record
            if (tx.plateNumber == normalizedPlate) {     // plate match
                found = true;                            // mark found
cout << formatDateTime(tx.entryDateTime) << " -> "
                          << formatDateTime(tx.exitDateTime) << " | "
                          << vehicleTypeToString(tx.vehicleType) << " | Slot: " << tx.slotId
                          << " | Billed: " << tx.billedHours << "h | Rate: " << tx.ratePerHour
                          << " RWF/h | Fee: " << tx.totalFee << " RWF\n";
            }
        }
        if (!found) cout << "No completed records found for this plate.\n";
cout << "\n";
    }

    // Task 5: all transaction history
    void displayAllHistory() const {
        if (transactionHistory_.empty()) {               // empty history
cout << "No completed parking transactions yet.\n";
            return;                                      // nothing to show
        }
cout << "\n--- All Transaction History ---\n";
        for (size_t i = 0; i < transactionHistory_.size(); ++i) { // traverse history
            const ParkingTransaction& tx = transactionHistory_[i]; // current tx
cout << (i + 1) << ". " << tx.plateNumber << " | "
                      << vehicleTypeToString(tx.vehicleType) << " | Slot: " << tx.slotId
                      << " | " << formatDateTime(tx.entryDateTime) << " -> "
                      << formatDateTime(tx.exitDateTime) << " | Fee: " << tx.totalFee
                      << " RWF (rate: " << tx.ratePerHour << ")\n";
        }
cout << "Total transactions: " << transactionHistory_.size() << "\n\n";
    }

    // Task 5: daily revenue report
    void displayDailyRevenue() const {
        long long total = 0;                             // revenue accumulator
        for (size_t i = 0; i < transactionHistory_.size(); ++i) { // sum fees
            total += transactionHistory_[i].totalFee;    // add each fee
        }
cout << "\n--- Daily Revenue Report ---\n";
cout << "Completed transactions: " << transactionHistory_.size() << "\n";
cout << "Total revenue:          " << total << " RWF\n\n";
    }

private:
unordered_map<string, ParkingSlot> slots_;              // slot id -> slot
unordered_map<string, VehicleEntry> activeVehicles_;   // plate -> active entry
vector<ParkingTransaction> transactionHistory_;               // completed sessions
unordered_map<VehicleType, int> currentPrices_;             // type -> active rate
vector<TariffPolicy*> tariffPolicies_;                      // polymorphic tariff objects

    bool findAvailableSlot(VehicleType type, string& outSlotId) const {
        for (unordered_map<string, ParkingSlot>::const_iterator it = slots_.begin();
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

    bool setSlotStatus(const string& slotId, SlotStatus status) {
unordered_map<string, ParkingSlot>::iterator it = slots_.find(slotId);
        if (it == slots_.end()) return false;            // slot missing
        it->second.status = status;                      // update status
        return true;                                     // success
    }

    bool getSlot(const string& slotId, ParkingSlot& outSlot) const {
unordered_map<string, ParkingSlot>::const_iterator it = slots_.find(slotId);
        if (it == slots_.end()) return false;            // slot missing
        outSlot = it->second;                            // copy slot data
        return true;                                     // success
    }
};

// Shows welcome banner at program start
void displayWelcome() {
cout << "\n================================================\n";
cout << "     KIGALI SMART PARKING MANAGEMENT SYSTEM     \n";
cout << "================================================\n";
    cout << " Default Rates: Motorcycle 500 | Car 1000 | Truck 2000 RWF/hr\n";
    cout << " Plate: 6-8 chars, no spaces or hyphens (Rwanda or foreign)\n";
    cout << " Date/time: TODAY only | past times OK | future time NOT allowed.\n";
cout << "================================================\n";
}

// Shows main menu; returns false only on read failure (should not exit program)
void displayMenu() {
cout << "\n=================== MAIN MENU ===================\n";
cout << " 1. Add Parking Slot\n";
cout << " 2. View All Parking Slots\n";
cout << " 3. View Available Slots\n";
cout << " 4. Register Vehicle Entry\n";
cout << " 5. Process Vehicle Exit\n";
cout << " 6. View Parked Vehicles\n";
cout << " 7. Update Parking Price\n";
cout << " 8. View Current Tariffs\n";
cout << " 9. Vehicle Parking History\n";
cout << "10. All Transaction History\n";
cout << "11. Daily Revenue Report\n";
cout << " 0. Exit\n";
cout << "=================================================\n";
}

// Reads menu choice safely; rejects 09, letters, decimals; ONLY exact 0 exits
bool readMenuChoice(int& choice) {
  displayMenu();                                         // show menu first
string line = readLine("Enter Choice: ");         // read full line as text
  line = trim(line);                                     // trim spaces
  if (line.empty()) {                                    // empty input
cout << "Invalid input! Please enter a valid choice (0-11).\n";
    return false;                                        // do not exit program
  }
  if (!parseStrictInteger(line, choice, 0, 11)) {        // strict parse 0-11
cout << "Invalid input! Please enter a valid choice (0-11).\n";
    return false;                                        // reject 09, abc, 1.5, etc.
  }
  return true;                                           // valid menu choice
}

// Prompts vehicle type 1-3 with strict validation
bool promptVehicleType(VehicleType& outType) {
cout << "Vehicle type: 1=Motorcycle  2=Car  3=Truck\n";
string line = readLine("Enter type (1-3): ");   // read as text
    int typeChoice = 0;                                  // parsed choice
    if (!parseStrictInteger(trim(line), typeChoice, 1, 3)) { // strict 1-3
cout << "Invalid input! Please enter 1, 2, or 3.\n";
        return false;                                    // invalid type input
    }
    if (typeChoice == 1) outType = VehicleType::MOTORCYCLE; // map to enum
    else if (typeChoice == 2) outType = VehicleType::CAR;   // map to enum
    else outType = VehicleType::TRUCK;                      // map to enum
    return true;                                         // success
}

// Prompts date and time; today only, no past/future dates, no future time
bool promptDateTime(const string& label, DateTime& outDt) {
    DateTime today = DateTime::now();                      // show today's date as hint
    char todayHint[12];                                    // buffer for DD-MM-YYYY
    snprintf(todayHint, sizeof(todayHint), "%02d-%02d-%04d", today.day, today.month, today.year);
    cout << "(Today: " << todayHint << " — use this date; past times OK, future time not allowed)\n";
    string line = trim(readLine(label));                   // read user line
    if (line.empty()) {                                    // empty check
        cout << "Error: Date/time cannot be empty.\n";
        return false;                                      // reject empty
    }
    if (!parseDateTime(line, outDt)) {                     // parse DD-MM-YYYY HH:MM
        cout << "Invalid format! Use DD-MM-YYYY HH:MM (e.g. " << todayHint << " 14:30).\n";
        return false;                                      // bad format
    }
    string dateError;                                      // validation message
    if (!validateParkingDateTime(outDt, dateError)) {      // date/time rules
        cout << dateError << "\n";
        return false;                                      // reject invalid datetime
    }
    return true;                                           // valid datetime
}

// Menu handler: add parking slot
void handleAddSlot(ParkingSystem& system) {
    try {                                                // exception guard
string slotId = trim(readLine("Enter Slot ID (e.g. C-A1): "));
        if (isEmptyOrWhitespace(slotId)) {               // empty validation
cout << "Error: Slot ID cannot be empty.\n";
            return;                                      // stay in program
        }
        VehicleType type;                                // selected type
        if (!promptVehicleType(type)) return;            // invalid type
string zone = trim(readLine("Enter Zone (e.g. Downtown): "));
        if (isEmptyOrWhitespace(zone)) {                 // empty zone
cout << "Error: Zone name cannot be empty.\n";
            return;                                      // stay in program
        }
        system.addParkingSlot(slotId, type, zone);       // delegate to system
    } catch (const exception& ex) {                 // catch runtime errors
cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: vehicle entry
void handleVehicleEntry(ParkingSystem& system) {
    try {                                                // exception guard
        string plate = trim(readLine("Enter plate number (6-8 chars): "));
        if (isEmptyOrWhitespace(plate)) {                // empty plate
            cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stay in program
        }
        VehicleType type;                                // selected type
        if (!promptVehicleType(type)) return;            // invalid type
        DateTime entryDt;                                // entry datetime
        if (!promptDateTime("Enter entry (DD-MM-YYYY HH:MM): ", entryDt)) return;
        system.registerVehicleEntry(plate, type, entryDt); // register entry
    } catch (const exception& ex) {                 // catch runtime errors
cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: vehicle exit
void handleVehicleExit(ParkingSystem& system) {
    try {                                                // exception guard
        string plate = trim(readLine("Enter plate number (6-8 chars): "));
        if (isEmptyOrWhitespace(plate)) {                // empty plate
            cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stay in program
        }
        DateTime exitDt;                                 // exit datetime
        if (!promptDateTime("Enter exit (DD-MM-YYYY HH:MM): ", exitDt)) return;
        ParkingTransaction receipt;                      // output receipt
        system.processVehicleExit(plate, exitDt, receipt); // process exit
    } catch (const exception& ex) {                 // catch runtime errors
cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: update parking price
void handlePriceUpdate(ParkingSystem& system) {
    try {                                                // exception guard
        VehicleType type;                                // type to update
        if (!promptVehicleType(type)) return;            // invalid type
string line = readLine("Enter new hourly price (RWF): "); // read price text
        int newPrice = 0;                                // parsed price
        if (!parseStrictInteger(trim(line), newPrice, 1, 1000000)) { // positive int
cout << "Invalid input! Enter a positive whole number.\n";
            return;                                      // stay in program
        }
        system.updateParkingPrice(type, newPrice);       // apply update
    } catch (const exception& ex) {                 // catch runtime errors
cout << "Unexpected error: " << ex.what() << "\n";
    }
}

// Menu handler: vehicle history search
void handleVehicleHistory(ParkingSystem& system) {
    try {                                                // exception guard
        string plate = trim(readLine("Enter plate to search (6-8 chars): "));
        if (isEmptyOrWhitespace(plate)) {                // empty plate
            cout << "Error: Plate number cannot be empty.\n";
            return;                                      // stay in program
        }
        system.displayVehicleHistory(plate);             // show history
    } catch (const exception& ex) {                 // catch runtime errors
cout << "Unexpected error: " << ex.what() << "\n";
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
cout << "Thank you for using Smart Parking System.\n";
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
                default:
cout << "Invalid input! Please enter a valid choice (0-11).\n";
                    break;                               // continue program
            }
        } catch (const exception& ex) {           // catch unexpected errors
cout << "System error: " << ex.what() << "\n";
        } catch (...) {                                  // catch unknown errors
cout << "Unknown error occurred. Program continues.\n";
        }
    }
    return 0;                                            // normal termination
}
