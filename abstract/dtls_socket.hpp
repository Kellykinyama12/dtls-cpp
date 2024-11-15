#ifndef DTLSSOCKET_HPP
#define DTLSSOCKET_HPP

#include <asio.hpp>
#include <openssl/ssl.h>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

// Generic interface for user-defined sockets
class Socket
{
public:
    virtual ~Socket() {}
    virtual int send(const void *data, size_t size) = 0;
    virtual int receive(void *data, size_t size) = 0;
    virtual void set_peer_address(const std::string &address, unsigned short port) = 0;
};

class DTLSocket
{
public:
    enum class Mode
    {
        CLIENT,
        SERVER
    };

    // Constructor that accepts a user-defined socket
    DTLSocket(std::shared_ptr<Socket> socket, Mode mode, const std::string &server_ip = "", unsigned short port = 0);
    ~DTLSocket();

    void send_message(const std::string &message);
    std::string receive_message();

    void perform_handshake();
    void enable_srtp(const std::string &profile = "SRTP_AES128_CM_SHA1_80");
    std::vector<unsigned char> extract_keying_material(size_t length = 64);

private:
    SSL_CTX *create_ssl_context();

    std::shared_ptr<Socket> socket_;
    Mode mode_;
    SSL_CTX *ssl_ctx_;
    SSL *ssl_;
    std::array<char, 1024> recv_buffer_;
    bool srtp_enabled_;
};

#endif // DTLSSOCKET_HPP
