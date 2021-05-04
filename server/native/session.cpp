#include "session.h"
#include "manager.h"
#include "devices/DeviceEntry.h"
#include "tools/logger.h"

using namespace std;

Session::Session() : manager_(Manager::get_instance()), clnt_sock(0) {
    Logger::i("New Listening Session is created.");
}

Session::~Session() {
    this->disconnect();
    if(registered)
        manager_.unregist_session(this);
    Logger::i("Session is destroyed.");
}
/*
    연결된 소켓 반환
*/
int *Session::get_socket() {
    return &clnt_sock;
}
/*
    클라이언트의 IP 반환
*/
char *Session::get_ip() {
    return inet_ntoa(clnt_addr.sin_addr);
}
/*
    클라이언트의 주소체계 반환
*/
struct sockaddr_in *Session::get_addr() {
    return &clnt_addr;
}
/*
    클라이언트 ID 반환
*/
uint16_t Session::get_client_id() {
    return client_id_;
}

/*
    클라이언트와 연결 수립
*/
bool Session::accept_client(int serv_sock) {
    clnt_addr_size=sizeof(clnt_addr);
    clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);

    Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> connected.");

    return receive_init_info();
}
/*
    클라이언트 초기 정보 받기 & 설정
*/
bool Session::receive_init_info() {
    std::this_thread::sleep_for(300ms); // Troble Shooting ......
    uint16_t mtype;
    uint16_t client_id;
    uint16_t payload_length;

    received_length=read(clnt_sock,&mtype, sizeof(uint16_t)); // read MSG_TYPE
    received_length=read(clnt_sock,&recv_payload_buffer, 4); // TRASH
    received_length=read(clnt_sock,&client_id, sizeof(uint16_t)); // read CLIENT_ID
    received_length=read(clnt_sock,&recv_payload_buffer, 5); // TRASH

    
    if(received_length == 0) { // 클라이언트와 연결 종료

        close(clnt_sock);
        Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> disconnected.");
        return false;
    } else if(received_length == -1) { // 클라이언트와 연결 에러

        close(clnt_sock);
        Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> unexpectedly disconnected.");
        return false;
    }

    if (mtype!=message_type::REGISTER_CLIENT) return false;

    client_id_ = client_id;

    if(manager_.regist_session(this)) {
        registered = true;
        this->send(message_type::REGISTER_DONE, nullptr, NULL, 0);

        Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> set id to <",  client_id_, ">");
        return true;
    } else{
        uint8_t reason_code = reason_code::REASON_ID_DUPLICATED;
        this->send(message_type::REGISTER_FAILED, nullptr, &reason_code, sizeof(uint8_t));
        return false;
    }
}

/*
    클라이언트로부터 받기
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

        received_length=read(clnt_sock,&mtype, sizeof(uint16_t)); // read MSG_TYPE
        if(received_length == 0) { // 클라이언트와 연결 종료

            close(clnt_sock);
            Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> disconnected.");
            return;
        } else if(received_length == -1) { // 클라이언트와 연결 에러

            close(clnt_sock);
            Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> unexpectedly disconnected.");
            return;
        }

        //std::cout << "MSG_TYPE: " << mtype << "\n";

        received_length=read(clnt_sock,&timestamp, sizeof(uint32_t)); // read TIMESTAMP
        //std::cout << "TIMESTAMP: " << timestamp << "\n";
        
        received_length=read(clnt_sock,&client_id, sizeof(uint16_t)); // read CLIENT_ID
        //std::cout << "CLIENT_ID: " << client_id << "\n";
        received_length=read(clnt_sock,&addr_depth, sizeof(uint8_t)); // read ADDR_DEPTH
        //std::cout << "ADDR_DEPTH: " << static_cast<unsigned>(addr_depth) << "\n";


        //std::cout << "ADDRRESS: ";
        uint16_t address_token;
        for(int i=0; i<addr_depth; i++) {
            read(clnt_sock,&address_token, sizeof(uint16_t)); // read DEVICE_ADDRESS
            address.push_back(address_token);
            //std::cout << address_token << "-";
        }
        //std::cout << "\n";
        
        received_length=read(clnt_sock,&device_id, sizeof(uint16_t)); // read DEVICE_ID
        //std::cout << "DEVICE_ID: " << device_id << "\n";
        received_length=read(clnt_sock,&payload_length, sizeof(uint16_t)); // read PAYLOAD_LENGTH
        //std::cout << "PAYLOAD_LENGTH: " << payload_length << "\n";

        if(payload_length > 0) { // 받을 데이터가 있으면
            received_length=read(clnt_sock,&recv_payload_buffer, payload_length);
        }
        
        if(received_length == 0) { // 클라이언트와 연결 종료

            close(clnt_sock);
            Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> disconnected.");
            return;
        } else if(received_length == -1) { // 클라이언트와 연결 에러

            close(clnt_sock);
            Logger::i("Client<", inet_ntoa(clnt_addr.sin_addr), "> unexpectedly disconnected.");
            return;
        }
        
        manager_.handle_message(this, static_cast<message_type>(mtype), address, device_id, recv_payload_buffer, payload_length);
        /* handler thread create*/
        //thread handler_thread(&Manager::handle_message, &manager_, this, recv_payload_buffer, payload_length);
        //handler_thread.detach();
    }
}
/*
    클라이언트로 전송
*/
void Session::send(message_type mtype, std::shared_ptr<DeviceEntry> pDev, const void* payload, uint16_t payload_size) {
    send(mtype, pDev.get(), payload, payload_size);
}

void Session::send(message_type mtype, DeviceEntry* pDev, const void* payload, uint16_t payload_size) {
    
    unique_lock<mutex> lk(send_mutex_); // 상호 배제

    uint16_t mtype_raw = mtype;
    uint32_t timestamp = 0;

    write(clnt_sock, &mtype_raw, sizeof(uint16_t)); // response type
    write(clnt_sock, &timestamp, sizeof(uint32_t)); // timestamp
    if(pDev==nullptr) {
        write(clnt_sock, &client_id_, sizeof(uint16_t)); // client id
    }else {
        uint16_t device_client_id = pDev->client_id();
        write(clnt_sock, &device_client_id, sizeof(uint16_t)); // Device's client id
    }

    if(pDev==nullptr) {
        uint8_t addr_depth = 0;
        write(clnt_sock, &addr_depth, sizeof(uint8_t)); // device address depth
    } else {
        uint8_t addr_depth = pDev->address().size();
        write(clnt_sock, &addr_depth, sizeof(uint8_t)); // device address depth
    }

    if(pDev==nullptr) {
        uint16_t device_id = 0;
        write(clnt_sock, &device_id, sizeof(uint16_t)); // device ID
    } else {
        uint16_t address_token;
        for(auto iter=pDev->address().begin(); iter!=pDev->address().end(); ++iter) {
            address_token = *iter;
            write(clnt_sock, &address_token, sizeof(uint16_t)); // each device address token
        }

        uint16_t device_id = pDev->id();
        write(clnt_sock, &device_id, sizeof(uint16_t)); // device ID
    }

    write(clnt_sock, &payload_size, sizeof(uint16_t)); // payload length
    if(payload_size > 0) {
        write(clnt_sock, payload, payload_size); // payload
    }

    lk.unlock();
}
/*
    클라이언트와 연결 종료
*/
void Session::disconnect() {
    close(clnt_sock);
}