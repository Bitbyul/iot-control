/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    session: 클라이언트와의 연결 및 통신 담당
*/
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

#include "constants/message_type.h"

class Controller;
class DeviceEntry;

class Session {
    std::mutex send_mutex_; // 원자적 전송을 위한 뮤텍스

    Controller& controller_;
    uint16_t client_id_ = 0;

    int serv_sock;
    struct sockaddr_in serv_addr;
    int received_length;

    unsigned char recv_payload_buffer[65536];

public:

    Session(uint16_t preferred_client_id, Controller& controller);
    ~Session();

    uint8_t connect_server(const char* ip, const uint16_t port);
    uint8_t register_init_info();
    int *get_socket();
    void receive();
    void send(message_type mtype, std::shared_ptr<DeviceEntry> pDev, const void* payload, uint16_t payload_size);
    void disconnect();
};