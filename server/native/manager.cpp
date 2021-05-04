#include "manager.h"

#include "tools/logger.h"
#include "tools/datatype.h"
#include "session.h"
#include "database/DevRepository.h"
#include "devices/DeviceBuilder.h"

using namespace std;

mutex Manager::singleton_mutex_;
Manager* Manager::instance_ = nullptr;

/*
    Manager 생성자
    싱글턴 패턴으로 인해 최초 1번만 호출
*/
Manager::Manager() : serv_sock(0), pDevRepo_(new DevRepository) {
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(7143);

    instance_ = this;

    Logger::i("Manager is created.");
}

/*
    싱글턴 패턴 인스턴스 가져오기
*/
Manager& Manager::get_instance(){

    unique_lock<mutex> lk(singleton_mutex_); // 상호 배제
    if(instance_==nullptr) {
        instance_ = new Manager();
    }
    lk.unlock();

    return *instance_;
}

/*
    네트워크 대기 시작
*/
bool Manager::start_listen(){

    Logger::i("Preparing to listen...");

    serv_sock=socket(PF_INET,SOCK_STREAM,0);
    if(serv_sock==-1){
        Logger::e("socket() error.");
        return false;
    }

    int optval = 1;
    setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)); // bind() error 회피

    if(bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1){
	    Logger::e("bind() error");
        return false;
    }

    if(listen(serv_sock,1)==-1){
        Logger::e("listen() error");
        return false;
    }
    
    Logger::i("IP: ", inet_ntoa(serv_addr.sin_addr), " / Port: ", ntohs(serv_addr.sin_port));


    ipc_ = make_shared<IPC>();
    Logger::i("Waiting for python IPC...");
    if(ipc_->accept_client(serv_sock)){ 
        thread ipc_receive_thread(&IPC::receive, ipc_);
        ipc_receive_thread.detach();
    }

    Logger::i("Listening...");
    while (1) {
        auto session = make_shared<Session>();
        
        if(session->accept_client(serv_sock)){ 
            thread session_receive_thread(&Session::receive, session);
            session_receive_thread.detach();
        }
    }

    return true;
}
/*
    세션 리스트에 세션 정보 등록
*/
bool Manager::regist_session(Session *session) {
    for(auto iter=session_list_.begin(); iter!=session_list_.end(); ++iter) { // 중복ID 검사
        if(iter->first == session->get_client_id())
            return false;
    }
    session_list_.push_back(std::make_pair(session->get_client_id(), session));
    Logger::i("Client<", session->get_client_id(), ",", session->get_ip(), ">", " registered.");
    return true;
}
/*
    세션 리스트에 세션 정보 삭제
*/
void Manager::unregist_session(Session *session) {
    for(auto iter=session_list_.begin(); iter!=session_list_.end(); ++iter) {
        if(iter->first == session->get_client_id()) {
            session_list_.erase(iter);
            break;
        }
    }
    Logger::i("Client<", session->get_client_id(), ",", session->get_ip(), ">", " unregistered.");
}
/*
    클라이언트 ID로 세선 얻어오기
*/
Session* Manager::get_session_by_id(uint16_t client_id) {
    for(auto iter=session_list_.begin(); iter!=session_list_.end(); ++iter) {
        if(iter->first == client_id) {
            return iter->second;
        }
    }
    return nullptr;
}

/* 
    들어온 메시지 핸들링
*/
void Manager::handle_message(Session *session, message_type mtype, std::vector<uint16_t> address, uint16_t device_id, unsigned char* buffer, uint16_t received_length) {
    //Logger::i("handle_message()");

    if(mtype == message_type::REGISTER_DEVICE) { // 장치 등록
        DeviceBuilder builder;
        uint8_t device_type_v = buffer[0];
        uint8_t device_alias_len = buffer[1];
        std::string alias_str(buffer+2, buffer+2+device_alias_len);
        auto rawpDev = builder.address(address).id(device_id).client_id(session->get_client_id()).alias(alias_str.c_str()).type(static_cast<device_type>(device_type_v)).build();
        
        if(pDevRepo_->insert_to_tree(rawpDev)) {
            session->send(message_type::REGISTER_DONE, rawpDev, NULL, 0);

            // Send to Python
            ipc_->send(message_type::IPC_RESP_DEVICE_INFO, rawpDev, buffer, received_length);
        } else{ // 중복된 장치 존재
            // TODO
        }
    }else if(mtype == message_type::SENSOR_DATA) { // 센서 데이터
        auto pDev = pDevRepo_->get_by_address_ID(address, device_id);
        if(pDev==nullptr) return;
        if(pDev->type() == device_type::SENSOR_BINARY) {
            uint8_t data = buffer[1];
            pDev->set_sensor_data_received();
            pDev->set_sensor_bin_data(data);
            Logger::i("Sensor<", pDev->info_str(), "> sent value is ", static_cast<unsigned>(data));

            // Send to Python
            ipc_->send(message_type::SENSOR_DATA, pDev, buffer, received_length);

        }else if(pDev->type() == device_type::SENSOR_NUMBER) {
            uint32_t data = buffer[1];
            pDev->set_sensor_data_received();
            pDev->set_sensor_num_data(data);
            Logger::i("Sensor<", pDev->info_str(), "> sent value is ", static_cast<unsigned>(data));

            // Send to Python
            ipc_->send(message_type::SENSOR_DATA, pDev, buffer, received_length);
        }
    }else if(mtype == message_type::PONG) { // 생존 응답
        auto pDev = pDevRepo_->get_by_address_ID(address, device_id);
        if(pDev==nullptr) return;
        ipc_->send(message_type::PONG, pDev, NULL, 0);
    }

    //session->send(message_type::PONG, pDevRepo_->get_test_device(), NULL, 0); // test

    return;
}

void Manager::signal_handler(int signum) {

    if (signum == 2){ // SIGINT
        std::cout << "SIGINT received\n";
        Manager::get_instance().destroy();
        Logger::i("IOT server is stopped.\n");
        exit(signum);
    }
}

void Manager::destroy(){
    Logger::i("Trying to destroy service...");
    close(serv_sock);

    Logger::i("Destroyed.");
}


/*
    PRIVATE FUNCTIONS
*/