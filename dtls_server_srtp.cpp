//// g++ -std=c++23 -I C:/msys64/mingw64/include -L C:/msys64/mingw64/lib -o dtls_server_srtp dtls_server_ciphers.cpp -lssl -lcrypto -lws2_32 -lpthread

#include <iostream>
#include <asio.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <array>
#include <vector>

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

        // Simulate DTLS handshake (keying material extraction example)
        SSL *ssl = create_ssl_for_client(); // Create a per-client SSL object
        if (ssl)
        {
            extract_keying_material(ssl);
            SSL_free(ssl);
        }

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

    SSL *create_ssl_for_client()
    {
        SSL *ssl = SSL_new(ssl_ctx_);
        if (!ssl)
        {
            std::cerr << "Failed to create SSL object for client." << std::endl;
            return nullptr;
        }

        // Associate the SSL object with the socket (not shown for simplicity)
        return ssl;
    }

    void extract_keying_material(SSL *ssl)
    {
        const char *label = "EXTRACTOR-dtls-srtp";
        unsigned char key_material[64]; // Example buffer size
        if (SSL_export_keying_material(ssl, key_material, sizeof(key_material), label, strlen(label), nullptr, 0, 0) == 1)
        {
            std::cout << "Keying material extracted: ";
            for (size_t i = 0; i < sizeof(key_material); ++i)
            {
                std::cout << std::hex << (key_material[i] & 0xFF);
            }
            std::cout << std::endl;
        }
        else
        {
            std::cerr << "Failed to extract keying material." << std::endl;
        }
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

    // Enable use-SRTP with supported profiles
    if (SSL_CTX_set_tlsext_use_srtp(ctx, "SRTP_AES128_CM_SHA1_80:SRTP_AES128_CM_SHA1_32") != 0)
    {
        throw std::runtime_error("Failed to set use-SRTP profiles");
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
