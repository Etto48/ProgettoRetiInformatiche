#include "network.h"

bool NetworkShutdownRequested = false;

void NetworkHandleNewMessage(int sockfd)
{
    switch (NetworkConnectedDevices[sockfd].mh.type)
    {
    case MESSAGE_SIGNUP:
        NetworkHandleSignup(sockfd);
        break;
    case MESSAGE_LOGIN:
        NetworkHandleLogin(sockfd);
        break;
    case MESSAGE_LOGOUT:
        NetworkHandleLogout(sockfd);
        break;
    case MESSAGE_HANGING:
        NetworkHandleHanging(sockfd);
        break;
    case MESSAGE_USERINFO_REQ:
        NetworkHandleUserinfoReq(sockfd);
        break;
    case MESSAGE_DATA:
        NetworkHandleData(sockfd);
        break;
    default:
        NetworkHandleError(sockfd);
    }
}

void NetworkHandleSignup(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    UserName username;
    Password password;
    NetworkDeserializeMessageSignup(ncd->mh.payload_size, ncd->receive_buffer, &username, &password);
    bool ok = AuthRegister(username, password);
    if (ok)
        printf("%s created a new account\n", username.str); // verbose
    NetworkSendMessageResponse(sockfd, ok);
}

void NetworkHandleLogin(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    uint16_t port;
    UserName username;
    Password password;
    NetworkDeserializeMessageLogin(ncd->mh.payload_size, ncd->receive_buffer, &port, &username, &password);
    bool ok = IndexLogin(username, password, ntohl(ncd->address.sin_addr.s_addr), port);
    if (ok)
    {
        ncd->username = username;
        printf("%s logged in\n", username.str); // verbose
    }
    NetworkSendMessageResponse(sockfd, ok);
    if (ok)
    { // we check if user has a pending syncread message
        RelaySyncreadNotice *rsn;
        while ((rsn = RelaySyncreadFind(&username, NULL)))
        {
            if (NetworkSendMessageSyncread(sockfd, rsn->dst, rsn->timestamp))
                RelaySyncreadDelete(&username, NULL);
            else
                break; // we failed to send a syncread message so we can't delete it, we will try next time
        }
    }
}

void NetworkHandleLogout(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    bool ok;
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        ok = IndexLogout(ncd->username);
        if (ok)
        {
            printf("%s logged out\n", ncd->username.str); // verbose
            memset(ncd->username.str, 0, USERNAME_MAX_LENGTH + 1);
        }
    }
    else
        ok = false;
    NetworkSendMessageResponse(sockfd, ok);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    NetworkDeleteConnection(sockfd);
}

/**
 * @brief this struct is used to memorize the usernames that need to be sent with a response to a MESSAGE_HANGING
 *
 */
typedef struct _UserNameList
{
    UserName user;
    struct _UserNameList *next;
} UserNameList;
/**
 * @brief find a username in a list made from the previous struct
 *
 * @param list pointer to the list
 * @param user username
 * @return pointer to the entry if found, NULL otherwise
 */
UserNameList *UserNameListFind(UserNameList *list, UserName user)
{
    for (UserNameList *j = list; j; j = j->next)
    {
        if (strncmp(j->user.str, user.str, USERNAME_MAX_LENGTH) == 0)
        {
            return j;
        }
    }
    return NULL;
}
/**
 * @brief add an entry to a username list
 *
 * @param list pointer to the list
 * @param user username to add
 */
void UserNameListAdd(UserNameList **list, UserName user)
{
    UserNameList *target = (UserNameList *)malloc(sizeof(UserNameList));
    target->user = user;
    target->next = *list;
    *list = target;
}
/**
 * @brief delete a username list
 *
 * @param list pointer to the list
 */
void UserNameListDelete(UserNameList **list)
{
    UserNameList *next = NULL;
    for (UserNameList *i = *list; i; i = next)
    {
        next = i->next;
        free(i);
    }
    *list = NULL;
}

void NetworkHandleHanging(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        UserName from;
        NetworkDeserializeMessageHanging(ncd->mh.payload_size, ncd->receive_buffer, &from);
        if (from.str[0] == '\0')
        { // only users
            UserNameList *list = NULL;
            for (RelayMessage *i = RelayHangingFindFirst(RelayHangingList, NULL, &ncd->username); i; i = RelayHangingFindFirst(i->next, NULL, &ncd->username))
            {
                if (!UserNameListFind(list, i->src))
                {
                    UserNameListAdd(&list, i->src);
                    NetworkSendMessageHanging(sockfd, &i->src);
                }
            }
            UserNameListDelete(&list);
            printf("%s requested a list of hanging contacts\n", ncd->username.str); // verbose
        }
        else
        { // only messages from user
            time_t last_timestamp = 0;
            for (RelayMessage *i = RelayHangingPopFirst(&from, &ncd->username); i; i = RelayHangingPopFirst(&from, &ncd->username))
            {
                switch (i->type)
                {
                case RELAY_MESSAGE_TEXT:
                    NetworkSendMessageDataText(sockfd, from, ncd->username, i->timestamp, (char *)i->data);
                    break;
                case RELAY_MESSAGE_FILE:
                    NetworkSendMessageDataFileBuffer(sockfd, from, ncd->username, i->timestamp, i->filename, i->data_size, i->data);
                    break;
                }
                last_timestamp = i->timestamp > last_timestamp ? i->timestamp : last_timestamp;
                RelayHangingDestroyMessage(i);
            }
            if (last_timestamp > 0)
            { // now we must send a message syncread to the other end if possible
                RelaySyncreadEdit(from, ncd->username, last_timestamp);
            }
            printf("%s requested a list of hanging messages from %s\n", ncd->username.str, from.str); // verbose
        }
        NetworkSendMessageResponse(sockfd, true);
    }
    else
        NetworkSendMessageResponse(sockfd, false);
}

void NetworkHandleUserinfoReq(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        UserName username;
        NetworkDeserializeMessageUserinfoReq(ncd->mh.payload_size, ncd->receive_buffer, &username);

        if (AuthExists(username))
        {
            IndexEntry *res = IndexGetOnline(username);
            uint32_t ip = res ? res->ip : 0;
            uint16_t port = res ? res->port : 0;
            NetworkSendMessageUserinfoRes(sockfd, ip, port);
            printf("%s requested info about %s\n", ncd->username.str, username.str); // verbose
        }
        else
            NetworkSendMessageResponse(sockfd, false);
    }
    else
        NetworkSendMessageResponse(sockfd, false);
}

void NetworkHandleData(int sockfd)
{
    NetworkDeviceConnection *ncd = &NetworkConnectedDevices[sockfd];
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        if (NetworkMessageDataContainsFile(ncd->mh.payload_size, ncd->receive_buffer))
        { // file
            size_t filename_len = NetworkMessageDataFilenameLength(ncd->mh.payload_size, ncd->receive_buffer);
            size_t file_size = NetworkMessageDataFileSize(ncd->mh.payload_size, ncd->receive_buffer);
            char *filename = (char *)malloc(filename_len + 1);
            uint8_t *file_data = (uint8_t *)malloc(file_size);
            UserName src;
            UserName dst;
            time_t timestamp;
            NetworkDeserializeMessageDataFile(ncd->mh.payload_size, ncd->receive_buffer, &src, &dst, &timestamp, filename, file_data);
            // we must use ncd->username instead of src to avoid security issues
            RelayHangingAdd(ncd->username, dst, timestamp, RELAY_MESSAGE_FILE, filename, file_size, file_data);
            free(filename);
            free(file_data);
            printf("%s sent a file to %s\n", ncd->username.str, dst.str); // verbose
        }
        else
        { // text
            size_t text_len = NetworkMessageDataTextLength(ncd->mh.payload_size, ncd->receive_buffer);
            char *text = (char *)malloc(text_len + 1);
            UserName src;
            UserName dst;
            time_t timestamp;
            NetworkDeserializeMessageDataText(ncd->mh.payload_size, ncd->receive_buffer, &src, &dst, &timestamp, text);
            // we must use ncd->username instead of src to avoid security issues
            RelayHangingAdd(ncd->username, dst, timestamp, RELAY_MESSAGE_TEXT, NULL, text_len, (uint8_t *)text);
            free(text);
            printf("%s sent a text message to %s\n", ncd->username.str, dst.str); // verbose
        }
        // NetworkSendMessageResponse(sockfd, true);
    }
    // else
    // NetworkSendMessageResponse(sockfd, false);
}

void NetworkHandleError(int sockfd)
{
    NetworkSendMessageResponse(sockfd, false);
}

void NetworkFreeTime()
{
    if (NetworkShutdownRequested)
        SaveAndExit(0);

    static time_t last_time = 0;
    time_t this_time = time(NULL);
    if (last_time == 0 || this_time - last_time > AUTOSAVE_TIME_INTERVAL)
    {
        last_time = this_time;
        Save();
    }
}

void NetworkDeletedConnectionHook(int sockfd)
{
    NetworkDeviceConnection *ncd = NetworkConnectedDevices + sockfd;
    if (NetworkIsSocketLoggedIn(sockfd))
    {
        IndexLogout(ncd->username);
        printf("%s disconnected\n", ncd->username.str); // verbose
    }
}