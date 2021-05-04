#ifndef IOT_DEVICE_TYPE_H
#define IOT_DEVICE_TYPE_H

/* C compatible */
enum device_type {
    NOT_SET = 0, // 지정되지 않음

    SENSOR_BINARY = 10, // ON,OFF 센서
    SENSOR_NUMBER = 11, // 수치형 정보를 반환하는 센서
    SENSOR_TEXT = 12, // 문자열을 반환하는 센서

    ACTUATOR_BINARY = 20, // ON,OFF 액추에이터
    ACTUATOR_NUMBER = 21, // 수치형 정보를 받아 동작을 수행하는 액추에이터
    ACTUATOR_TEXT = 22, // 문자열 정보를 받아 동작을 수행하는 액추에이터
};
#endif