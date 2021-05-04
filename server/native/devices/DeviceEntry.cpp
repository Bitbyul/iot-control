#include "DeviceEntry.h"

#include "../tools/datatype.h"

DeviceEntry::DeviceEntry(std::vector<uint16_t>& address, uint16_t id, uint16_t client_id, std::string& alias, device_type type)
    : address_(address), id_(id), client_id_(client_id), alias_(alias), type_(type)
{ 
    std::cout << "Device " << address_str() << "." << id_ << "<" << client_id_ << "> type:" << type_ << "[" << alias_ << "] created.\n";
}
DeviceEntry::~DeviceEntry() {
    std::cout << "Device <" << id_ << "> destroyed.\n";
}

void DeviceEntry::set_sensor_data_received() {
    sensor_data_received = true;
}

std::string DeviceEntry::get_sensor_txt_data(){
    return sensor_txt_data;
}
bool DeviceEntry::get_sensor_bin_data(){
    return sensor_bin_data;
}
uint32_t DeviceEntry::get_sensor_num_data(){
    return sensor_num_data;
}
void DeviceEntry::set_sensor_txt_data(std::string& data){
    sensor_txt_data = data;
}
void DeviceEntry::set_sensor_bin_data(bool data){
    sensor_bin_data = data;
}
void DeviceEntry::set_sensor_num_data(uint32_t data){
    sensor_num_data = data;
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