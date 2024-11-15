#include <iostream>
#include <asio.hpp>

// g++ -std=c++23 -I C:/msys64/mingw64/include -o hello_server hello_server.cpp -lws2_32

using asio::ip::tcp;

int main()
{
    try
    {
        asio::io_context io_context;

        // Create a TCP acceptor to listen for incoming connections
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

        std::cout << "Server is running. Waiting for connections..." << std::endl;

        // Wait for a client to connect
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        std::string message = "Hello, World!\n";

        // Send the message to the client
        asio::write(socket, asio::buffer(message));

        std::cout << "Message sent to client." << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}