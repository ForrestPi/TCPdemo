//hwserver.c: Hello World server
//https://github.com/anjuke/zguide-cn/blob/master/chapter1.md
//
//  Hello World �����
//  ��һ��REP�׽�����tcp://*:5555
//  �ӿͻ��˽���Hello����Ӧ��World
//
//
// Hello World client
// Connects REQ socket to tcp://localhost:5555
// Sends "Hello" to server, expects "World" back
//
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include "unistd.h"

int main (void)
{
    void *context = zmq_init (1);

    //  ��ͻ���ͨ�ŵ��׽���
    void *responder = zmq_socket (context, ZMQ_REP);
    zmq_bind (responder, "tcp://*:5555");

    while (1) {
        //  �ȴ��ͻ�������
        zmq_msg_t request;
        zmq_msg_init (&request);
        zmq_recv (responder, &request,10, 0);
        printf ("�յ� Hello\n");
        zmq_msg_close (&request);

        //  ��Щ������
        Sleep (1000);  //VC:Sleep(ms)  Linux:sleep(s)

        //  ����Ӧ��
        zmq_msg_t reply;
        zmq_msg_init_size (&reply, 5);
        memcpy (zmq_msg_data (&reply), "World", 5);
        zmq_send (responder, &reply,5, 0);
        zmq_msg_close (&reply);
    }
    //  ���򲻻����е��������ֻ����ʾ����Ӧ����ν���
    zmq_close (responder);
    zmq_term (context);
    return 0;
}