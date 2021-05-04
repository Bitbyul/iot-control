#include "DevRepository.h"

#include "../constants/server_constants.h"
#include "../manager.h"
#include "../devices/DeviceBuilder.h"
#include "../tools/logger.h"

using namespace std;

DevRepository::DevRepository() {

}

/* 
    개별 장치 정보 트리에 신규 저장
*/
bool DevRepository::insert_to_tree(DeviceEntry* prawDev) {
    device_tree.insert(prawDev);
    return true;    
}
bool DevRepository::insert_to_tree(std::shared_ptr<DeviceEntry> pDev) {
    return insert_to_tree(pDev.get());
}
/*
    Address/ID로 장치 검색
*/
std::shared_ptr<DeviceEntry> DevRepository::get_by_address_ID(std::vector<uint16_t> address, uint16_t device_id) {

    auto device_list = get_all_devices();

    for(auto iter=device_list.begin(); iter!=device_list.end(); ++iter) {
        if((*iter)->address()==address && (*iter)->id()==device_id) {
            return *iter;
        }
    }
    return nullptr;
}

std::shared_ptr<DeviceEntry> DevRepository::get_test_device() {
    return device_tree.getAllDevices()[0];
}

std::vector<std::shared_ptr<DeviceEntry>> DevRepository::get_all_devices() {
    return device_tree.getAllDevices();
}