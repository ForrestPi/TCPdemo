#ifndef RWHANDLER_H
#define RWHANDLER_H

#include <array>
#include <functional>
#include <iostream>
using namespace std;

#include <boost/asio.hpp>
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost;

const int MAX_IP_PACK_SIZE=65536;
const int HEAD_LEN=4;

class RWHandler{
private:
    tcp::socket m_sock;
    std::array<char,MAX_IP_PACK_SIZE> m_buff;
    int m_connId;
    std::function<void(int)>m_callbackError;
public:
    RWHandler(io_service& ios):m_sock(ios){}
    ~RWHandler(){}
    void HandleRead(){
        async_read(m_sock,buffer(m_buff),transfer_at_least(HEAD_LEN),
                   [this](const boost::system::error_code& ec,size_t size)
        {
            if(ec!=nullptr)
            {
                HandleError(ec);
                return;
            }
            std::count<<m.buffer.data()+HEAD_LEN<<std::endl;
            HandleRead();
        });
    }

    void HandleWrite(char* data,int len){
        boost::system::error_code ec;
        write(m_sock,buffer(data,len),ec);
        if(ec!=nullptr)
            HandleError(ec);
    }
    tcp::socket& GetSocket(){
        return m_sock;
    }

    void closesocket(){
        boost::system::error_code ec;
        m_sock.shutdown(tcp::socket::shutdown_send,ec);
        m_sock.close(ec);
    }

    void SetConnId(int connId){
        m_connId=connId;
    }

    int GetConnId()const{
        return m_connId;
    }

    template<typename F>
    void SetCallBackError(F f){
        m_callbackError=f;
    }
private:
    void HandleError(const boost::system::error_code& ec){
        closesocket();
        std::count<<ec.message()<<std::endl;
        if(m_callbackError)
            m_callbackError(m_connId);
    }
};

#endif // RWHANDLER_H
