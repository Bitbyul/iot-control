/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    datatype: 데이터 형과 관련한 도구 모음
*/
#ifndef IOT_DATATYPE_H
#define IOT_DATATYPE_H

#include <string>
#include <vector>
#include <sstream>

namespace DataType {
    std::vector<uint16_t> str_to_address(std::string input);
    std::string address_to_str(std::vector<uint16_t> address);
};

#endif