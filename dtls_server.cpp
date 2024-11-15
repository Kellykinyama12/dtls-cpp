// g++ -std=c++23 -I C:/msys64/mingw64/include -L C:/msys64/mingw64/lib -o dtls_server dtls_server.cpp -lssl -lcrypto -lws2_32 -lpthread

#include <iostream>
#include <array>
#include <asio.hpp>
#include <asio/ssl.hpp>

using asio::ip::udp;

class DTLSServer
{
public:
    DTLSServer(asio::io_context &io_context, asio::ssl::context &ssl_context, unsigned short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)),
          ssl_context_(ssl_context)
    {
        start_receive();
    }

private:
    void start_receive()
    {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), remote_endpoint_,
            [this](std::error_code ec, std::size_t bytes_recvd)
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

        std::string message = "Hello, DTLS client!";
        socket_.async_send_to(
            asio::buffer(message), remote_endpoint_,
            [this](std::error_code ec, std::size_t /*bytes_sent*/)
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
    asio::ssl::context &ssl_context_;
};

int main()
{
    try
    {
        asio::io_context io_context;
        asio::ssl::context ssl_context(asio::ssl::context::tlsv12_server);

        // Load server certificate and private key
        ssl_context.use_certificate_chain_file("server.crt");
        ssl_context.use_private_key_file("server.key", asio::ssl::context::pem);

        DTLSServer server(io_context, ssl_context, 12345);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}