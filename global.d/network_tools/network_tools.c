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
        header.payload_size = (payload && payload[0] == '\0') ? 0 : USERNAME_MAX_LENGTH;
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
        header.payload_size =
            USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 +
            payload[USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8] == 'F'
                ? 5 + strlen((char *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1 + 4))
                : 1 + strlen((char *)(payload + USERNAME_MAX_LENGTH + USERNAME_MAX_LENGTH + 8 + 1));
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
    MessageHeader header;
    header.type = (MessageType)src_stream[0];
    header.payload_size = ntohl(*(uint32_t *)&src_stream[1]);
    *payload = (uint8_t *)malloc(header.payload_size);
    memcpy(*payload, src_stream + NETWORK_SERIALIZED_HEADER_SIZE, header.payload_size);
    *type = (MessageType *)malloc(sizeof(MessageType));
    **type = header.type;
    return header.payload_size;
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