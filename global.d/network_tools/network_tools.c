#include "network_tools.h"

#define NETWORK_SEND_MESSAGE_EPILOGUE(msg_type, file_size_ptr, before_exit)                \
    uint8_t *serialized = NULL;                                                            \
    size_t len = NetworkSerializeMessage((msg_type), payload, &serialized, file_size_ptr); \
    ssize_t ret = send(sockfd, serialized, len, 0);                                        \
    free(serialized);                                                                      \
    before_exit if (ret < 0 || (size_t)ret < len)                                          \
    {                                                                                      \
        dbgerror("Error sending a message");                                               \
        return false;                                                                      \
    }                                                                                      \
    else return true;

size_t NetworkSerializeMessage(MessageType type, const uint8_t *payload, uint8_t **dst_stream, const size_t *file_size)
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
        header.payload_size = MESSAGE_DATA_TEXT_MIN_SIZE;
        header.payload_size +=
            payload[USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + sizeof(uint64_t)] == 'F'
                ? sizeof(uint32_t) + (file_size ? *file_size + ntohl(*(uint32_t *)(payload + MESSAGE_DATA_TEXT_MIN_SIZE)) : strlen((char *)(payload + MESSAGE_DATA_FILE_MIN_SIZE)))
                : strlen((char *)(payload + MESSAGE_DATA_TEXT_MIN_SIZE));
        break;
    default:
        header.payload_size = 0;
        break;
    }

    size_t total_size = NETWORK_SERIALIZED_HEADER_SIZE + header.payload_size;

    *dst_stream = (uint8_t *)malloc(total_size);
    *dst_stream[0] = header.type;
    *(uint32_t *)(*dst_stream + 1) = htonl(header.payload_size);
    memcpy(*dst_stream + NETWORK_SERIALIZED_HEADER_SIZE, payload, header.payload_size);
    return total_size;
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
    ret.payload_size = ntohl(*(uint32_t *)(src_stream + 1));
    return ret;
}

void NetworkDeserializeMessageResponse(size_t payload_size, const uint8_t *payload, bool *ok)
{
    if (payload_size == MESSAGE_RESPONSE_SIZE && payload && ok)
        *ok = (bool)*payload;
    else
        DebugLog("RESPONSE deserialization error");
}
void NetworkDeserializeMessageSignup(size_t payload_size, const uint8_t *payload, UserName *username, Password *password)
{
    if (payload_size == MESSAGE_SIGNUP_SIZE && payload && username && password)
    {
        memcpy(username->str, payload, USERNAME_MAX_LENGTH);
        memcpy(password->data, payload + USERNAME_MAX_LENGTH, PASSWORD_SIZE);
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("SIGNUP deserialization error");
}
void NetworkDeserializeMessageLogin(size_t payload_size, const uint8_t *payload, uint16_t *port, UserName *username, Password *password)
{
    if (payload_size == MESSAGE_LOGIN_SIZE && payload && port && username && password)
    {
        *port = ntohs(*(uint16_t *)payload);
        memcpy(username->str, payload + sizeof(uint16_t), USERNAME_MAX_LENGTH);
        memcpy(password->data, payload + sizeof(uint16_t) + USERNAME_MAX_LENGTH, PASSWORD_SIZE);
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("LOGIN deserialization error");
}
void NetworkDeserializeMessageHanging(size_t payload_size, const uint8_t *payload, UserName *username)
{
    if (payload_size == MESSAGE_HANGING_MIN_SIZE)
    {
        memset(username->str, 0, USERNAME_MAX_LENGTH + 1);
    }
    else if (payload_size == MESSAGE_HANGING_MAX_SIZE && payload && username)
    {
        memcpy(username->str, payload, USERNAME_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("HANGING deserialization error");
}
void NetworkDeserializeMessageUserinfoReq(size_t payload_size, const uint8_t *payload, UserName *username)
{
    if (payload_size == MESSAGE_USERINFO_REQ_SIZE && payload && username)
    {
        memcpy(username->str, payload, USERNAME_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
    }
    else
        DebugLog("USERINFO_REQ deserialization error");
}
void NetworkDeserializeMessageUserinfoRes(size_t payload_size, const uint8_t *payload, uint32_t *ip, uint16_t *port)
{
    if (payload_size == MESSAGE_USERINFO_RES_SIZE && payload && ip && port)
    {
        *ip = ntohl(*(uint32_t *)payload);
        *port = ntohs(*(uint16_t *)(payload + sizeof(uint32_t)));
    }
    else
        DebugLog("USERINFO_RES deserialization error");
}
void NetworkDeserializeMessageSyncread(size_t payload_size, const uint8_t *payload, UserName *username, time_t *timestamp)
{
    if (payload_size == MESSAGE_SYNCREAD_SIZE && payload && username && timestamp)
    {
        memcpy(username->str, payload, USERNAME_MAX_LENGTH);
        username->str[USERNAME_MAX_LENGTH] = '\0';
        *timestamp = ntohq(*(uint64_t *)(payload + USERNAME_MAX_LENGTH));
    }
    else
        DebugLog("SYNCREAD deserialization error");
}
bool NetworkMessageDataContainsFile(size_t payload_size, const uint8_t *payload)
{
    if (payload_size >= MESSAGE_DATA_TEXT_MIN_SIZE && payload)
    {
        return (char)payload[USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + sizeof(uint64_t)] == 'F';
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return false;
    }
}
size_t NetworkMessageDataTextLength(size_t payload_size, const uint8_t *payload)
{
    if (payload_size >= MESSAGE_DATA_TEXT_MIN_SIZE && payload)
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
    if (payload_size >= MESSAGE_DATA_FILE_MIN_SIZE && payload)
    {
        return ntohl(*(uint32_t *)(payload + MESSAGE_DATA_TEXT_MIN_SIZE));
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return 0;
    }
}
size_t NetworkMessageDataFileSize(size_t payload_size, const uint8_t *payload)
{
    if (payload_size >= MESSAGE_DATA_FILE_MIN_SIZE && payload)
    {
        return payload_size - MESSAGE_DATA_FILE_MIN_SIZE - NetworkMessageDataFilenameLength(payload_size, payload);
    }
    else
    {
        DebugLog("DATA predeserialization analysis error");
        return 0;
    }
}
void NetworkDeserializeMessageDataText(size_t payload_size, const uint8_t *payload, UserName *src_username, UserName *dst_username, time_t *timestamp, char *text)
{
    if (payload_size >= MESSAGE_DATA_TEXT_MIN_SIZE && payload && src_username && dst_username && timestamp && text)
    {
        memcpy(src_username->str, payload, USERNAME_MAX_LENGTH);
        memcpy(dst_username->str, payload + USERNAME_MAX_LENGTH, USERNAME_MAX_LENGTH);
        uint64_t net_timestamp;
        memcpy(&net_timestamp, payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH, sizeof(uint64_t));
        *timestamp = ntohq(net_timestamp);

        size_t text_size = NetworkMessageDataTextLength(payload_size, payload);
        memcpy(text, payload + MESSAGE_DATA_TEXT_MIN_SIZE, text_size);
        text[text_size] = '\0';
    }
    else
        DebugLog("DATA deserialization error");
}
void NetworkDeserializeMessageDataFile(size_t payload_size, const uint8_t *payload, UserName *src_username, UserName *dst_username, time_t *timestamp, char *filename, uint8_t *data)
{
    if (payload_size >= MESSAGE_DATA_FILE_MIN_SIZE && payload && src_username && dst_username && timestamp && filename && data)
    {
        memcpy(src_username->str, payload, USERNAME_MAX_LENGTH);
        memcpy(dst_username->str, payload + USERNAME_MAX_LENGTH, USERNAME_MAX_LENGTH);
        uint64_t net_timestamp;
        memcpy(&net_timestamp, payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH, sizeof(uint64_t));
        *timestamp = ntohq(net_timestamp);

        size_t filename_size = NetworkMessageDataFilenameLength(payload_size, payload);
        size_t file_size = NetworkMessageDataFileSize(payload_size, payload);
        memcpy(filename, payload + MESSAGE_DATA_FILE_MIN_SIZE, filename_size);
        filename[filename_size] = '\0';
        memcpy(data, payload + MESSAGE_DATA_FILE_MIN_SIZE + filename_size, file_size);
    }
    else
        DebugLog("DATA deserialization error");
}

bool NetworkSendMessageResponse(int sockfd, bool ok)
{
    uint8_t payload[1] = {ok};

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_RESPONSE, NULL, )
}
bool NetworkSendMessageSignup(int sockfd, UserName username, Password password)
{
    uint8_t payload[USERNAME_MAX_LENGTH + PASSWORD_SIZE];
    memcpy(payload, username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + USERNAME_MAX_LENGTH, password.data, PASSWORD_SIZE);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_SIGNUP, NULL, )
}
bool NetworkSendMessageLogin(int sockfd, uint16_t port, UserName username, Password password)
{
    uint8_t payload[2 + USERNAME_MAX_LENGTH + PASSWORD_SIZE];
    *(uint16_t *)payload = htons(port);
    memcpy(payload + 2, username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + 2 + USERNAME_MAX_LENGTH, password.data, PASSWORD_SIZE);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_LOGIN, NULL, )
}
bool NetworkSendMessageLogout(int sockfd)
{
    uint8_t payload[0];

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_LOGOUT, NULL, )
}
bool NetworkSendMessageHanging(int sockfd, UserName *username)
{
    uint8_t payload[USERNAME_MAX_LENGTH];
    if (username)
        memcpy(payload, username->str, USERNAME_MAX_LENGTH);
    else
        memset(payload, 0, USERNAME_MAX_LENGTH);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_HANGING, NULL, )
}
bool NetworkSendMessageUserinfoReq(int sockfd, UserName username)
{
    uint8_t payload[USERNAME_MAX_LENGTH];
    memcpy(payload, username.str, USERNAME_MAX_LENGTH);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_USERINFO_REQ, NULL, )
}
bool NetworkSendMessageUserinfoRes(int sockfd, uint32_t ip, uint16_t port)
{
    uint8_t payload[MESSAGE_USERINFO_RES_SIZE];
    *(uint32_t *)(payload) = htonl(ip);
    *(uint16_t *)(payload + sizeof(uint32_t)) = htons(port);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_USERINFO_RES, NULL, )
}
bool NetworkSendMessageSyncread(int sockfd, UserName username, time_t timestamp)
{
    uint8_t payload[USERNAME_MAX_LENGTH + 8];
    memcpy(payload, username.str, USERNAME_MAX_LENGTH);
    *(uint64_t *)(payload + USERNAME_MAX_LENGTH) = htonq((uint64_t)timestamp);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_SYNCREAD, NULL, )
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

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_DATA, NULL,
                                  free(payload);)
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
    close(fd);
    size_t basename_offset = ToolsBasename(filename);

    bool ret = NetworkSendMessageDataFileBuffer(sockfd, src_username, dst_username, timestamp, filename + basename_offset, st.st_size, file_buffer);
    free(file_buffer);
    return ret;
}

bool NetworkSendMessageDataFileBuffer(int sockfd, UserName src_username, UserName dst_username, time_t timestamp, const char *filename, size_t file_size, const uint8_t *data)
{
    uint32_t basename_length = strlen(filename);
    uint8_t *payload = (uint8_t *)malloc(USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4 + basename_length + file_size);
    memcpy(payload, src_username.str, USERNAME_MAX_LENGTH);
    memcpy(payload + USERNAME_MAX_LENGTH, dst_username.str, USERNAME_MAX_LENGTH);
    *(uint64_t *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH) = htonq(timestamp);
    *(char *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8) = 'F';
    *(uint32_t *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1) = htonl(basename_length);
    memcpy(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4, filename, basename_length);
    memcpy(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4 + basename_length, data, file_size);

    NETWORK_SEND_MESSAGE_EPILOGUE(MESSAGE_DATA, &file_size,
                                  free(payload);)
}