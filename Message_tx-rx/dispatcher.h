#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>
#include <unordered_map>
#include<bits/stdc++.h>

class dispatcher{

    private :
         std::unordered_map<int16_t, std::function<bool(uint8_t *, size_t)>> handler_list;
         struct data {
            uint32_t sequenceid;  // Status is '0' for msg_type 1 and 2 and only used by message type 0x7f
            int16_t  msgtype; // Msg type 1 : 0x0001 (Meas data), Msg type 2 : 0x0002, Msg type 3: 0x007f
            int16_t  comid; // Data format in the user payload
            int16_t  datalength;  // Length of data payload
            uint8_t  status;
            uint8_t  padding; // Not used
            uint8_t  *Data_payload; // Data payload with data length bytes
         };
     
        std::vector<int16_t>  allowed_msg_type;

    public:
        dispatcher()
        {
                // replace it to enum 
            allowed_msg_type = {0x0001, 0x0002, 0x007F};
        };
        ~dispatcher(){};
        void add_message_handler (int16_t msgtype, std::function<bool(uint8_t *, size_t length)> handlerfunc);
        bool dispatch(uint8_t *data, size_t total_packet_length);
};