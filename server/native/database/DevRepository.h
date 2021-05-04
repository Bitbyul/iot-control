/*
    한신대학교 IoT 프로그래밍
    201658059 김재혁
    DevRepository: 장치 목록 관리
*/
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <iostream>

#include "../devices/DeviceEntry.h"
#include "../tools/datatype.h"

class DevRepository {

    class DevTree {

        struct Node {
            uint16_t depth_id; // 깊이 ID
            std::string depth_alias; // 깊이 별칭
            std::vector<std::shared_ptr<DeviceEntry>> devices; // 해당 노드에 연결된 장치 목록

            std::vector<std::unique_ptr<Node>> children; // 해당 노드에 연결된 자식 노드

            Node(uint16_t id, std::string alias) : depth_id(id), depth_alias(alias) {}
        };

        std::unique_ptr<Node> root_; // 루트 노드

        Node* searchNode(Node* node, uint16_t id) { // 현재 노드에서 특정 아이디를 가진 자식 노드 반환 내부구현
            for(auto iter=node->children.begin(); iter!=node->children.end(); ++iter) {
                if((*iter)->depth_id==id) return iter->get();
            }
            return nullptr; // 없으면 null 반환
        }

        Node* insertNode(std::vector<uint16_t> address) { // 장소 노드 추가 내부구현 1
            Node* rawpNode = root_.get();
            uint16_t depth=0;
            std::vector<uint16_t>::iterator iter;
            for(iter=address.begin(); iter!=address.end()-1; ++iter) { // 최종 자식 노드 제외
                Node* child = searchNode(rawpNode, address[depth]);
                if(child==nullptr) {
                    rawpNode->children.emplace_back(new Node(address[depth], ""));
                    child = rawpNode->children.back().get();
                }
                rawpNode = child;
                ++depth;
            }
            // 최종 자식 노드
            auto node = searchNode(rawpNode, address[depth]);
            if(node==nullptr) { // 동일한 ID의 자식 노드가 존재하지 않는다면 새로 생성
                rawpNode->children.emplace_back(new Node(address[depth], ""));
                node = rawpNode->children.back().get();
            }
            return node;
        }

        Node* insertNode(std::vector<uint16_t> address, std::string alias) { // 장소 노드 추가 내부구현 2
            Node* rawpNode = root_.get();
            uint16_t depth=0;
            std::vector<uint16_t>::iterator iter;
            for(iter=address.begin(); iter!=address.end()-1; ++iter) { // 최종 자식 노드 제외
                Node* child = searchNode(rawpNode, address[depth]);
                if(child==nullptr) {
                    rawpNode->children.emplace_back(new Node(address[depth], ""));
                    child = rawpNode->children.back().get();
                }
                rawpNode = child;
                ++depth;
            }
            // 최종 자식 노드
            auto node = searchNode(rawpNode, address[depth]);
            if(node!=nullptr) { // 동일한 ID의 자식 노드가 존재한다면 덮어씌운다.
                node->depth_alias = alias;
            }else { // 존재하지 않는다면 새로 생성
                rawpNode->children.emplace_back(new Node(address[depth], alias));
                node = rawpNode->children.back().get();
            }
            return node;
        }

        void getAllChildren(Node* node, std::vector<std::shared_ptr<DeviceEntry>>& list) { // 해당 노드에 있는 모든 장치 목록 불러오기 내부 구현
            //std::cout << "--- " << "{" << node->depth_id << "}" << node->depth_alias << " ---\n";
            list.insert(list.end(), node->devices.begin(), node->devices.end());
            for(auto iter=node->devices.begin(); iter!=node->devices.end(); ++iter) {
                //std::cout << (*iter)->info_str() << "\n";
            }
            for(auto iter=node->children.begin(); iter!=node->children.end(); ++iter) {
                getAllChildren(iter->get(), list);
            }
            return;
        }

    public:

        DevTree() : root_(std::make_unique<Node>(0, "[ROOT]")) {}

        bool insertNode(std::string address_str, std::string alias) { // 장소 노드 추가
            std::vector<uint16_t> address = DataType::str_to_address(address_str);
            insertNode(address, alias);
        }
        
        bool insert(DeviceEntry* rawpDev) { // 장치 추가
            Node* node = insertNode(rawpDev->address()); // 노드 검사
            // 중복 장치 ID 검사
            for(auto iter=node->devices.begin(); iter!=node->devices.end(); ++iter) {
                if((*iter)->address() == rawpDev->address() && (*iter)->id() == rawpDev->id()) {
                    node->devices.erase(iter);
                    std::cout << "중복 장치 제거됨.\n";
                    break;
                }
            }
            node->devices.emplace_back(rawpDev);

            return true;
        }

        std::vector<std::shared_ptr<DeviceEntry>> getAllDevices() {
            std::vector<std::shared_ptr<DeviceEntry>> list;
            getAllChildren(root_.get(), list);
            //for(auto iter=list.begin(); iter!=list.end(); ++iter) {
            //    std::cout << DataType::address_to_str((*iter)->address()) << "." << (*iter)->id() << "\n";
            //}
            return list;
        }
    };

    // 디바이스 정보 저장 트리 자료구조
    DevTree device_tree; // general tree
public:

    DevRepository();

    void db_test();
    bool insert_to_tree(DeviceEntry*); // 개별 장치 정보 트리에 신규 저장
    bool insert_to_tree(std::shared_ptr<DeviceEntry>);

    std::shared_ptr<DeviceEntry> get_by_address_ID(std::vector<uint16_t> address, uint16_t device_id); // 주소 코드로 DeviceEntry 구함
    std::shared_ptr<DeviceEntry> get_test_device();

    std::vector<std::shared_ptr<DeviceEntry>> get_all_devices(); // 모든 장치 목록 불러오기
};