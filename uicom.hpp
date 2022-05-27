#ifndef UICOM_HPP
#define UICOM_HPP


// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string>


class UiCom{
public:
    static const std::string TAG;
static void sendCount(int count);

};

#endif // UICOM_HPP
