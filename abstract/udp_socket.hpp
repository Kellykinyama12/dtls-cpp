#ifndef UDPSOCKET_HPP
#define UDPSOCKET_HPP

#include "dtls_socket.hpp"
#include <asio.hpp>
#include <memory>

class UDPSocket : public Socket
{
public:
    UDPSocket(asio::io_context &io_context, const std::string &server_ip, unsigned short port)
        : socket_(io_context, asio::ip::udp::v4())
    {
        remote_endpoint_ = *asio::ip::udp::resolver(io_context).resolve(server_ip, std::to_string(port)).begin();
    }

    int send(const void *data, size_t size) override
    {
        return socket_.send_to(asio::buffer(data, size), remote_endpoint_);
    }

    int receive(void *data, size_t size) override
    {
        asio::ip::udp::endpoint sender_endpoint;
        return socket_.receive_from(asio::buffer(data, size), sender_endpoint);
    }

    void set_peer_address(const std::string &address, unsigned short port) override
    {
        remote_endpoint_ = asio::ip::udp::endpoint(asio::ip::make_address(address), port);
    }

private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
};

#endif // UDPSOCKET_HPP
