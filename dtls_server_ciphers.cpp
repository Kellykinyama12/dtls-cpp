// g++ -std=c++23 -I C:/msys64/mingw64/include -L C:/msys64/mingw64/lib -o dtls_server_ciphers dtls_server_ciphers.cpp -lssl -lcrypto -lws2_32 -lpthread

#include <iostream>
#include <asio.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <array>

using asio::ip::udp;

class DTLSServer
{
public:
    DTLSServer(asio::io_context &io_context, SSL_CTX *ssl_ctx, unsigned short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)),
          ssl_ctx_(ssl_ctx)
    {
        start_receive();
    }

private:
    void start_receive()
    {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), remote_endpoint_,
            [this](const asio::error_code &ec, std::size_t bytes_recvd)
            {
                if (!ec)
                {
                    handle_receive(bytes_recvd);
                }
                start_receive();
            });
    }

    void handle_receive(std::size_t bytes_recvd)
    {
        std::cout << "Received message: " << std::string(recv_buffer_.data(), bytes_recvd) << std::endl;

        // Here you would handle the DTLS handshake and encrypt data using OpenSSL

        std::string message = "Hello, DTLS client!";
        socket_.async_send_to(
            asio::buffer(message), remote_endpoint_,
            [this](const asio::error_code &ec, std::size_t /*bytes_sent*/)
            {
                if (!ec)
                {
                    std::cout << "Message sent to client." << std::endl;
                }
            });
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;
    SSL_CTX *ssl_ctx_;
};

SSL_CTX *create_ssl_context()
{
    const SSL_METHOD *method = DTLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if (!ctx)
    {
        throw std::runtime_error("Failed to create SSL context");
    }

    // Load the server certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0)
    {
        throw std::runtime_error("Failed to load certificate");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
    {
        throw std::runtime_error("Failed to load private key");
    }

    // Set the cipher suite
    if (!SSL_CTX_set_cipher_list(ctx, "ECDHE-ECDSA-AES128-GCM-SHA256"))
    {
        throw std::runtime_error("Failed to set cipher list");
    }

    return ctx;
}

int main()
{
    try
    {
        asio::io_context io_context;

        SSL_CTX *ssl_ctx = create_ssl_context();

        DTLSServer server(io_context, ssl_ctx, 12345);

        io_context.run();

        SSL_CTX_free(ssl_ctx);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
