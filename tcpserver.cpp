#include "tcpserver.hpp"
#include <signal.h>

#include <iostream>
#include <thread>
#include "inference.hpp"
#define SA struct sockaddr
const std::string TcpServer::TAG = "[tcpServer] ";

TcpServer::TcpServer(){
    //inference = new Inference();
}

void TcpServer::startWaitForClientThread(){

    //   struct sigaction sigIntHandler;

    //  sigIntHandler.sa_handler = &TcpServer::handlerSigpipe;
    //  sigemptyset(&sigIntHandler.sa_mask);
    //  sigIntHandler.sa_flags = 0;

    // sigaction(SIGINT, &sigIntHandler, NULL);
    hasClientConnected = false;
    std::thread t1(&TcpServer::waitForClient,this); // passing 'this' by
    t1.detach();
}

void TcpServer::startReadingThread(){

    std::thread t1(&TcpServer::receive,this);
    t1.detach();
}

void TcpServer::startServerSocket()
{
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        //  exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(receivingPort);

    // Binding newly created socket to given IP and verification
    while ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        usleep(1000000);
        //   exit(0);
    }

    printf("Socket successfully binded..\n");




}
void TcpServer::waitForClient()
{
    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) != 0) {
        printf("Listen failed...\n");
        //     exit(0);
    }
    else
        printf("Server listening..\n");
    lengthOfCliAdrr = sizeof(cliaddr);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cliaddr, &lengthOfCliAdrr);
    if (connfd < 0) {
        printf("server accept failed...\n");
        //     exit(0);
    }
    else{
        printf("server accept the client...\n");
        hasClientConnected = true;
        startReadingThread();
    }
    std::string s = "from Server";
    //  int sent =  send (connfd, s.data(), s.size(),MSG_DONTWAIT);

    //if(sent<0)std::cout<<TAG<<" could not send: "<<s<<std::endl;

    std::cout<<TAG<< "started  thread, connfd: "<<connfd<<std::endl;
}

void TcpServer::sendString(std::string s)
{
    if(hasClientConnected){
        int sent =  send (connfd, s.data(), s.size(),MSG_DONTWAIT|MSG_NOSIGNAL);

        if(sent<0){
            std::cout<<TAG<<"connfd: "<<connfd<<" , could not send: "<<s<<std::endl;
            startWaitForClientThread();
        }
    }
}

void TcpServer::receive(){
    int framesReceived =0;
    int overflow =0;
    int lengthOfFrame = 0;
    while(hasClientConnected){
        int firstRead = 0;
        if(overflow>0){
            memcpy(buffer,buffer+lengthOfFrame+4,overflow);
            firstRead = overflow;
            overflow =0;
        }else
            firstRead = read(connfd, buffer, MAXLINE);
        //int   n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &lengthOfCliAdrr);
        if(firstRead>=4){  lengthOfFrame = *((int*)buffer);       std::cout<<TAG<<"received length: "<< lengthOfFrame<<std::endl;
            if(lengthOfFrame<100 || lengthOfFrame >MAXLINE){
                close(connfd);
                std::cout<<TAG<<" resetting conection because of bad length = "<<lengthOfFrame<<std::endl;
                hasClientConnected = false;
                startWaitForClientThread();
                return;
            }
            int frameBytesReceived =firstRead-4;
            while(frameBytesReceived<lengthOfFrame){
                int  n = read(connfd, buffer+frameBytesReceived+4, MAXLINE);
                if(n <=0){// skip unknown data untill size of frame is received
                    usleep(10000);
                    std::cout<<TAG<<" in frame n = "<<n<<std::endl;
                    hasClientConnected = false;
                    startWaitForClientThread();
                    return;
                }

                frameBytesReceived+=n;
                if(frameBytesReceived> lengthOfFrame){// there is part of next framein this data
                    overflow =  frameBytesReceived -lengthOfFrame;
                    std::cout<<TAG<<"handling overflow of bytes:  "<<overflow <<std::endl;
                }

                //std::cout<<TAG<<"received part "<<n <<" ,total received: "<<frameBytesReceived<<std::endl;

            }
            std::cout<<TAG<<"received whole frames: "<<++framesReceived<<std::endl;

            inference->infere(lengthOfFrame,buffer+4);
            if(!inference->wasValidImageData){
                //reset connection
                close(connfd);
                std::cout<<TAG<<" resetting conection because of bad image data = "<<lengthOfFrame<<std::endl;
                hasClientConnected = false;
                startWaitForClientThread();
                return;

            }

        }else
            if(firstRead >0){// skip unknown data untill size of frame is received
                std::cout<<TAG<<"received bad amount of data: "<<firstRead<<std::endl;

            }else{
                usleep(10000);
                std::cout<<TAG<<" n = "<<firstRead<<std::endl;
                hasClientConnected = false;
                startWaitForClientThread();
            }


    }
}

void TcpServer::handlerSigpipe(int s){
    //  Control::pathExecutor.te.motorControl->rc->shutDown();
    //usleep(1000);

    //Control::pathExecutor.te.motorControl->rc->shutDown();
    std::cout<<"Caught signal"<<  s<<std::endl;;

    exit(0);

}

