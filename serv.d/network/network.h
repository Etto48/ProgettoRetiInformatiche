#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/select.h>
#include "../integrity/integrity.h"
#include "../../global.d/debug/debug.h"
#include "../../global.d/network_tools/network_tools.h"

#define NETWORK_MAX_CONNECTIONS 1023

/**
 * @brief main loop of the server, here we listen for new connections and we handle open connections
 * 
 * @param port port on which we listen for connections
 */
void NetworkServerMainLoop(uint16_t port);