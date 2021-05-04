from socket import *
import threading
import time
import copy

class message_type():
    PONG = 0x0001 # 생존 응답
    REQ_ACTUATOR_OPERATION = 0x0023 # 액추에이터 동작 실행 요구

    IPC_REQ_ALL_DEVICE_INFO = 0x1000 # 모든 장치 정보 요구
    IPC_RESP_DEVICE_INFO = 0x1010 # 장치 정보 응답
    SENSOR_DATA = 0x0022 # 센서 데이터 (비동기, 정기적)

class device_type():
    NOT_SET = 0 # 지정되지 않음

    SENSOR_BINARY = 10 # ON,OFF 센서
    SENSOR_NUMBER = 11 # 수치형 정보를 반환하는 센서
    SENSOR_TEXT = 12 # 문자열을 반환하는 센서

    ACTUATOR_BINARY = 20 # ON,OFF 액추에이터
    ACTUATOR_NUMBER = 21 # 수치형 정보를 받아 동작을 수행하는 액추에이터
    ACTUATOR_TEXT = 22 # 문자열 정보를 받아 동작을 수행하는 액추에이터

class Device():
    def __init__(self, address, device_id, client_id, alias, type):
        self.address = address
        self.device_id = device_id
        self.client_id = client_id
        self.alias = alias
        self.type = type
        self.sensor_data = None
        self.last_updated_time = 0

    def is_alive(self) -> bool:
        if (time.time() - self.last_updated_time) < 20:
            return "alive"
        else:
            return "dead"

    def __hash__(self): 
        return hash((self.address, self.device_id)) 

    def __str__(self):
        return self.address + "." + str(self.device_id) + "<"+str(self.client_id)+">" + " [" + self.alias + "] " + "type: " + str(self.type)

    def __getitem__(self, key):
        return getattr(self, key)
    def __setitem__(self, key, value):
        return setattr(self, key, value)
    def __eq__(self, other):
        return (self.device_id == other.device_id) and (self.address == other.address)

    def as_dict(self):
        return_dict = copy.deepcopy(self.__dict__)
        return_dict.update({'alive':self.is_alive()})
        return return_dict

class DeviceList():
    def __init__(self):
        self.list = list()
    def clear(self):
        self.list.clear()
    def insert(self, device):
        for device_elem in self.list:
            if device_elem == device:
                return
        self.list.append(device)

    def __str__(self):
        return str(self.as_dict_list())

    def as_dict_list(self):
        lst_temp = list()
        for device in self.list:
            lst_temp.append(device.as_dict())
        return lst_temp

    def get_alias_by_addrid(self, addrid):
        addr = addrid.split('.')[0]
        device_id = int(addrid.split('.')[1])

        for device_elem in self.list:
            print(device_elem.address, device_elem.device_id, ":", addr, device_id)
            if ((device_elem.device_id == device_id) and (device_elem.address == addr)):
                return device_elem.alias
        return "?"

class Scenario():
    def __init__(self, deviceList):
        self.deviceList = deviceList
        self.name = ""
        self.sensor_list = [] # {addrid, operator, value, alias}
        self.actuator_list = [] # {addrid, value, alias}

    def check_scenario(self):
        for each_sensor in self.sensor_list:
            addr = each_sensor['addrid'].split('.')[0]
            device_id = int(each_sensor['addrid'].split('.')[1])
            for device_elem in self.deviceList.list:
                if ((device_elem.device_id == device_id) and (device_elem.address == addr)):
                    if device_elem.sensor_data is None: return False
                    #print(device_elem.sensor_data, each_sensor['operator'], int(each_sensor['value']))
                    if each_sensor['operator'] == '=':
                        if device_elem.sensor_data != int(each_sensor['value']): return False
                    elif each_sensor['operator'] == '<':
                        if device_elem.sensor_data >= int(each_sensor['value']): return False
                    elif each_sensor['operator'] == '>':
                        if device_elem.sensor_data <= int(each_sensor['value']): return False
                    else:
                        return False

        return True

class ScenarioList():
    def __init__(self):
        self.scenario_list = [] # Scenario

    def insert(self, scenario):
        self.scenario_list.append(scenario)
        print("Scenario [",scenario.name,"] inserted.")

    def check_scenario_list(self, addr, device_id):
        slist = []
        for each_scenario in self.scenario_list:
            for each_scenario_sensor in each_scenario.sensor_list:
                if each_scenario_sensor['addrid'] == (addr + "." + str(device_id)):
                    if each_scenario.check_scenario():
                        print("Scenario [",each_scenario.name,"] triggered.")
                        slist.append(each_scenario)

        return slist

class IPC():
    def __init__(self):
        self.deviceList = DeviceList()
        self.scenarioList = ScenarioList()
        self.load_scenario_list()
        self.csock = socket(AF_INET, SOCK_STREAM)
        self.csock.connect(("127.0.0.1", 7143))
        receiver = threading.Thread(target=self.recv)
        receiver.start()

    def load_scenario_list(self):
        scenario = Scenario(self.deviceList)
        scenario.name = "모바일컨텐츠 입장 알림"
        scenario.sensor_list.append({"addrid": "1-18-205.3", "operator": "=", "value": "1", "alias": "모션인식"})
        scenario.actuator_list.append({"addrid": "1-18-205.4", "operator": "=", "value": "1", "alias": "알림음"})
        self.scenarioList.insert(scenario)
        
        scenario = Scenario(self.deviceList)
        scenario.name = "중앙 에어컨 자동 ON"
        scenario.sensor_list.append({"addrid": "1-18.2", "operator": ">", "value": "50", "alias": "중앙습도계"})
        scenario.sensor_list.append({"addrid": "1-18.1", "operator": ">", "value": "30", "alias": "중앙온도계"})
        scenario.actuator_list.append({"addrid": "1-18.5", "operator": "=", "value": "1", "alias": "중앙에어컨"})
        self.scenarioList.insert(scenario)

        scenario = Scenario(self.deviceList)
        scenario.name = "중앙 에어컨 자동 OFF"
        scenario.sensor_list.append({"addrid": "1-18.2", "operator": "<", "value": "30", "alias": "중앙습도계"})
        scenario.sensor_list.append({"addrid": "1-18.1", "operator": "<", "value": "20", "alias": "중앙온도계"})
        scenario.actuator_list.append({"addrid": "1-18.5", "operator": "=", "value": "0", "alias": "중앙에어컨"})
        self.scenarioList.insert(scenario)

    def add_scenario(self, name, 
                        sensor_1_addrid, sensor_1_operation, sensor_1_value,
                        sensor_2_addrid, sensor_2_operation, sensor_2_value,
                        sensor_3_addrid, sensor_3_operation, sensor_3_value,
                        actuator_1_addrid, actuator_1_value,
                        actuator_2_addrid, actuator_2_value,
                        actuator_3_addrid, actuator_3_value
                        ):
                        
        scenario = Scenario(self.deviceList)
        scenario.name = name
        if not sensor_1_addrid == "":
            scenario.sensor_list.append({'addrid':sensor_1_addrid, 'operator':sensor_1_operation, 'value':sensor_1_value, 'alias':self.deviceList.get_alias_by_addrid(sensor_1_addrid)})
        if not sensor_2_addrid == "":
            scenario.sensor_list.append({'addrid':sensor_2_addrid, 'operator':sensor_2_operation, 'value':sensor_2_value, 'alias':self.deviceList.get_alias_by_addrid(sensor_2_addrid)})
        if not sensor_3_addrid == "":
            scenario.sensor_list.append({'addrid':sensor_3_addrid, 'operator':sensor_3_operation, 'value':sensor_3_value, 'alias':self.deviceList.get_alias_by_addrid(sensor_3_addrid)})
        if not actuator_1_addrid == "":
            scenario.actuator_list.append({'addrid':actuator_1_addrid, 'value':actuator_1_value, 'alias':self.deviceList.get_alias_by_addrid(actuator_1_addrid)})
        if not actuator_2_addrid == "":
            scenario.actuator_list.append({'addrid':actuator_2_addrid, 'value':actuator_2_value, 'alias':self.deviceList.get_alias_by_addrid(actuator_2_addrid)})
        if not actuator_3_addrid == "":
            scenario.actuator_list.append({'addrid':actuator_3_addrid, 'value':actuator_3_value, 'alias':self.deviceList.get_alias_by_addrid(actuator_3_addrid)})

        self.scenarioList.insert(scenario)

    def get_scenario_list(self):
        return self.scenarioList

    def recv(self):
        while True:
            mtype = int.from_bytes(self.csock.recv(2), byteorder='little', signed=False) # MSG_TYPE
            timestamp = int.from_bytes(self.csock.recv(4), byteorder='little', signed=False) # TIMESTAMP
            client_id = int.from_bytes(self.csock.recv(2), byteorder='little', signed=False) # CLIENT_ID
            addr_depth = int.from_bytes(self.csock.recv(1), byteorder='little', signed=False) # ADDR_DEPTH
            addr = ""
            if (addr_depth > 0):
                for i in range(0, addr_depth):
                    addr_token = int.from_bytes(self.csock.recv(2), byteorder='little', signed=False) # DEVICE_ADDRESS(TOKEN)
                    addr += str(addr_token)
                    if(i != addr_depth-1):
                        addr += "-"

            device_id = int.from_bytes(self.csock.recv(2), byteorder='little', signed=False) # DEVICE_ID
            payload_length = int.from_bytes(self.csock.recv(2), byteorder='little', signed=False) # PAYLOAD_LENGTH
            
            payload = None
            if (payload_length > 0):
                payload = self.csock.recv(payload_length) # payload

            #print(mtype, timestamp, client_id, addr_depth, addr, device_id, payload_length)

            if mtype==message_type.IPC_RESP_DEVICE_INFO: # 기기 정보 응답
                type = int.from_bytes(payload[:1], byteorder='little', signed=False) # DEVICE_TYPE
                alias = payload[2:].decode('utf-8') # DEVICE_ALIAS
                self.deviceList.insert(Device(addr, device_id, client_id, alias, type))

            elif mtype==message_type.SENSOR_DATA: # 센서 값 응답
                type = int.from_bytes(payload[:1], byteorder='little', signed=False) # DEVICE_TYPE
                
                if type==device_type.SENSOR_BINARY:
                    data = int.from_bytes(payload[1:2], byteorder='little', signed=False) # Binary Value
                elif type==device_type.SENSOR_NUMBER:
                    data = int.from_bytes(payload[1:], byteorder='little', signed=False) # Number Value
                
                #print("data:", data)

                for device_elem in self.deviceList.list:
                    #print(device_elem.address, device_elem.device_id, ":", addr, device_id)
                    if ((device_elem.device_id == device_id) and (device_elem.address == addr)):
                        device_elem.sensor_data = data
                        device_elem.last_updated_time = time.time()

                # 시나리오 체크
                triggered_scenario_list = self.scenarioList.check_scenario_list(addr, device_id)
                for scenario in triggered_scenario_list:
                    for each_actuator in scenario.actuator_list:
                        addr = each_actuator['addrid'].split('.')[0]
                        device_id = int(each_actuator['addrid'].split('.')[1])
                        for device_elem in self.deviceList.list:
                            if ((device_elem.device_id == device_id) and (device_elem.address == addr)):
                                self.set_actuator_operation(device_elem, each_actuator['value'])
            
            elif mtype==message_type.PONG: # 생존 응답
                 for device_elem in self.deviceList.list:
                    if ((device_elem.device_id == device_id) and (device_elem.address == addr)):
                        device_elem.last_updated_time = time.time()

    def get_all_device_info(self):
        timestamp = 0x12341234

        self.csock.send(message_type.IPC_REQ_ALL_DEVICE_INFO.to_bytes(2, byteorder='little')) # MSG_TYPE
        self.csock.send(timestamp.to_bytes(4, byteorder='little')) # TIMESTAMP
        self.csock.send((0).to_bytes(2, byteorder='little')) # CLIENT_ID: 0
        self.csock.send((0).to_bytes(1, byteorder='little')) # ADDR_DEPTH: 0
        self.csock.send((0).to_bytes(2, byteorder='little')) # DEVICE_ID: 0
        self.csock.send((0).to_bytes(2, byteorder='little')) # PAYLOAD_LENGTH: 0

    def find_device_by_addrid(self, addrid):
        address = addrid.split('.')[0]
        id = int(addrid.split('.')[1])
        for device_elem in self.deviceList.list:
            if ((device_elem.device_id == id) and (device_elem.address == address)):
                return device_elem
        return None


    def set_actuator_operation(self, device, val):
        timestamp = 0x12341234

        self.csock.send(message_type.REQ_ACTUATOR_OPERATION.to_bytes(2, byteorder='little')) # MSG_TYPE
        self.csock.send(timestamp.to_bytes(4, byteorder='little')) # TIMESTAMP
        self.csock.send((device.client_id).to_bytes(2, byteorder='little')) # device's CLIENT_ID
        self.csock.send((device.address.count('-')+1).to_bytes(1, byteorder='little')) # ADDR_DEPTH
        addr_tokens = device.address.split('-')
        for token in addr_tokens:
            self.csock.send(int(token).to_bytes(2, byteorder='little')) # address token

        self.csock.send((device.device_id).to_bytes(2, byteorder='little')) # DEVICE_ID        

        if(device.type == device_type.ACTUATOR_BINARY):
            payload_length = 2

            payload_buffer = bytes()
            payload_buffer += device.type.to_bytes(1, byteorder='little') # DEVICE_TYPE
            payload_buffer += (int(val)).to_bytes(1, byteorder='little') # data 1 bytes

            self.csock.send(payload_length.to_bytes(2, byteorder='little')) # PAYLOAD_LENGTH
            self.csock.send(payload_buffer) # payload

        elif(device.type == device_type.ACTUATOR_NUMBER):
            payload_length = 5

            payload_buffer = bytes()
            payload_buffer += device.type.to_bytes(1, byteorder='little') # DEVICE_TYPE
            payload_buffer += (int(val)).to_bytes(4, byteorder='little') # data 4 bytes

            self.csock.send(payload_length.to_bytes(2, byteorder='little')) # PAYLOAD_LENGTH
            self.csock.send(payload_buffer) # payload

        elif(device.type == device_type.ACTUATOR_TEXT):
            encoded_str = val.encode('utf-8')
            payload_length = 1 + len(encoded_str) + 1

            payload_buffer = bytes()
            payload_buffer += device.type.to_bytes(1, byteorder='little') # DEVICE_TYPE
            payload_buffer += encoded_str # str

            self.csock.send(payload_length.to_bytes(2, byteorder='little')) # PAYLOAD_LENGTH
            self.csock.send(payload_buffer) # payload