#ifndef IOT_MESSAGE_TYPE_H
#define IOT_MESSAGE_TYPE_H

/* C compatible */
enum message_type {
    PING = 0x0000, // 생존 체크
    PONG = 0x0001, // 생존 응답
    REGISTER_CLIENT = 0x0010, // 클라이언트 등록
    REGISTER_DEVICE = 0x0011, // 장치 등록
    REGISTER_DONE = 0x0012, // 등록 완료
    REGISTER_FAILED = 0x0013, // 등록 실패
    UNREGISTER_CLIENT = 0x0014, // 클라이언트 등록 해제
    UNREGISTER_DEVICE = 0x0015, // 장치 등록 해제
    REQ_SENSOR_DATA = 0x0020,  // 센서 데이터 요구
    RESP_SENSOR_DATA = 0x0021, // 센서 데이터 응답 (동기, 비정기적)
    SENSOR_DATA = 0x0022, // 센서 데이터 (비동기, 정기적)
    REQ_ACTUATOR_OPERATION = 0x0023, // 액추에이터 동작 실행 요구
    RESP_ACTUATOR_OPERATION = 0x0024, // 액추에이터 동작 실행 응답(결과)
};

enum reason_code {
    REASON_SERVER_OFFLINE = 1, // 서버가 꺼져있음
    REASON_SERVER_ERROR = 2, // 서버 커넥션 에러
    REASON_ID_DUPLICATED = 3, // 중복된 ID의 장치 또는 클라이언트 존재
    REASON_ALEADY_REGISTERED = 4, // 이미 등록되어 있음
    REASON_CLIENT_NOT_REGISTERED = 5 // 장치가 연결된 클라이언트가 등록되어 있지 않음
};
enum response_code {
    OK = 0, // 정상
    
    DEVICE_NOT_EXIST = 1, // 장치 존재하지 않음
    DEVICE_TYPE_MISMATCH = 2, // 장치 타입 불일치
    CANT_GET_FROM_DEVICE = 3, // 장치에서 정보 얻어오기 실패
    CANT_OPERATE = 4, // 액추에이터 동작 실패
};
#endif