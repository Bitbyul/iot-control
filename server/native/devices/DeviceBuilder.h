#include <memory>
#include <vector>
#include "../constants/device_type.h"

class DeviceEntry;

/*
    For builder pattern
*/
class DeviceBuilder {
    std::vector<uint16_t> address_;
    uint16_t id_ = -1;
    uint16_t client_id_ = -1;
    std::string alias_ = "";
    device_type type_;

public:
    DeviceBuilder();
    DeviceBuilder& address(const std::vector<uint16_t>& address);
    DeviceBuilder& address(const char*);
    DeviceBuilder& id(uint16_t);
    DeviceBuilder& client_id(uint16_t);
    DeviceBuilder& alias(const char*);
    DeviceBuilder& type(device_type);

    DeviceEntry* build(); 
};
