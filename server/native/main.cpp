#include <iostream>
#include <csignal>

#include "manager.h"
#include "tools/logger.h"

using namespace std;

int main(int argc, char** argv){
    Logger::i("Started IoT Communicate server.");
    
    auto &manager = Manager::get_instance();
    
    signal(SIGINT, Manager::signal_handler); // SIGINT 핸들러 등록

    if( manager.start_listen() == false )
        Logger::e("Server has encountered a problem. shutdown...");

    return 0;
}