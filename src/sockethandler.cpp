/*! \license
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * \copyright 2021 Dan Fernández
 *
 * \file Class definition for \p SocketHandler.
 *
 * \author fdaniel, trice2
 */

#include "sockethandler.hpp"


// Constructor initializes all member variables.
SocketHandler::SocketHandler( )
        :
        serverSocket_( ),
        clientSocket_( ),
        serverAddress_( ),
        clientAddress_( ),
        sin_size_( sizeof( struct sockaddr_in ) ),
        curTime_( time( NULL ) ),
        isClientConnected( false )
{

    // Clear server and client address information
    bzero( (char *) &serverAddress_, sizeof( serverAddress_ ) );
    bzero( (char *) &clientAddress_, sizeof( clientAddress_ ) );

}


SocketHandler::~SocketHandler( )
{

}


void SocketHandler::connectServer(
        const int32_t& type,
        const uint64_t& address,
        const uint16_t& port,
        const bool& isTCM )
{

    int opt = 1;

    // Generate a new socket
    serverSocket_ = socket( AF_INET, type, 0 );

    // Populate socket information; AF_INET used by default for internet routing
    serverAddress_.sin_family = AF_INET;
    serverAddress_.sin_addr.s_addr = inet_addr( (char*)address );
    serverAddress_.sin_port = htons( port );

    if( setsockopt( serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) ) != 0 )
    {
        printf( "setsockopt fail. SOL_SOCKET, SO_REUSEADDR port: %d", port );
    }

    // for the signalHandler, exit without calling ::bind
    if( isTCM == true )
    {
        std::cout << "---" << std::endl << "TCM socket created with address: ";
        std::cout << inet_ntoa( serverAddress_.sin_addr ) << ":";
        std::cout << ntohs( serverAddress_.sin_port ) << std::endl;

        return;
    }

    // bind (this socket, local address, address length)
    // bind server socket (serverSock) to server address (serverAddr).
    // Necessary so that server can use a specific port
    if (bind( serverSocket_,
              (struct sockaddr*)&serverAddress_,
              sizeof( serverAddress_ ) ) != 0)
    {
        std::cout << "socket bind failed" << std::endl;
    }

    if( type == SOCK_DGRAM && port == UDP_PORT )
    {

        std::cout << "---" << std::endl << "ASPM awaiting TCM connection on: ";
        std::cout << inet_ntoa( serverAddress_.sin_addr ) << ":";
        std::cout << ntohs( serverAddress_.sin_port ) << std::endl;

        return;
    }

    // With server connected, now listen for a new client to connect.
    listenForNewClient_( );


    return;

}


void SocketHandler::connectClient( )
{

    // Clear client address information
    bzero( (char *) &clientAddress_, sizeof( clientAddress_ ) );

    // Connect client and server sockets
    clientSocket_ = accept(
            serverSocket_,
            (struct sockaddr*)&clientAddress_,
            &sin_size_ );

    std::cout << "---" << std::endl << "Mobile device connected." << std::endl;
    std::cout << "Mobile Address: " << inet_ntoa( clientAddress_.sin_addr );
    std::cout << ":" << ntohs( clientAddress_.sin_port ) << std::endl;

    // Set bool value to true for reference in other classes.
    isClientConnected = true;


    return;

}


void SocketHandler::receiveTCP(
        ssize_t& val,
        std::string& receivedMsg,
        uint32_t& headerLen,
        uint32_t& msgLen )
{

    std::cout << "---" << std::endl << "TCM awaiting JSON input." << std::endl;
    std::cout << "Server Address: " << inet_ntoa( serverAddress_.sin_addr );
    std::cout << ":" << ntohs( serverAddress_.sin_port ) << std::endl;
    std::cout << "Mobile Address: " << inet_ntoa( clientAddress_.sin_addr );
    std::cout << ":" << ntohs( clientAddress_.sin_port ) << std::endl;
    std::cout << "---" << std::endl;

    headerLen = 0;
    msgLen = 0;

    // Read in message from client
    ssize_t header_len_bytes = read( clientSocket_, (void*)(&headerLen), sizeof(std::uint32_t));
    ssize_t msg_len_bytes = read( clientSocket_, (void*)(&msgLen), sizeof(std::uint32_t));
    if( header_len_bytes < 0 || msg_len_bytes < 0 )
    {
        std::cout << "read error" << std::endl;
        perror( "ERROR reading from socket." );

        val = -1;
    }
    else if ( header_len_bytes == 0 ) {
        std::cout << "TCP connection closed" << std::endl;
        disconnectClient();
        return;
    }
    else
    {
        headerLen = ntohl(headerLen);
        msgLen = ntohl(msgLen);
        uint32_t totalMsgLen( headerLen + msgLen );

        // Print time at receipt of new message
        curTime_ = time( NULL );
        std::cout << "Data received at: " << ctime( &curTime_ );

        receivedMsg.clear( );
        receivedMsg.resize( totalMsgLen );

        ssize_t bytes_read = 0;
        while (bytes_read < totalMsgLen) {
            val = (ssize_t)read( clientSocket_, (void*)(receivedMsg.data() + bytes_read), (totalMsgLen - bytes_read) );
            bytes_read += val;
        }
    }
}


ssize_t SocketHandler::receiveUDP( void* buffer, const uint16_t& bufSize )
{
    // std::cout << "---" << std::endl << "Awaiting UDP message on: ";
    // std::cout << inet_ntoa( serverAddress_.sin_addr ) << ":";
    // std::cout << ntohs( serverAddress_.sin_port ) << std::endl;
    // std::cout << "---" << std::endl;

    ssize_t receipt = recvfrom(
            serverSocket_,
            buffer,
            bufSize,
            0,
            (struct sockaddr *)&serverAddress_,
            &sin_size_ );


    return receipt;
}


void SocketHandler::sendTCP(
        const std::string& msgHeader,
        const std::string& msgBody )
{
    // leave per commonly-used state debugging statements.
    // std::cout << "headerLen = " << (int)msgHeader.length() << std::endl;
    std::cout << "rawHeader: " << msgHeader << std::endl;
    // std::cout << "bodyLen = " << (int)msgBody.length() << std::endl;
    std::cout << "rawBody: " << msgBody << std::endl;

    std::uint32_t headerSize = htonl(msgHeader.length());
    std::uint32_t bodySize = htonl(msgBody.length());

    // Send reply back to client
    if( write( clientSocket_, &headerSize, sizeof( headerSize ) ) < 0
        || write( clientSocket_, &bodySize, sizeof( bodySize ) ) < 0 )

    {
        perror( "ERROR writing to socket." );
    }
    else
    {
        write( clientSocket_, msgHeader.c_str( ), msgHeader.size( ) );
        write( clientSocket_, msgBody.c_str( ), msgBody.size( ) );
    }

    return;

}


int32_t SocketHandler::sendUDP( const void* buffer, const uint16_t& bufSize )
{

    // std::cout << "---" << std::endl << "Sending UDP message on: ";
    // std::cout << inet_ntoa( serverAddress_.sin_addr ) << ":";
    // std::cout << ntohs( serverAddress_.sin_port ) << std::endl;
    // std::cout << "---" << std::endl;


    return sendto(
            serverSocket_,
            ((uint8_t*)buffer),
            bufSize,
            0,
            (const struct sockaddr*)&serverAddress_,
            sin_size_ );
}


void SocketHandler::disconnectClient( bool listenForNew )
{
    close( clientSocket_ );

    // Set bool value to false for reference in other classes.
    isClientConnected = false;

    // With server connected, now listen for a new client to connect.
    if (listenForNew) {
        listenForNewClient_( );
    }


    return;

}


void SocketHandler::disconnectServer( )
{
    shutdown(clientSocket_, SHUT_RDWR);
    close( clientSocket_ );
    shutdown(serverSocket_, SHUT_RDWR);
    close( serverSocket_ );

    return;

}


int SocketHandler::checkClientConnection( )
{

    int error;
    socklen_t errorSize = sizeof( error );

    // If client socket has disconnected, return value of "-1" will be returned.
    return getsockopt(clientSocket_, SOL_SOCKET, SO_ERROR, &error, &errorSize);

}


void SocketHandler::listenForNewClient_( )
{

    // wait for a client
    /* listen (this socket, request queue length) */
    listen( serverSocket_, 5 );

    // Use this to repopulate server address information if it was cleared.
    getsockname( serverSocket_, (struct sockaddr*)&serverAddress_, &sin_size_ );

    std::cout << "---" << std::endl << "TCM awaiting mobile connection." << std::endl;
    std::cout << "Listening on: " << inet_ntoa( serverAddress_.sin_addr );
    std::cout << ":" << ntohs( serverAddress_.sin_port ) << std::endl;


    return;

}
