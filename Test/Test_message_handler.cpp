#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>
#include <unordered_map>

class dispatcher{

    std::unordered_map<int16_t, std::function<bool(uint8_t *, size_t)>> handler_list;
    uint32_t sequenceid;  // Counter which is incremented for each message
    int16_t  msgtype;  // Msg type 1 : 0x0001 (Meas data), Msg type 2 : 0x0002, Msg type 3: 0x007f
    int16_t  comid; // Data format in the user payload  
    int16_t  datalength;  // Length of data payload
    uint8_t  status;  // Status is '0' for msg_type 1 and 2 and only used by message type 0x7f
    uint8_t  padding; // Not used
    uint8_t  Data_payload[]; // Data payload with data length bytes

    public:
        dispatcher(){};
        ~dispatcher(){};
        void add_message_handler (int16_t msgtype, std::function<bool(uint8_t *, size_t length)> handlerfunc);
        bool dispatch(uint8_t *data, size_t total_packet_length);
};


void dispatcher::add_message_handler (int16_t msgtype, std::function<bool(uint8_t *, size_t length)> handlerfunc){

    handler_list.insert(make_pair(msgtype, handlerfunc));
}

bool dispatcher::dispatch(uint8_t *data, size_t total_packet_length){

    int16_t msgtype = 0x0002; //Yet to implement
    auto it = handler_list.find(msgtype);

    //if(it->first == NULL) return false;

    auto result = it->second(data, total_packet_length);

    return result;
}

bool print_num(uint8_t *data, size_t length)
{
    std::cout << *data << '\n';
    std::cout << length << '\n';

    bool result;
    return result;
}
 
int main (int argc, char *argv[]){

    dispatcher test;
    
    uint8_t *data;
    data[0] = 1;
    size_t length = 4;



    std::function<bool(uint8_t *, size_t)> f_display = print_num;

    test.add_message_handler(0x0001, f_display(data, length)) const;

    std::cout << "Hello World" << std::endl;
    return 0;
}
