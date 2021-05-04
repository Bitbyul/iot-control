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

#include "../constants/device_type.h"

class DeviceEntry {
    std::vector<uint16_t> address_; // 디바이스 주소
    uint16_t id_; // 장치 ID
    uint16_t client_id_; // 장치와 직결된 클라이언트 ID
    std::string alias_; // 별칭
    device_type type_; // 장치 종류

    bool sensor_data_received = false;

    std::string sensor_txt_data = "";
    bool sensor_bin_data = false;
    uint32_t sensor_num_data = 0;

public:
    DeviceEntry(std::vector<uint16_t>& address_, uint16_t id, uint16_t client_id, std::string& alias, device_type type);
    ~DeviceEntry();
    void set_sensor_data_received();
    std::string get_sensor_txt_data();
    bool get_sensor_bin_data();
    uint32_t get_sensor_num_data();
    void set_sensor_txt_data(std::string& data);
    void set_sensor_bin_data(bool data);
    void set_sensor_num_data(uint32_t data);

    std::vector<uint16_t>& address();
    std::string address_str();
    uint16_t id();
    uint16_t client_id();
    std::string alias();
    device_type type();

    std::string info_str();
};

#endif