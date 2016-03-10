#ifndef CONNECTOR_H
#define CONNECTOR_H
#include <string>
#include <thread>
using namespace std;
#include <boost/asio.hpp>
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost;

#include "RWHandler.h"

class Connector{
private:
    ios_service& m_ios;
    tcp::socket m_socket;
    tcp::endpoint m_serverAddr;
    std::shared_ptr<RWHandler> m_eventHandler;
    bool m_isConnected;
    std::shared_ptr<std::thread> m_chkThread;

public:
    Connector(io_service& ios,const string& strIP,short port):m_ios(ios),m_socket(ios),
        m_serverAddr(tcp::endpoint(address::from_string(strIP),port)),m_isConnected(false),m_chkThread(nullptr){
        CreateEventHandler(ios);
    }
    ~Connector(){
    }
    bool Start(){
        m_eventHandler->GetSocket().async_connect(m_serverAddr,[this](const boost::system::error_code& error){
            if(error){
                HandleConnectError(error);
                return false;
            }
            std::cout<<"connect ok"<<std::endl;
            m_isConnected=true;
            m_eventHandler->HandleRead();
        });
        std::this_thread::sleep_for(std::chrono::seconds(1));
        //boost::this_thread::sleep(boost::posix_time::seconds(1));
        return m_isConnected;
    }

    bool IsConnected()const{
        return m_isConnected;
    }

    void Send(char* data,int len){
        if(!m_isConnected)
            return;
        m_eventHandler->HandleWrite(data,len);
    }
    void AsyncSend(char* data,int len){
        if(!m_isConnected)
            return;
        m_eventHandler->HandleAsyncWrite(data,len);
    }
};

#endif // CONNECTOR_H
