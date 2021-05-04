#include "DeviceBuilder.h"
#include "DeviceEntry.h"

#include "../tools/datatype.h"

DeviceBuilder::DeviceBuilder() {

}

DeviceBuilder& DeviceBuilder::address(const std::vector<uint16_t>& address) {
    address_ = address;
}
DeviceBuilder& DeviceBuilder::address(const char* address_str_arg) {
    address_ = DataType::str_to_address(address_str_arg);
    return *this;
}
DeviceBuilder& DeviceBuilder::id(uint16_t id_arg) {
    id_ = id_arg;
    return *this;
}
DeviceBuilder& DeviceBuilder::client_id(uint16_t client_id_arg) {
    client_id_ = client_id_arg;
    return *this;
}
DeviceBuilder& DeviceBuilder::alias(const char* alias_arg) {
    alias_ = alias_arg;
    return *this;
}
DeviceBuilder& DeviceBuilder::type(device_type type_arg) {
    type_ = type_arg;
    return *this;
}

DeviceEntry* DeviceBuilder::build() {
    if(id_==-1) { // id is not set.
        return nullptr;
    }
    if(address_.size()==0) { // address is not set.
        return nullptr;
    }

    return new DeviceEntry(address_, id_, client_id_, alias_, type_);
}