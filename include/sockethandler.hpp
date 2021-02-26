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
 * \file Header for \p SocketHandler class.
 *
 * \author fdaniel, trice2
 */

#if !defined( SOCKETHANDLER_HPP )
#define SOCKETHANDLER_HPP

#include <tuple>
#include <string>
#include <iostream>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

constexpr auto TCP_ADDR = "0.0.0.0";
constexpr auto UDP_ADDR = "127.0.0.1";
constexpr auto TCP_PORT = 8063;
constexpr auto UDP_PORT = 8064;


/*!
 * \brief Socket management object for communicating via TCP or UDP.
 *
 * Stores all address information, generates sockets, and sends/receives data
 * via TCP or UDP
 *
 * \warning
 * This class is constructed to be used in a bench test setup and may require
 * additional functionality once deployed to a vehicle VDC.
 *
 */
class SocketHandler
{

public:

    SocketHandler( );

    ~SocketHandler( );

    /*!
     * Public-accessible function to connect server socket to desired address.
     *
     * \param type  socket type to open
     * \param address  address to bind socket to
     * \param port port to bind socket to
     * \param isTCM  boolean value to signify SignalHandler is generating socket
     *
     */
    void connectServer(
            const int32_t& type,
            const uint64_t& address,
            const uint16_t& port,
            const bool& isTCM = false );

    /*!
     * Public-accessible function to connect client socket to desired address.
     */
    void connectClient( );

    /*!
     * Receive a message from a client and pass it to the server for parsing.
     *
     * \param readVal  Return value from message receipt
     * \param receivedMsg  String value of received message
     * \param headerLen  message header length
     * \param msgLen  message body length
     *
     */
    void receiveTCP(
            ssize_t& readVal,
            std::string& receivedMsg,
            uint32_t& headerLen,
            uint32_t& msgLen );

    /**
     * Receive a message from a ASPM and pass it to the server for parsing.
     * \param[in] buffer  raw message received.
     * \param[in] bufSize  size of message buffer.
     */
    ssize_t receiveUDP( void* buffer, const uint16_t& bufSize );

    /*!
     * Public-accessible function to send desired message from server to client.
     *
     * \param msgHeader  String value of message header to be sent
     * \param msgBody  String value of message body to be sent
     *
     */
    void sendTCP(
            const std::string& msgHeader,
            const std::string& msgBody );

    /*!
     * Public-accessible function to send desired message from server to client.
     *
     * \param buffer  message buffer to be sent
     * \param bufSize  size of message buffer to be sent
     *
     */
    int32_t sendUDP( const void* buffer, const uint16_t& bufSize );

    /*!
     * Public-accessible function to disconnect client socket from server.
     *
     * \param listen_for_new  true if server should begin listening for another client to connect
     */
    void disconnectClient( bool listen_for_new = true );

    /*!
     * Public-accessible function to disconnect server socket.
     */
    void disconnectServer( );

    /*!
     * Public-accessible function to check client / server socket connection.
     *
     * \return int  Return value from socket check; "0" means good connection
     */
    int checkClientConnection( );

    /*!
     * Public-accessible reference for if client is currently connected.
     */
    bool isClientConnected;


private:

    /*!
     * Used to set server socket to listen for a new client.
     */
    void listenForNewClient_( );

    /*!
     * int to hold server socket information.
     */
    int serverSocket_;

    /*!
    * int to hold client socket information.
     */
    int clientSocket_;

    /*!
    * struct to hold server socket information.
     */
    struct sockaddr_in serverAddress_;

    /*!
    * struct to hold client socket information.
     */
    struct sockaddr_in clientAddress_;

    /*!
    * socklen_t to hold socket size information.
     */
    socklen_t sin_size_;

    /*!
     * time_t value for storing current time.
     */
    time_t curTime_;

};


#endif //SOCKETHANDLER_HPP
