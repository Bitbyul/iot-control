#include "session.h"
#include "controller.h"
#include "devices/DeviceEntry.h"
#include "tools/logger.h"

using namespace std;

Session::Session(uint16_t preferred_client_id, Controller& controller) : client_id_(preferred_client_id), controller_(controller), serv_sock(0) {
    Logger::i("Communicating Session is created.");
}

Session::~Session() {
    this->disconnect();
    Logger::i("Session is destroyed.");
}
/*
    연결된 소켓 반환
*/
int *Session::get_socket() {
    return &serv_sock;
}
/*
    서버로 연결
*/
uint8_t Session::connect_server(const char* ip, const uint16_t port) {
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1) return false;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);
	if (connect(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) return reason_code::REASON_SERVER_OFFLINE;

    uint8_t reason_code;
    reason_code = register_init_info();
    if(reason_code > 0) {
        if(reason_code == reason_code::REASON_ID_DUPLICATED) {
            Logger::i("A duplicated CLIENT_ID exists on the server.");
        }else {
            Logger::i("Client register failed. REASON_CODE: ", static_cast<unsigned>(reason_code));
        }
        return reason_code;
    }

    Logger::i("Client registered to server as ID <", client_id_, ">.");
    controller_.session_connected = true;

    // 수신용 쓰레드 생성
    thread session_receive_thread(&Session::receive, this);
    session_receive_thread.detach();

    return 0;
}
/*
    서버로 클라이언트의 초기 정보 전송
*/
uint8_t Session::register_init_info() {
    this->send(message_type::REGISTER_CLIENT, nullptr, NULL, 0); // Register Client
    
    uint16_t mtype;
    uint16_t client_id;
    uint16_t payload_length;
    uint8_t reason_code = reason_code::REASON_SERVER_ERROR;
    received_length=read(serv_sock,&mtype, sizeof(uint16_t)); // read MSG_TYPE
    received_length=read(serv_sock,&recv_payload_buffer, 4); // TRASH
    received_length=read(serv_sock,&client_id, sizeof(uint16_t)); // read CLIENT_ID
    received_length=read(serv_sock,&recv_payload_buffer, 3); // TRASH
    received_length=read(serv_sock,&payload_length, sizeof(uint16_t)); // read PAYLOAD_LENGTH
    if(payload_length > 0) {
        received_length=read(serv_sock,&reason_code, sizeof(uint8_t)); // read REASON_CODE
    }

    if (mtype==message_type::REGISTER_DONE) return 0;

    return reason_code;
}
/*
    서버로부터 받기
*/
void Session::receive() {
    
    uint16_t mtype;
    uint32_t timestamp;
    uint16_t client_id;
    uint8_t addr_depth;
    vector<uint16_t> address;
    uint16_t device_id;
    uint16_t payload_length;

    while (true) {
        address.clear();

        received_length=read(serv_sock,&mtype, sizeof(uint16_t)); // read MSG_TYPE
        if(received_length == 0) { // 클라이언트와 연결 종료

            close(serv_sock);
            Logger::i("Server<", inet_ntoa(serv_addr.sin_addr), "> disconnected.");
            controller_.session_connected = false;
            return;
        } else if(received_length == -1) { // 클라이언트와 연결 에러

            close(serv_sock);
            Logger::i("Server<", inet_ntoa(serv_addr.sin_addr), "> unexpectedly disconnected.");
            controller_.session_connected = false;
            return;
        }

        //std::cout << "MSG_TYPE: " << mtype << "\n";

        received_length=read(serv_sock,&timestamp, sizeof(uint32_t)); // read TIMESTAMP
        //std::cout << "TIMESTAMP: " << timestamp << "\n";
        received_length=read(serv_sock,&client_id, sizeof(uint16_t)); // read CLIENT_ID
        //std::cout << "CLIENT_ID: " << client_id << "\n";
        received_length=read(serv_sock,&addr_depth, sizeof(uint8_t)); // read ADDR_DEPTH
        //std::cout << "ADDR_DEPTH: " << static_cast<unsigned>(addr_depth) << "\n";


        //std::cout << "ADDRRESS: ";
        uint16_t address_token;
        for(int i=0; i<addr_depth; i++) {
            read(serv_sock,&address_token, sizeof(uint16_t)); // read DEVICE_ADDRESS
            address.push_back(address_token);
            //std::cout << address_token << "-";
        }
        //std::cout << "\n";
        
        received_length=read(serv_sock,&device_id, sizeof(uint16_t)); // read DEVICE_ID
        //std::cout << "DEVICE_ID: " << device_id << "\n";
        received_length=read(serv_sock,&payload_length, sizeof(uint16_t)); // read PAYLOAD_LENGTH
        //std::cout << "PAYLOAD_LENGTH: " << payload_length << "\n";

        if(payload_length > 0) { // 받을 데이터가 있으면
            received_length=read(serv_sock,&recv_payload_buffer, payload_length);
        }
        
        if(received_length == 0) { // 클라이언트와 연결 종료

            close(serv_sock);
            Logger::i("Server<", inet_ntoa(serv_addr.sin_addr), "> disconnected.");
            controller_.session_connected = false;
            return;
        } else if(received_length == -1) { // 클라이언트와 연결 에러

            close(serv_sock);
            Logger::i("Server<", inet_ntoa(serv_addr.sin_addr), "> unexpectedly disconnected.");
            controller_.session_connected = false;
            return;
        }

        /* handler thread create*/
        thread handler_thread(&Controller::handle_message, &controller_, this, static_cast<message_type>(mtype), address, device_id, recv_payload_buffer, payload_length);
        handler_thread.detach();
    }
}
/*
    서버로 전송
*/
void Session::send(message_type mtype, shared_ptr<DeviceEntry> pDev, const void* payload, uint16_t payload_size) {
    unique_lock<mutex> lk(send_mutex_); // 상호 배제

    uint16_t mtype_raw = mtype;
    uint32_t timestamp = 0x12345678;

    write(serv_sock, &mtype_raw, sizeof(uint16_t)); // response type
    write(serv_sock, &timestamp, sizeof(uint32_t)); // timestamp
    write(serv_sock, &client_id_, sizeof(uint16_t)); // client id

    if(pDev==nullptr) {
        uint8_t addr_depth = 0;
        write(serv_sock, &addr_depth, sizeof(uint8_t)); // device address depth
    } else {
        uint8_t addr_depth = pDev->address().size();
        write(serv_sock, &addr_depth, sizeof(uint8_t)); // device address depth
    }
    
    if(pDev==nullptr) {
        uint16_t device_id = 0;
        write(serv_sock, &device_id, sizeof(uint16_t)); // device ID
    } else {
        uint16_t address_token;
        for(auto iter=pDev->address().begin(); iter!=pDev->address().end(); ++iter) {
            address_token = *iter;
            write(serv_sock, &address_token, sizeof(uint16_t)); // each device address token
        }
        uint16_t device_id = pDev->id();
        write(serv_sock, &device_id, sizeof(uint16_t)); // device ID
    }

    write(serv_sock, &payload_size, sizeof(uint16_t)); // payload length

    if(payload_size > 0) {
        write(serv_sock, payload, payload_size); // payload
    }

    lk.unlock();
}
/*
    서버와 연결 종료
*/
void Session::disconnect() {
    shutdown(serv_sock, SHUT_RDWR);
    controller_.session_connected = false;
}