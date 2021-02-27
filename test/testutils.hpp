#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "json.hpp"
#include "aspm.hpp"

using json = nlohmann::json;

struct TCPMessage {
    std::string header;
    std::string body;
};

json constructHeader(const std::string& group_name);

// Represents the ASP socket connection for testing purposes
class StubASP : public ASPM {
public:
    StubASP(const std::string addr = "127.0.0.1", const unsigned int port = 8064) {
        int opt = 1;
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr( (char*)addr.c_str() );
        serv_addr.sin_port = htons(port);
        if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) ) != 0 ) {
            std::cout << "setsockopt SO_REUSEADDR failed" << std::endl;
        }
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if( setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) ) != 0 ) {
            std::cout << "setsockopt SO_RCVTIMEO failed" << std::endl;
        }
        if (bind(sock,
                 (struct sockaddr*)&serv_addr,
                 sizeof( serv_addr ) ) != 0)
        {
            std::cout << "socket bind failed" << std::endl;
        }
        ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
    }
    // Sends the current ASP state to the TCM server and ensures it receives it
    void sync() {
        ssize_t bytes_received = 0;
        socklen_t addr_len = sizeof( struct sockaddr_in );
        uint8_t bufferToTCM[ UDP_BUF_MAX ];
        uint8_t bufferFromTCM[ UDP_BUF_MAX ];
        bzero( bufferToTCM, UDP_BUF_MAX );
        bzero( bufferFromTCM, UDP_BUF_MAX );
        // clear initial send from TCM
        recvfrom(sock, bufferFromTCM, sizeof(bufferFromTCM), 0,(struct sockaddr *)&serv_addr, &addr_len);
        while (bytes_received <= 0) {
            uint16_t outBufSize = encodeASPMSignalData(bufferToTCM);
            ssize_t bytes_sent = sendto(
                sock,
                ((uint8_t*)bufferToTCM),
                outBufSize,
                0,
                (const struct sockaddr*)&serv_addr,
                addr_len );
            // If we receive a packet back, that means the TCM server received the packet we sent
            // timeout flag is set so that this call will only block for 1 second
            bytes_received = recvfrom(
                sock,
                bufferFromTCM,
                sizeof(bufferFromTCM),
                0,
                (struct sockaddr *)&serv_addr,
                &addr_len );
            decodeTCMSignalData(bufferFromTCM);
            if (bytes_received < 0) {
                std::cout << "WARNING: UDP packet not received by server...retrying" << std::endl;
            }
        }
    }
    void disconnect() {
        shutdown(sock, SHUT_RDWR);
        close( sock );
    }
private:
    int sock;
    struct sockaddr_in serv_addr;
};

// Represents the remote device for testing purposes
class MobileClient {
public:
    MobileClient(const std::string host = "localhost", const unsigned int port = 8063) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        struct hostent *server = gethostbyname(host.c_str());
        struct sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            throw std::runtime_error("ERROR: RD to TCM Connection failed");
        }
    }
    void send(const TCPMessage& msg) {
        uint32_t headerSize = htonl(msg.header.length());
        uint32_t bodySize = htonl(msg.body.length());
        if(write(sock, &headerSize, sizeof(headerSize)) < 0 ||
            write(sock, &bodySize, sizeof(bodySize)) < 0)
        {
            throw std::runtime_error("ERROR: failed to send header/body lengths");
        }
        if (write(sock, msg.header.c_str(), msg.header.size()) < 0 ||
            write(sock, msg.body.c_str(), msg.body.size()) < 0)
        {
            throw std::runtime_error("ERROR: failed to send header/body payloads");
        }
    }
    struct TCPMessage receive(bool ignore_vehicle_status = false) {
        struct TCPMessage msg;
        uint32_t headerSize;
        uint32_t bodySize;
        while (true) {
            if(read(sock, (void*)(&headerSize), sizeof(uint32_t)) < 0 ||
                read(sock, (void*)(&bodySize), sizeof(uint32_t)) < 0 )
            {
                throw std::runtime_error("ERROR: failed to receive header/body lengths");
            }
            headerSize = ntohl(headerSize);
            bodySize = ntohl(bodySize);
            msg.header.resize(headerSize);
            msg.body.resize(bodySize);
            if(read(sock, (void*)msg.header.data(), headerSize) < 0 ||
                read(sock, (void*)msg.body.data(), bodySize) < 0) {
                throw std::runtime_error("ERROR: failed to receive header/body payloads");
            }
            if (ignore_vehicle_status && json::parse(msg.header)["group"] == "vehicle_status") {
                continue;
            }
            else {
                break;
            }
        }
        return msg;
    }
    void disconnect() {
        shutdown(sock, SHUT_RDWR);
        close( sock );
    }
private:
    int sock;
};
