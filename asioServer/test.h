#ifndef TEST_H
#define TEST_H

#include "Server.h"

void TestServer(){
    io_service ios;
    Server server(ios,9900);
    server.Accept();
    ios.run();
}
#endif // TEST_H
