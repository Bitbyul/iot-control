#include "controller.h"

#include "tools/logger.h"
#include "tools/datatype.h"
#include "session.h"
#include "devices/DeviceBuilder.h"
#include "devices/DeviceEntry.h"

using namespace std::chrono_literals;

Controller::Controller() {
    DeviceBuilder builder;
    /*
    // 60주념기념관 중앙, 랩실
    deviceList.emplace_back(builder.address("1-18").id(1).alias("중앙온도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18").id(2).alias("중앙습도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18").id(3).alias("중앙조도").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18").id(4).alias("중앙전광판").type(device_type::ACTUATOR_TEXT).build());
    deviceList.emplace_back(builder.address("1-18").id(5).alias("중앙에어컨").type(device_type::ACTUATOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18").id(6).alias("중앙불켜기").type(device_type::ACTUATOR_BINARY).build());
    
    deviceList.emplace_back(builder.address("1-18-205").id(1).alias("모컨온도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18-205").id(2).alias("모컨습도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18-205").id(3).alias("모션인식").type(device_type::SENSOR_BINARY).build());
    deviceList.emplace_back(builder.address("1-18-205").id(4).alias("알림음").type(device_type::ACTUATOR_BINARY).build());
    */
    /*
    // 60주념기념관 강의실
    deviceList.emplace_back(builder.address("1-18-422").id(1).alias("온도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18-422").id(2).alias("습도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18-422").id(6).alias("문잠금").type(device_type::ACTUATOR_BINARY).build());
    deviceList.emplace_back(builder.address("1-18-424").id(1).alias("온도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18-424").id(2).alias("습도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("1-18-424").id(6).alias("문잠금").type(device_type::ACTUATOR_BINARY).build());
    */
    // 서울캠퍼스
    deviceList.emplace_back(builder.address("2-1-1").id(1).alias("조도센서").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("2-1-1").id(2).alias("문잠금").type(device_type::ACTUATOR_BINARY).build());
    deviceList.emplace_back(builder.address("2-1").id(6).alias("온도계").type(device_type::SENSOR_NUMBER).build());
    deviceList.emplace_back(builder.address("2-2").id(1).alias("문잠금").type(device_type::ACTUATOR_BINARY).build());
}

void Controller::register_device(Session *session) {
    
    std::shared_ptr<char> payload_buffer(new char[100](), [](char *p) { delete [] p; }); // own deleter

    auto dev_list = get_all_device();

    for(auto iter=dev_list.begin(); iter!=dev_list.end(); ++iter) {
        auto& pDev = *iter;
        uint8_t device_type = pDev->type();
        uint8_t device_alias_len = pDev->alias().size();
        std::copy(&device_type, &device_type+1, payload_buffer.get());
        std::copy(&device_alias_len, &device_alias_len+1, payload_buffer.get()+sizeof(uint8_t));
        std::copy(pDev->alias().c_str(), pDev->alias().c_str()+device_alias_len, payload_buffer.get()+2*sizeof(uint8_t));

        session->send(message_type::REGISTER_DEVICE, pDev, payload_buffer.get(), 2+device_alias_len);
    }
}

void Controller::enable_sensor_timer(Session* session) {
    
    // 정기적 센서 데이터 전송용 쓰레드 생성
    //std::thread sensor_data_thread(&Controller::sensor_data_routine, this, session);
    //sensor_data_thread.detach();
    sensor_data_routine(session);
}

void Controller::sensor_data_routine(Session* session) {
    
    while(true) {
        if(!session_connected) break;

        std::shared_ptr<char> payload_buffer(new char[100](), [](char *p) { delete [] p; }); // own deleter

        auto dev_list = get_all_device();

        for(auto iter=dev_list.begin(); iter!=dev_list.end(); ++iter) {
            auto& pDev = *iter;
            uint8_t device_type = pDev->type();
            
            std::copy(&device_type, &device_type+1, payload_buffer.get()); // DEVICE_TYPE

            if(device_type == device_type::SENSOR_BINARY) {
                uint8_t data = pDev->sensor_bin_operation();
                std::memcpy(payload_buffer.get()+sizeof(uint8_t), &data, sizeof(uint8_t));

                session->send(message_type::SENSOR_DATA, pDev, payload_buffer.get(), 2*sizeof(uint8_t));
            }else if(device_type == device_type::SENSOR_NUMBER) {
                uint32_t data = pDev->sensor_num_operation();
                std::memcpy(payload_buffer.get()+sizeof(uint8_t), &data, sizeof(uint32_t));

                session->send(message_type::SENSOR_DATA, pDev, payload_buffer.get(), sizeof(uint8_t)+sizeof(uint32_t));

            // TODO: TEXT SENSOR
            }else { // Actuator
                session->send(message_type::PONG, pDev, NULL, 0);
            }
        }
        std::this_thread::sleep_for(10s);


    }
}

void Controller::handle_message(Session *session, message_type mtype, std::vector<uint16_t> address, uint16_t device_id, unsigned char* buffer, uint16_t received_length) {
    if(mtype == message_type::REQ_ACTUATOR_OPERATION) { // 액추에이터 동작 요구
        for(auto iter=deviceList.begin(); iter!=deviceList.end(); ++iter) {
            if((*iter)->address() == address && (*iter)->id() == device_id) {
                device_type dtype = (*iter)->type();
                if(dtype==device_type::ACTUATOR_BINARY) {
                    (*iter)->actuator_operation(static_cast<bool>(*(buffer+1)));
                }else if(dtype==device_type::ACTUATOR_NUMBER) {
                    (*iter)->actuator_operation(static_cast<uint32_t>(*(buffer+1)));
                }else {
                    std::string data_str(buffer+1, buffer+received_length);
                    (*iter)->actuator_operation(data_str);
                }
                break;
            }
        }
    }
    return;
}

std::vector<std::shared_ptr<DeviceEntry>> Controller::get_all_device() {
    return deviceList;
}

std::shared_ptr<DeviceEntry> Controller::get_test_device() {
    return deviceList[0];
}