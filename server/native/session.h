/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    session: 클라이언트와의 연결 및 통신 담당
    IPC: 파이썬 웹 GUI와의 연결 및 통신 담당
*/
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

#include "constants/server_constants.h"
#include "constants/message_type.h"

class Manager;
class DeviceEntry;

class Session {
    std::mutex send_mutex_; // 원자적 전송을 위한 뮤텍스

protected:
    Manager& manager_;
    uint16_t client_id_;
    bool registered = false;

    int clnt_sock;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;
    int received_length;

    unsigned char recv_payload_buffer[65536];

public:

    Session();
    virtual ~Session();

    int *get_socket();
    char *get_ip();
    uint16_t get_client_id();
    struct sockaddr_in *get_addr();
    virtual bool accept_client(int serv_sock);
    bool receive_init_info();
    virtual void receive();
    virtual void send(message_type mtype, std::shared_ptr<DeviceEntry> pDev, const void* payload, uint16_t payload_size);
    virtual void send(message_type mtype, DeviceEntry* pDev, const void* payload, uint16_t payload_size);
    void disconnect();
};


class IPC : public Session {

public:
    IPC();
    virtual bool accept_client(int serv_sock);
    virtual void receive();
    void handle_message(message_type mtype, uint16_t client_id, std::vector<uint16_t> address, uint16_t device_id, const unsigned char* buffer, uint16_t received_length);
};