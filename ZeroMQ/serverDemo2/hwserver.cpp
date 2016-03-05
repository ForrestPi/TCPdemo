//hwserver.cpp: Hello World server
//https://github.com/anjuke/zguide-cn/blob/master/chapter1.md
//
// Hello World ����� C++���԰�
// ��һ��REP�׽�����tcp://*:5555
// �ӿͻ��˽���Hello����Ӧ��World
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include "unistd.h"

int main () {
    // ׼�������ĺ��׽���
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5555");

    while (true) {
        zmq::message_t request;

        // �ȴ��ͻ�������
        socket.recv (&request);
        std::cout << "�յ� Hello" << std::endl;

        // ��һЩ������
        Sleep (1000); //VC:Sleep(ms)  Linux:sleep(s)

        // Ӧ��World
        zmq::message_t reply (5);
        memcpy ((void *) reply.data (), "World", 5);
        socket.send (reply);
    }
    return 0;
}