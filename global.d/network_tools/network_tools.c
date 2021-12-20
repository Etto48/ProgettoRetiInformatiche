#include "network_tools.h"

void NetworkSerializeMessage(MessageType type, const char *payload, uint8_t **dst_stream)
{
    MessageHeader header;
    header.type = type;
    switch (header.type)
    {
    case MESSAGE_RESPONSE:
        header.payload_size = 1;
        break;
    case MESSAGE_SIGNUP:
        header.payload_size = USERNAME_MAX_LENGTH + PASSWORD_MAX_LENGTH;
        break;
    case MESSAGE_LOGIN:
        header.payload_size = 2 + USERNAME_MAX_LENGTH + PASSWORD_MAX_LENGTH;
        break;
    case MESSAGE_LOGOUT:
        header.payload_size = 0;
        break;
    case MESSAGE_HANGING:
        header.payload_size = strlen(payload) == 0 ? 0 : USERNAME_MAX_LENGTH;
        break;
    case MESSAGE_USERINFO_REQ:
        header.payload_size = USERNAME_MAX_LENGTH;
        break;
    case MESSAGE_USERINFO_RES:
        header.payload_size = 4 + 2;
        break;
    case MESSAGE_SYNCREAD:
        header.payload_size = USERNAME_MAX_LENGTH + 8;
        break;
    case MESSAGE_DATA:
        header.payload_size = USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 
            sizeof(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8);
        break;
    default:
        header.payload_size = 0;
        break;
    }

    size_t total_size = sizeof(uint8_t) + sizeof(uint32_t) + header.payload_size;

    *dst_stream = (uint8_t *)malloc(total_size);
    *dst_stream[0] = header.type;
    *(uint32_t *)&(*dst_stream[1]) = htonl(header.payload_size);
    memcpy(*dst_stream + 5, payload, header.payload_size);
}

void NetworkDeserializeMessage(const uint8_t *src_stream, MessageType **type, char **payload)
{
    MessageHeader header;
    header.type = (MessageType)src_stream[0];
    header.payload_size = ntohl(*(uint32_t *)&src_stream[1]);
    *payload = (char *)malloc(header.payload_size);
    memcpy(*payload, &(src_stream[5]), header.payload_size);
    *type = (MessageType *)malloc(sizeof(MessageType));
    **type = header.type;
}