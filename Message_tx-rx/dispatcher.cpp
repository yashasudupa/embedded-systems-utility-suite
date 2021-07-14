#include "dispatcher.h"

void dispatcher::add_message_handler (int16_t msgtype, std::function<bool(uint8_t *, size_t length)> handlerfunc)
{
    // Compare with the allowed message type

    {
       auto it = std::find (allowed_msg_type.begin(), allowed_msg_type.end(), msgtype);
        if (it == allowed_msg_type.end()) return;
    }

    // Repla
    {
        auto it = handler_list.find(msgtype);
        if(it != handler_list.end()) 
            it->second = handlerfunc;
    }

    // 
    
    // Replace it to list
    handler_list.insert(make_pair(msgtype, handlerfunc));

}

bool dispatcher::dispatch(uint8_t *data, size_t total_packet_length)
{
    //struct data *data_struct = (struct data *) data;

    struct data data_struct;
    int16_t message_type = (data[5] ) | (data[4] << 8);

    auto it = handler_list.find(data_struct.msgtype);

    if(it == handler_list.end()) return false;
    auto result = it->second(data, total_packet_length);

    return result;

}

