/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    manager: 프로그램 흐름 총괄, 세션 제어
*/
#include <mutex>
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "constants/message_type.h"

class DevRepository;
class FileEntry;
class Session;
class IPC;

class Manager {

    static std::mutex singleton_mutex_;

    static Manager* instance_; // for singleton pattern
    
    std::shared_ptr<IPC> ipc_; // IPC session

    std::vector<std::pair<uint16_t, Session*>> session_list_; // 세션 목록 <client_id, session>

    int serv_sock;
    struct sockaddr_in serv_addr;

    Manager(); // hide default constructor
    Manager(const Manager &) = delete;
	Manager(Manager &&) = delete;
	Manager &operator=(const Manager &) = delete;
	Manager &operator=(Manager &&) = delete;
    
public:
    std::shared_ptr<DevRepository> pDevRepo_; // Device Repository

    static Manager& get_instance(); // for singleton pattern

    bool start_listen(); // 네트워크 통신 대기 시작
    bool regist_session(Session *session); // 세션 리스트에 세션 정보 등록
    void unregist_session(Session *session); // 세션 리스트에서 삭제
    Session* get_session_by_id(uint16_t client_id); // 클라이언트 ID로 세션 가져오기

    void handle_message(Session *session, message_type mtype, std::vector<uint16_t> address, uint16_t device_id, unsigned char* buffer, uint16_t received_length);

    static void signal_handler(int signum);
    void destroy(); // 매니저 종료

};