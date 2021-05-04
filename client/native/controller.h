#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

#include "constants/message_type.h"
#include "constants/device_type.h"

class Session;
class DeviceEntry;

class Controller {

    std::vector<std::shared_ptr<DeviceEntry>> deviceList;

public:
    Controller();
    volatile bool session_connected = false;
    void register_device(Session *session);
    void enable_sensor_timer(Session* session);
    void sensor_data_routine(Session* session);
    void handle_message(Session *session, message_type mtype, std::vector<uint16_t> address, uint16_t device_id, unsigned char* buffer, uint16_t received_length);
    std::vector<std::shared_ptr<DeviceEntry>> get_all_device();
    std::shared_ptr<DeviceEntry> get_test_device();
};