#include "network_tools.h"

#define NETWORK_SEND_MESSAGE_EPILOGUE(before_exit)                                \
    uint8_t *serialized = NULL;                                                   \
    size_t len = NetworkSerializeMessage(MESSAGE_RESPONSE, payload, &serialized); \
    ssize_t ret = send(sockfd, serialized, len, 0);                               \
    free(serialized);                                                             \
    before_exit if (ret < 0 || (size_t)ret < len)                                 \
    {                                                                             \
        dbgerror("Error sending a message");                                      \
        return false;                                                             \
    }                                                                             \
    else return true;

size_t NetworkSerializeMessage(MessageType type, const uint8_t *payload, uint8_t **dst_stream)
{
    MessageHeader header;
    header.type = type;
    switch (header.type)
    {
    case MESSAGE_RESPONSE:
        header.payload_size = MESSAGE_RESPONSE_SIZE;
        break;
    case MESSAGE_SIGNUP:
        header.payload_size = MESSAGE_SIGNUP_SIZE;
        break;
    case MESSAGE_LOGIN:
        header.payload_size = MESSAGE_LOGIN_SIZE;
        break;
    case MESSAGE_LOGOUT:
        header.payload_size = MESSAGE_LOGOUT_SIZE;
        break;
    case MESSAGE_HANGING:
        header.payload_size = (payload && payload[0] != '\0') ? MESSAGE_HANGING_MAX_SIZE : MESSAGE_HANGING_MIN_SIZE;
        break;
    case MESSAGE_USERINFO_REQ:
        header.payload_size = MESSAGE_USERINFO_REQ_SIZE;
        break;
    case MESSAGE_USERINFO_RES:
        header.payload_size = MESSAGE_USERINFO_RES_SIZE;
        break;
    case MESSAGE_SYNCREAD:
        header.payload_size = MESSAGE_SYNCREAD_SIZE;
        break;
    case MESSAGE_DATA:
        header.payload_size =
            MESSAGE_DATA_TEXT_MIN_SIZE +
            payload[USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + sizeof(uint64_t)] == 'F'
                ? sizeof(uint32_t) + strlen((char *)(payload + MESSAGE_DATA_FILE_MIN_SIZE))
                : strlen((char *)(payload + MESSAGE_DATA_TEXT_MIN_SIZE));
        break;
    default:
        header.payload_size = 0;
        break;
    }

    size_t total_size = sizeof(uint8_t) + sizeof(uint32_t) + header.payload_size;

    *dst_stream = (uint8_t *)malloc(total_size);
    *dst_stream[0] = header.type;
    *(uint32_t *)(*dst_stream + 1) = htonl(header.payload_size);
    memcpy(*dst_stream + NETWORK_SERIALIZED_HEADER_SIZE, payload, header.payload_size);
    return NETWORK_SERIALIZED_HEADER_SIZE + header.payload_size;
}
size_t NetworkDeserializeMessage(const uint8_t *src_stream, MessageType **type, uint8_t **payload)
{
    MessageHeader header = NetworkDeserializeHeader(src_stream);
    *payload = (uint8_t *)malloc(header.payload_size);
    memcpy(*payload, src_stream + NETWORK_SERIALIZED_HEADER_SIZE, header.payload_size);
    *type = (MessageType *)malloc(sizeof(MessageType));
    **type = header.type;
    return header.payload_size;
}
MessageHeader NetworkDeserializeHeader(const uint8_t *src_stream)
{
    MessageHeader ret;
    ret.type = (MessageType)src_stream[0];
    ret.payload_size = ntohl(*(uint32_t *)(src_stream+1));
    return ret;
}



void NetworkDeserializeMessageResponse(size_t payload_size, const uint8_t *payload, bool *ok)
{
    if(payload_size==MESSAGE_RESPONSE_SIZE && payload && ok)
        *ok = (bool)*payload;
    else
        DebugLog("RESPONSE deserialization error");
}
void NetworkDeserializeMessageSignup(size_t payload_size, const uint8_t *payload, UserName *username, Password *password)
{
    if(payload_size==MESSAGE_SIGNUP_SIZE && payload && username && password)
    {
        memcpy(username->str,payload,USERNAME_MAX_LENGTH);
        memcpy(password->str,payload+USERNAME_MAX_LENGTH,PASSWORD_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
        password->str[PASSWORD_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("SIGNUP deserialization error");
}
void NetworkDeserializeMessageLogin(size_t payload_size, const uint8_t *payload, uint16_t* port, UserName *username, Password *password)
{
    if(payload_size==MESSAGE_LOGIN_SIZE && payload && port && username && password)
    {
        *port = ntohs(*(uint16_t *)payload);
        memcpy(username->str,payload+sizeof(uint16_t),USERNAME_MAX_LENGTH);
        memcpy(password->str,payload+sizeof(uint16_t)+USERNAME_MAX_LENGTH,PASSWORD_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
        password->str[PASSWORD_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("LOGIN deserialization error");
}
void NetworkDeserializeMessageHanging(size_t payload_size, const uint8_t *payload, UserName *username)
{
    if(payload_size == MESSAGE_HANGING_MIN_SIZE)
    {
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else if(payload_size == MESSAGE_HANGING_MAX_SIZE && payload && username)
    {
        memcpy(username->str,payload,USERNAME_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("HANGING deserialization error");
}
void NetworkDeserializeMessageUserinfoReq(size_t payload_size, const uint8_t *payload, UserName *username)
{
    if(payload_size==MESSAGE_USERINFO_REQ_SIZE && payload && username)
    {
        memcpy(username->str,payload,USERNAME_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("USERINFO_REQ deserialization error");
}
void NetworkDeserializeMessageUserinfoRes(size_t payload_size, const uint8_t *payload, uint32_t *ip, uint16_t *port)
{
    if(payload_size==MESSAGE_USERINFO_RES_SIZE && payload && ip && port)
    {
        *ip = ntohl(*(uint32_t *)payload);
        *port = ntohs(*(uint16_t *)(payload+sizeof(uint32_t)));
    }
    else
        DebugLog("USERINFO_RES deserialization error");
}
void NetworkDeserializeMessageSyncread(size_t payload_size, const uint8_t *payload, UserName *username, time_t *timestamp)
{
    if(payload_size==MESSAGE_SYNCREAD_SIZE && payload && username && timestamp)
    {
        memcpy(username->str,payload,USERNAME_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
        *timestamp = ntohq(*(uint64_t *)(payload+USERNAME_MAX_LENGTH));
    }
    else
        DebugLog("SYNCREAD deserialization error");
}
bool NetworkMessageDataContainsFile(size_t payload_size, const uint8_t *payload)
{
    if(payload_size>=MESSAGE_DATA_TEXT_MIN_SIZE && payload)
    {
        return (char)payload[USERNAME_MAX_LENGTH+USERNAME_MAX_LENGTH+sizeof(uint64_t)]=='F';
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return false;
    }

}
size_t NetworkMessageDataTextLength(size_t payload_size, const uint8_t *payload)
{
    if(payload_size>=MESSAGE_DATA_TEXT_MIN_SIZE && payload)
    {
        return payload_size - MESSAGE_DATA_TEXT_MIN_SIZE;
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return 0;
    }
}
size_t NetworkMessageDataFilenameLength(size_t payload_size, const uint8_t *payload)
{
    if(payload_size>=MESSAGE_DATA_FILE_MIN_SIZE && payload)
    {
        return ntohl(*(uint32_t *)(payload+MESSAGE_DATA_TEXT_MIN_SIZE));
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return 0;
    }
}
size_t NetworkMessageDataFileSize(size_t payload_size, const uint8_t *payload)
{
    if(payload_size>=MESSAGE_DATA_FILE_MIN_SIZE && payload)
    {
        return B64DecSize(payload_size - MESSAGE_DATA_FILE_MIN_SIZE - NetworkMessageDataFilenameLength(payload_size,payload));
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return 0;
    }
}
void NetworkDeserializeMessageDataText(size_t payload_size, const uint8_t *payload, UserName *src_username, UserName *dst_username, time_t *timestamp, char *text)
{  
    if(payload_size>=MESSAGE_DATA_TEXT_MIN_SIZE && payload && src_username && dst_username && timestamp && text)
    {
        size_t text_size = NetworkMessageDataTextLength(payload_size,payload);
        memcpy(text,payload+MESSAGE_DATA_TEXT_MIN_SIZE,text_size);
        text[text_size]='\0';
    }
    else
        DebugLog("DATA deserialization error");
}
void NetworkDeserializeMessageDataFile(size_t payload_size, const uint8_t *payload, UserName *src_username, UserName *dst_username, time_t *timestamp, char *filename, uint8_t *data)
{
    if(payload_size>=MESSAGE_DATA_FILE_MIN_SIZE && payload && src_username && dst_username && timestamp && filename && data)
    {
        size_t filename_size = NetworkMessageDataFilenameLength(payload_size,payload);
        size_t file_size = NetworkMessageDataFileSize(payload_size,payload);
        memcpy(filename,payload+MESSAGE_DATA_FILE_MIN_SIZE,filename_size);
        filename[filename_size]='\0';
        B64Decode(payload+MESSAGE_DATA_FILE_MIN_SIZE+filename_size,B64EncSize(file_size),data);
    }
    else
        DebugLog("DATA deserialization error");
}



bool NetworkSendMessageResponse(int sockfd, bool ok)
{
    uint8_t payload[1] = {ok};

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageSignup(int sockfd, UserName username, Password password)
{
    uint8_t payload[USERNAME_MAX_LENGTH + PASSWORD_MAX_LENGTH];
    memcpy(payload, username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + USERNAME_MAX_LENGTH, password.str, PASSWORD_MAX_LENGTH);

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageLogin(int sockfd, uint16_t port, UserName username, Password password)
{
    uint8_t payload[2 + USERNAME_MAX_LENGTH + PASSWORD_MAX_LENGTH];
    *(uint16_t *)payload = htons(port);
    memcpy(payload + 2, username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + 2 + USERNAME_MAX_LENGTH, password.str, PASSWORD_MAX_LENGTH);

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageLogout(int sockfd)
{
    uint8_t payload[0];

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageHanging(int sockfd, UserName *username)
{
    uint8_t payload[USERNAME_MAX_LENGTH];
    if (username)
        memcpy(payload, username->str, USERNAME_MAX_LENGTH);
    else
        memset(payload, 0, USERNAME_MAX_LENGTH);

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageUserinfoReq(int sockfd, UserName username)
{
    uint8_t payload[USERNAME_MAX_LENGTH];
    memcpy(payload, username.str, USERNAME_MAX_LENGTH);

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageUserinfoRes(int sockfd, uint32_t ip, uint16_t port)
{
    uint8_t payload[4 + 2];
    *(uint32_t *)(payload) = htonl(ip);
    *(uint16_t *)(payload + 4) = htons(port);

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageSyncread(int sockfd, UserName username, time_t timestamp)
{
    uint8_t payload[USERNAME_MAX_LENGTH + 8];
    memcpy(payload, username.str, USERNAME_MAX_LENGTH);
    *(uint64_t *)(payload + USERNAME_MAX_LENGTH) = htonq((uint64_t)timestamp);

    NETWORK_SEND_MESSAGE_EPILOGUE()
}
bool NetworkSendMessageDataText(int sockfd, UserName src_username, UserName dst_username, time_t timestamp, char *text)
{
    size_t text_len = strlen(text);
    uint8_t *payload = (uint8_t *)malloc(USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + text_len + 1);
    memcpy(payload, src_username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + USERNAME_MAX_LENGTH, dst_username.str, USERNAME_MAX_LENGTH);
    *(uint64_t *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH) = htonq((uint64_t)timestamp);
    *(char *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8) = 'T';
    strcpy((char *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1), text);

    NETWORK_SEND_MESSAGE_EPILOGUE
    (
        free(payload);
    )
}
bool NetworkSendMessageDataFile(int sockfd, UserName src_username, UserName dst_username, time_t timestamp, const char *filename)
{
    struct stat st;
    // get file size (if exists)
    if (stat(filename, &st) < 0)
    {
        dbgerror("Error opening file");
        return false;
    }
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        dbgerror("Error opening file");
        return false;
    }
    uint8_t *file_buffer = (uint8_t *)malloc(st.st_size);
    read(fd, file_buffer, st.st_size);
    size_t file_base64_size = B64EncSize(st.st_size) + 1;
    uint8_t *file_base64_buffer = (uint8_t *)malloc(file_base64_size);
    B64Encode(file_buffer, st.st_size, file_base64_buffer);
    free(file_buffer);

    size_t basename_offset = ToolsBasename(filename);
    uint32_t basename_length = strlen(filename + basename_offset);
    uint8_t *payload = (uint8_t *)malloc(USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4 + basename_length + file_base64_size);
    memcpy(payload, src_username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + USERNAME_MAX_LENGTH, dst_username.str, USERNAME_MAX_LENGTH);
    *(uint64_t *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH) = htonq(timestamp);
    *(char *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8) = 'F';
    *(uint32_t *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1) = htonl(basename_length);
    memcpy(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4, filename + basename_offset, basename_length);
    memcpy(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4 + basename_length, file_base64_buffer, file_base64_size);
    free(file_base64_buffer);

    NETWORK_SEND_MESSAGE_EPILOGUE
    (
        free(payload);
    )
}

