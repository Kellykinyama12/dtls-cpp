#include <asio.hpp>
#include <iostream>
#include "dtls_socket.hpp"
#include "udp_socket.hpp"

int main()
{
    try
    {
        asio::io_context io_context;

        // Create the user-defined UDP socket
        std::shared_ptr<Socket> udp_socket = std::make_shared<UDPSocket>(io_context, "127.0.0.1", 12345);

        // Create the DTLS client socket with user-defined UDP socket
        DTLSocket client(udp_socket, DTLSocket::Mode::CLIENT);

        // Enable SRTP before handshake
        client.enable_srtp();

        // Perform the DTLS handshake
        client.perform_handshake();

        // Send a message
        client.send_message("Hello, DTLS Server!");

        // Receive a response
        std::string response = client.receive_message();
        std::cout << "Received response: " << response << std::endl;

        // Extract keying material
        std::vector<unsigned char> keying_material = client.extract_keying_material();
        std::cout << "Extracted " << keying_material.size() << " bytes of keying material" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
