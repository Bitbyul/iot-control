/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    datatype: 로그 기능 구현 클래스
*/
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>

class Logger {
    static std::mutex log_mutex_;
    
public:

    template <typename T>
    static void concat_all(std::ostringstream& ss, T arg) {
        ss << arg << "\n";
    }

    template <typename T, typename... Types>
    static void concat_all(std::ostringstream& ss, T arg, Types... args) {
        ss << arg;
        concat_all(ss, args...);
    }

    template <typename... Types>
    static void i(Types... args); // 정보 로그

    template <typename... Types>
    static void e(Types... args); // 에러 로그

private:

    Logger(){

    };

    static const char* now_time(){ // 현재 시간 구하기
        static char str_date[50];
        struct std::tm* pNowTimeInfo;

        auto now = time(NULL);
        pNowTimeInfo = std::localtime(&now);

        sprintf(str_date, "%d/%02d/%02d %02d:%02d:%02d", pNowTimeInfo->tm_year+1900, pNowTimeInfo->tm_mon+1, pNowTimeInfo->tm_mday, 
                                        pNowTimeInfo->tm_hour, pNowTimeInfo->tm_min, pNowTimeInfo->tm_sec);

        return str_date;
    }
};

template <typename... Types>
void Logger::i(Types... args){
    std::ostringstream ss;

    concat_all(ss, args...);
    
    std::unique_lock<std::mutex> llk(log_mutex_);

    std::cout << "[" << now_time() << "][I] " << ss.str();
    std::ofstream out("log.txt", std::ios::app);
    if (out.is_open()) {
        out << "[" << now_time() << "][I] " << ss.str();
    }

    llk.unlock();
}

template <typename... Types>
void Logger::e(Types... args){ // 에러 로그
    std::ostringstream ss;

    concat_all(ss, args...);

    std::unique_lock<std::mutex> llk(log_mutex_);

    std::cout << "[" << now_time() << "][E] " << ss.str();
    
    std::ofstream out("log.txt", std::ios::app);
    if (out.is_open()) {
        out << "[" << now_time() << "][I] " << ss.str();
    }

    llk.unlock();
}

#endif