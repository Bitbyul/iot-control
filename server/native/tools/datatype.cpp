#include "datatype.h"

using namespace std;

namespace DataType {
    vector<uint16_t> str_to_address(string input) {
        vector<uint16_t> answer;
        stringstream ss(input);
        string temp;

        char delimiter = '-';
    
        while (getline(ss, temp, delimiter)) {
            answer.push_back(stoi(temp));
        }
    
        return answer;
    }

    string address_to_str(vector<uint16_t> address) {
        stringstream ss{};
        for(auto iter=address.begin(); iter!=address.end(); ++iter) {
            ss << *iter;
            if(iter+1 != address.end())
                ss << "-";
        }

        return ss.str();
    }
};