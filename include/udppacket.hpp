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
 */

#include "constants.h"

/*!
 * Describes the type of UDP packet to process
 */
enum class UdpPacketType : uint8_t {
    SendTCMPacket = 0,   //!< Use when sending a packet from the TCM to the ASPM
    ReadASPMPacket = 1,  //!< Use when receiving an ASPM packet in the TCM
    SendASPMPacket = 2,  //!< Use when sending a packet from the ASPM to the TCM
    ReadTCMPacket = 3    //!< Use when receiving a TCM packet in the ASPM
};

/*!
 * Container for the 8-byte header of each PDU in the UDP packet
 */
typedef struct
{
    unsigned int header_id;  //!< PDU identifier (4 bytes)
    unsigned int length;     //!< Length of payload (4 bytes)
} __attribute__((packed)) pdu_header_t;

/*!
 * Container for a full PDU (each UDP packet is made up of several PDUs)
 */
typedef struct
{
    pdu_header_t header;                        //!< 8-byte header (ID + length)
    char payload[LENGTH_OF_ASPM_LARGEST_Size];  //!< Values of each signal in the PDU (bitwise translation)
} pdu_packet_t;

/*!
 * \brief Constructs and interprets PDU packets passed between the TCM and the ASPM
 *
 * This class provides convenience functions for reading and writing valid SOME/IP PDUs
 * which are byte arrays used for UDP communication between the TCM and the ASPM.
 * Each UDP datagram has several PDUs sent in sequence, each with their own header and payload.
 *
 * \sa SignalHandler::encodeTCMSignalData
 * \sa SignalHandler::decodeASPMSignalData
 */
class SomePacket
{
    char* start;            //!< the first byte in the packet
    char* current;          //!< the byte currently being processed
    pdu_packet_t *someip;   //!< packet separated into header and payload
    char *payload;          //!< points to the start of the payload (after the header)
    UdpPacketType pkt_type; //!< type of packet being interpreted (for reads) or created (for writes)
    PDU_TYPE::ID pdu_type;  //!< type of PDU being processed

public :
    /*!
     * Constructs a PDU of a certain type from a byte array.
     * If it's used to read a packet, it will automatically set the header ID based on the data passed in.
     * If it's used to write a packet, use putHeaderID() to set the header ID.
     *
     * \param buf  pointer to start of byte array to use as PDU packet
     * \param what  type of PDU (read/write, to/from TCM/ASPM)
     * \sa putHeaderId
     */
    SomePacket(char* buf, UdpPacketType what) : start(buf), current(buf), someip(0), pkt_type(what), pdu_type(PDU_TYPE::PDU_NONE)
    {
        someip = (pdu_packet_t *)buf;
        payload = someip->payload;
        current += sizeof (pdu_header_t);
        //printf("SomePacket(%p, %p)", start, payload);

        if (pkt_type == UdpPacketType::ReadASPMPacket) {
            switch(getHeaderID()) {
                case HRD_ID_OF_ASPM_LM:
                    this->pdu_type = PDU_TYPE::PDU_ASPM_LM;
                    break;
                case HRD_ID_OF_ASPM_RemoteTarget:
                    this->pdu_type = PDU_TYPE::PDU_ASPM_RemoteTarget;
                    break;
                case HRD_ID_OF_ASPM_LM_Session:
                    this->pdu_type = PDU_TYPE::PDU_ASPM_LM_Session;
                    break;
                case HRD_ID_OF_ASPM_LM_ObjSegment:
                    this->pdu_type = PDU_TYPE::PDU_ASPM_LM_ObjSegment;
                    break;
                case HRD_ID_OF_ASPM_LM_Trunc:
                    this->pdu_type = PDU_TYPE::PDU_ASPM_LM_Trunc;
                    break;
                default :
                    break;
            }
        }
        else if (pkt_type == UdpPacketType::ReadTCMPacket) {
            switch(getHeaderID()) {
                case HRD_ID_OF_TCM_LM:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM;
                    break;
                case HRD_ID_OF_TCM_LM_App:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM_App;
                    break;
                case HRD_ID_OF_TCM_LM_KeyAlpha:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyAlpha;
                    break;
                case HRD_ID_OF_TCM_LM_KeyBeta:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyBeta;
                    break;
                case HRD_ID_OF_TCM_LM_KeyGamma:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyGamma;
                    break;
                case HRD_ID_OF_TCM_LM_KeyID:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyID;
                    break;
                case HRD_ID_OF_TCM_LM_Session:
                    this->pdu_type = PDU_TYPE::PDU_TCM_LM_Session;
                    break;
                case HRD_ID_OF_TCM_RemoteControl:
                    this->pdu_type = PDU_TYPE::PDU_TCM_RemoteControl;
                    break;
                case HRD_ID_OF_TCM_TransportKey:
                    this->pdu_type = PDU_TYPE::PDU_TCM_TransportKey;
                    break;
            }
        }
    }
    virtual ~SomePacket() {};

    /*!
     * Sets the header ID and length of the PDU, used for write packets only.
     *
     * \param headerID  value to set as PDU header ID
     */
    void putHeaderID(const uint32_t headerID)
    {
        if (someip) {
            someip->header.header_id = htonl(headerID);

            if (pkt_type == UdpPacketType::SendTCMPacket) {
                switch(headerID) {
                    case HRD_ID_OF_TCM_LM:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM;
                        this->putPayloadLength(LENGTH_OF_TCM_LM);
                        break;
                    case HRD_ID_OF_TCM_RemoteControl:
                        this->pdu_type = PDU_TYPE::PDU_TCM_RemoteControl;
                        this->putPayloadLength(LENGTH_OF_TCM_RemoteControl);
                        break;
                    case HRD_ID_OF_TCM_LM_Session:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM_Session;
                        this->putPayloadLength(LENGTH_OF_TCM_LM_Session);
                        break;
                    case HRD_ID_OF_TCM_TransportKey:
                        this->pdu_type = PDU_TYPE::PDU_TCM_TransportKey;
                        this->putPayloadLength(LENGTH_OF_TCM_TransportKey);
                        break;
                    case HRD_ID_OF_TCM_LM_App:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM_App;
                        this->putPayloadLength(LENGTH_OF_TCM_LM_App);
                        break;
                    case HRD_ID_OF_TCM_LM_KeyID:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyID;
                        this->putPayloadLength(LENGTH_OF_TCM_LM_KeyID);
                        break;
                    case HRD_ID_OF_TCM_LM_KeyAlpha:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyAlpha;
                        this->putPayloadLength(LENGTH_OF_TCM_LM_KeyAlpha);
                        break;
                    case HRD_ID_OF_TCM_LM_KeyBeta:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyBeta;
                        this->putPayloadLength(LENGTH_OF_TCM_LM_KeyBeta);
                        break;
                    case HRD_ID_OF_TCM_LM_KeyGamma:
                        this->pdu_type = PDU_TYPE::PDU_TCM_LM_KeyGamma;
                        this->putPayloadLength(LENGTH_OF_TCM_LM_KeyGamma);
                        break;
                    default :
                        printf("Invalid header information.");
                        break;
                }
            }
            else if (pkt_type == UdpPacketType::SendASPMPacket) {
                switch(headerID) {
                    case HRD_ID_OF_ASPM_LM:
                        this->pdu_type = PDU_TYPE::PDU_ASPM_LM;
                        this->putPayloadLength(LENGTH_OF_ASPM_LM);
                        break;
                    case HRD_ID_OF_ASPM_LM_ObjSegment:
                        this->pdu_type = PDU_TYPE::PDU_ASPM_LM_ObjSegment;
                        this->putPayloadLength(LENGTH_OF_ASPM_LM_ObjSegment);
                        break;
                    case HRD_ID_OF_ASPM_LM_Session:
                        this->pdu_type = PDU_TYPE::PDU_ASPM_LM_Session;
                        this->putPayloadLength(LENGTH_OF_ASPM_LM_Session);
                        break;
                    case HRD_ID_OF_ASPM_LM_Trunc:
                        this->pdu_type = PDU_TYPE::PDU_ASPM_LM_Trunc;
                        this->putPayloadLength(LENGTH_OF_ASPM_LM_Trunc);
                        break;
                    case HRD_ID_OF_ASPM_RemoteTarget:
                        this->pdu_type = PDU_TYPE::PDU_ASPM_RemoteTarget;
                        this->putPayloadLength(LENGTH_OF_ASPM_RemoteTarget);
                        break;
                }
            }
            //RCD_LOGD("==================> SEND PACKET: %d", this->pdu_type);
        } else {
            printf ("******putHeaderID: someip is null, err******");
        }
    }

    /*!
     * Sets length of PDU's payload.
     * No need to call this directly as it's called automatically in putHeaderID.
     *
     * \param payload_len  length of payload
     * \sa putHeaderId
     */
    void putPayloadLength(const uint32_t payload_len) //4bytes
    {
        if (someip) {
            someip->header.length = htonl(payload_len);
        } else {
            printf("putPayloadLength: someip is null, err");
        }
    }
    ///////////////////////////////////////////////////
    /*!
     * Accessor for packet's header ID
     *
     * \returns header ID of PDU
     */
    uint32_t getHeaderID(void)
    {
        if (someip) {
            return ntohl(someip->header.header_id);
        } else {
            printf("getHeaderID: someip is null, err");
        }
        return 0;
    }
    /*!
     * Accessor for length of packet's payload
     *
     * \returns length of PDU payload (in bytes)
     */
    uint32_t getPayloadLength(void)
    {
        return someip ? ntohl(someip->header.length) : 0;
    }
    /*!
     * Accessor for start of packet's payload
     *
     * \returns pointer to beginning of PDU's payload
     */
    char *const getPayloadStartAddress(void)
    {
        return payload;
    }
    ///////////////////////////////////////////////////
};
