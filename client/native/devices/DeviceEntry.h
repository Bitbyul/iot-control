/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    DeviceEntry: 개별 디바이스 정보 관리 클래스
*/
#ifndef IOT_DEVICE_ENTRY_H
#define IOT_DEVICE_ENTRY_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <functional>

#include "../constants/device_type.h"
#include "../tools/logger.h"

class DeviceEntry {
    std::vector<uint16_t> address_; // 디바이스 주소
    uint16_t id_; // 장치 ID
    uint16_t client_id_; // 장치와 직결된 클라이언트 ID
    std::string alias_; // 별칭
    device_type type_; // 장치 종류

public:
    DeviceEntry(std::vector<uint16_t>& address_, uint16_t id, uint16_t client_id, std::string& alias, device_type type);
    ~DeviceEntry();
    std::vector<uint16_t>& address();
    std::string address_str();
    uint16_t id();
    uint16_t client_id();
    std::string alias();
    device_type type();

    std::string info_str();
    
    template<typename T>
    void actuator_operation(T data) {
        Logger::i("Actuator<", info_str(), "> set value as ", data);
    }

    bool sensor_bin_operation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 1);
        
        bool data = dis(gen);

        Logger::i("Sensor<", info_str(), "> now value is ", data);
        return data;
    }
    
    uint32_t sensor_num_operation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 100);

        uint32_t data = dis(gen);
        Logger::i("Sensor<", info_str(), "> now value is ", data);

        return data;
    }
};

#endif