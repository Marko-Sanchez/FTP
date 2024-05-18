#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <atomic>
#include <future>

#include "LoadBalancer.hpp"

void bindSocket(int socket, uint16_t port, struct sockaddr_in& server_address)
{
    // TODO: currently listens on localhost, look into INADDR_ANY.
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "Error: Could not bind socket" << std::endl;
        exit(EXIT_FAILURE);
    }
}

/*
 * Waits for user to enter 'q' to exit the program.
 * @param listening: atomic boolean to signal the program to exit.
 *
 * @note temporary function to test the program.
 */
void programExit(std::atomic<bool>& listening)
{
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nUNBLOCK\r\n\r\n";
    struct sockaddr_in server_address{};

    int unblock_socket{socket(AF_INET, SOCK_STREAM, 0)};
    if (unblock_socket == -1)
    {
        std::cerr << "Error: Could not create unblock socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    bindSocket(unblock_socket, 8080, server_address);

    // TODO: improve in future.
    char input;
    while (true)
    {
        std::cin >> input;
        if (input == 'q')
        {
            listening.store(false);

            // Connect to unblock the accept() call.
            if (connect(unblock_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
            {
                std::cerr << "Error: Could not connect to unblock socket" << std::endl;
                close(unblock_socket);
                break;
            }

            if (send(unblock_socket, response, sizeof(response), MSG_DONTWAIT) == -1)
            {
                std::cerr << "Error: Could not send unblock message" << std::endl;
                close(unblock_socket);
                break;
            }
            break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

int main()
{
    const std::string ftp_server{"localhost"};
    const uint16_t ftp_port{8080};

    std::cout << "FTP server: " << ftp_server << std::endl;
    std::cout << "FTP port: " << ftp_port << std::endl;

    int server_socket{socket(AF_INET, SOCK_STREAM, 0)};
    if (server_socket == -1)
    {
        std::cerr << "Error: Could not create socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address{};
    bindSocket(server_socket, ftp_port, server_address);

    if (listen(server_socket, SOMAXCONN) == -1)
    {
        std::cerr << "Error: Could not listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections.
    sockaddr_in client_address{};
    socklen_t client_address_size{sizeof(client_address)};
    char ip_buffer[INET_ADDRSTRLEN];

    LoadBalancer lb;

    std::atomic<bool> listening{true};
    auto unused_fut = std::async(std::launch::async, programExit, std::ref(listening));

    while (listening.load())
    {
        // accept blocks.
        int client_socket{accept(server_socket, (struct sockaddr*)&client_address, &client_address_size)};
        if (client_socket == -1)
        {
            std::cerr << "Error: Could not accept incoming connection" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Accepted connection" << std::endl;
        inet_ntop(AF_INET, &client_address.sin_addr, ip_buffer, INET_ADDRSTRLEN);
        std::cout << "Client IP: " << ip_buffer << std::endl;

        lb.AddJob(client_socket);
    }

    close(server_socket);
    exit(EXIT_SUCCESS);
}
