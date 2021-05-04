
#include "session.h"

#include "manager.h"
#include "database/DevRepository.h"
#include "tools/logger.h"

IPC::IPC() : Session() {
    client_id_ = 0;
    Logger::i("IPC Session is created.");
}

bool IPC::accept_client(int serv_sock) {
    clnt_addr_size=sizeof(clnt_addr);
    clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);

    Logger::i("IPC python connected.");

    return true;
}

// from Session::receive()
void IPC::receive() {
    uint16_t mtype;
    uint32_t timestamp;
    uint16_t client_id;
    uint8_t addr_depth;
    std::vector<uint16_t> address;
    uint16_t device_id;
    uint16_t payload_length;

    while (true) {
        address.clear();

        received_length=read(clnt_sock,&mtype, sizeof(uint16_t)); // read MSG_TYPE
        if(received_length == 0) { // 클라이언트와 연결 종료

            close(clnt_sock);
            Logger::i("IPC disconnected.");
            return;
        } else if(received_length == -1) { // 클라이언트와 연결 에러

            close(clnt_sock);
            Logger::i("IPC unexpectedly disconnected.");
            return;
        }

        //std::cout << "MSG_TYPE: " << mtype << "\n";

        received_length=read(clnt_sock,&timestamp, sizeof(uint32_t)); // read TIMESTAMP
        //std::cout << "TIMESTAMP: " << timestamp << "\n";
        
        received_length=read(clnt_sock,&client_id, sizeof(uint16_t)); // read CLIENT_ID
        //std::cout << "CLIENT_ID: " << client_id << "\n";
        received_length=read(clnt_sock,&addr_depth, sizeof(uint8_t)); // read ADDR_DEPTH
       // std::cout << "ADDR_DEPTH: " << static_cast<unsigned>(addr_depth) << "\n";


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
        
        handle_message(static_cast<message_type>(mtype), client_id, address, device_id, recv_payload_buffer, payload_length);
        /* handler thread create*/
        //thread handler_thread(&Manager::handle_message, &manager_, this, recv_payload_buffer, payload_length);
        //handler_thread.detach();
    }
}

/* 
    들어온 메시지 핸들링
*/
void IPC::handle_message(message_type mtype, uint16_t client_id, std::vector<uint16_t> address, uint16_t device_id, const unsigned char* buffer, uint16_t received_length) {
    if(mtype==message_type::IPC_REQ_ALL_DEVICE_INFO) { // 모든 기기 정보 요청
        
        std::shared_ptr<char> payload_buffer(new char[100](), [](char *p) { delete [] p; }); // own deleter

        auto device_list = manager_.pDevRepo_->get_all_devices();
        for(auto iter=device_list.begin(); iter!=device_list.end(); ++iter) {
            
            uint8_t device_type = (*iter)->type();
            uint8_t device_alias_len = (*iter)->alias().size();
            std::copy(&device_type, &device_type+1, payload_buffer.get());
            std::copy(&device_alias_len, &device_alias_len+1, payload_buffer.get()+sizeof(uint8_t));
            auto alias_cstr = (*iter)->alias().c_str();
            std::copy(alias_cstr, alias_cstr+device_alias_len, payload_buffer.get()+2*sizeof(uint8_t));

            this->send(message_type::IPC_RESP_DEVICE_INFO, iter->get(), payload_buffer.get(), 2+device_alias_len);
        }
    }

    if(mtype==message_type::REQ_ACTUATOR_OPERATION) { // 액추에이터 동작 실행
        auto prawSession = manager_.get_session_by_id(client_id);
        if(prawSession!=nullptr) {
            auto pDev = manager_.pDevRepo_->get_by_address_ID(address, device_id);
            if(pDev!=nullptr) {
                Logger::i("Actuator<", pDev->info_str(), "> operation requested.");
                prawSession->send(message_type::REQ_ACTUATOR_OPERATION, pDev, buffer, received_length);
            }else{
                Logger::i("장치 찾을 수 없음!");
            }

        }else{
            Logger::i("장치가 연결된 세션 찾을 수 없음!");
        }
    }
}