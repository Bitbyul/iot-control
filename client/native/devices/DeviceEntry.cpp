#include "DeviceEntry.h"

#include "../tools/datatype.h"

DeviceEntry::DeviceEntry(std::vector<uint16_t>& address, uint16_t id, uint16_t client_id, std::string& alias, device_type type)
    : address_(address), id_(id), client_id_(client_id_), alias_(alias), type_(type)
{ 
    std::cout << "Device <" << address_str() << "." << id_ << ">[" << alias_ << "] created.\n";
}
DeviceEntry::~DeviceEntry() {
    std::cout << "Device <" << id_ << "> destroyed.\n";
}

std::vector<uint16_t>& DeviceEntry::address() {
    return address_;
}
std::string DeviceEntry::address_str() {
    return DataType::address_to_str(address_);
}

uint16_t DeviceEntry::id() {
    return id_;
}

uint16_t DeviceEntry::client_id() {
    return client_id_;
}

std::string DeviceEntry::alias() {
    return alias_;
}

device_type DeviceEntry::type() {
    return type_;
}

std::string DeviceEntry::info_str() {
    std::stringstream ss{};

    ss << DataType::address_to_str(address_) << "." << id_ << ":" << alias_;

    return ss.str();
}