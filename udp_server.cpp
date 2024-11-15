#include <iostream>
#include <asio.hpp>

// g++ -std=c++23 -I C:/msys64/mingw64/include -o udp_server udp_server.cpp -lws2_32
// g++ -std=c++23 -I C:/msys64/mingw64/include -o hello_server hello_server.cpp -lws2_32

using asio::ip::udp;

int main()
{
    try
    {
        asio::io_context io_context;

        // Create a UDP socket to listen for incoming connections
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));

        std::cout << "UDP server is running. Waiting for messages..." << std::endl;

        for (;;)
        {
            char data[1024];
            udp::endpoint sender_endpoint;
            std::size_t length = socket.receive_from(asio::buffer(data), sender_endpoint);

            std::cout << "Received message: " << std::string(data, length) << std::endl;

            std::string message = "Hello, UDP client!";
            socket.send_to(asio::buffer(message), sender_endpoint);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}