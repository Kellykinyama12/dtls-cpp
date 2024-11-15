#include <asio.hpp>
#include <iostream>
#include <thread>
#include "dtls_socket.hpp"
#include "udp_socket.hpp"

// DTLS Server Section
void run_dtls_server(asio::io_context &io_context)
{
    try
    {
        // Create the user-defined UDP socket for the server
        std::shared_ptr<Socket> udp_socket = std::make_shared<UDPSocket>(io_context, "127.0.0.1", 12345);

        // Create the DTLS server socket with the user-defined UDP socket
        DTLSocket server(udp_socket, DTLSocket::Mode::SERVER);

        // Perform the DTLS handshake (server side)
        server.perform_handshake();

        // Receive a message
        std::string message = server.receive_message();
        std::cout << "Server received: " << message << std::endl;

        // Send a response back to the client
        server.send_message("Hello from DTLS Server!");

        // Extract keying material (just an example, not used here)
        std::vector<unsigned char> keying_material = server.extract_keying_material();
        std::cout << "Server extracted " << keying_material.size() << " bytes of keying material" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

// DTLS Client Section
void run_dtls_client(asio::io_context &io_context)
{
    try
    {
        // Create the user-defined UDP socket for the client
        std::shared_ptr<Socket> udp_socket = std::make_shared<UDPSocket>(io_context, "127.0.0.1", 12345);

        // Create the DTLS client socket with the user-defined UDP socket
        DTLSocket client(udp_socket, DTLSocket::Mode::CLIENT);

        // Enable SRTP before handshake
        client.enable_srtp();

        // Perform the DTLS handshake (client side)
        client.perform_handshake();

        // Send a message to the server
        client.send_message("Hello from DTLS Client!");

        // Receive the response from the server
        std::string response = client.receive_message();
        std::cout << "Client received: " << response << std::endl;

        // Extract keying material (just an example, not used here)
        std::vector<unsigned char> keying_material = client.extract_keying_material();
        std::cout << "Client extracted " << keying_material.size() << " bytes of keying material" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main()
{
    asio::io_context io_context;

    // Run server in a separate thread
    std::thread server_thread([&]()
                              { run_dtls_server(io_context); });

    // Allow the server to start first
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Run client
    run_dtls_client(io_context);

    // Wait for the server thread to finish
    server_thread.join();

    return 0;
}
