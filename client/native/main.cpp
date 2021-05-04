#include <iostream>

#include "controller.h"
#include "session.h"
#include "tools/logger.h"

#define PREFERRED_CLIENT_ID 3

int main(void) {

    Controller controller{};

    Session session{PREFERRED_CLIENT_ID, controller};

    uint8_t server_connected_result;
    while(true) {
        while(true) {
            server_connected_result = session.connect_server("127.0.0.1", 7143);
            if(server_connected_result == 0) {
                // 연결 성공
                controller.register_device(&session);
                controller.enable_sensor_timer(&session);
                break;
            }
            else if(server_connected_result!=reason_code::REASON_SERVER_OFFLINE) {
                // 서버는 켜졌지만 기타 이유로 인해 연결 실패
                std::cout << static_cast<unsigned>(server_connected_result);
                return -1;
            }
            Logger::i("Server is offline. wait until server become online...");
            sleep(10);
        }
        while(controller.session_connected){
            sleep(3);
        }
        Logger::i("Retrying to connect to server...");
    }

    return 0;
}