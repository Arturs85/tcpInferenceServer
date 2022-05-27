#include <torch/script.h> // One-stop header.

#include <iostream>
#include <memory>
#include <fstream>

#include <bits/stdc++.h>

#include "inference.hpp"
#include "tcpserver.hpp"

int main(int argc, const char* argv[]) {

    TcpServer tcpServer;
    Inference inference;
    tcpServer.inference = &inference;

    tcpServer.startServerSocket();
    tcpServer.startWaitForClientThread();
    while(true){

usleep(100000);
    };
}
