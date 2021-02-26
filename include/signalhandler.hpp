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
 * \file Header for \p SignalHandler class.
 *
 * \author fdaniel, trice2
 */

#if !defined( SIGNALHANDLER_HPP )
#define SIGNALHANDLER_HPP

#include "constants.h"
#include "sockethandler.hpp" // TO use ASPM_PORT
#include "udppacket.hpp"

#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <bitset>
#include <iomanip>
#include <unistd.h>
#include <sys/time.h>
#include <functional>

#define UDP_BUF_MAX    512

/*!
 * Base container for signals used in TCM <-> ASPM communications
 */
typedef struct
{
    int16_t index;          //!< enum value of signal (per PDU)
    std::string name;       //!< name of signal
    uint64_t value;         //!< current value of signal
    uint64_t default_value; //!< default value of signal
    uint8_t bit_length;     //!< number of bits required for this signal in UDP payload
} lm_signal_t;

/*!
 * \brief Container for signals used in TCM <-> ASPM communications
 *
 * Holds all information for a specific signal including the name, index, value,
 * default value, and bit length.  Also provides convenience functions for comparison,
 * printing, etc.
 *
 * \sa lm_signal_t
 *
 */
class LMSignalInfo
{
    public:
        LMSignalInfo(lm_signal_t &signal) : lm_signal(signal) {}
        bool operator==(const lm_signal_t &other) const
        {
            return this->lm_signal.name == other.name;
        }
        bool operator==(const std::string &other) const
        {
            return this->lm_signal.name == other;
        }

        virtual ~LMSignalInfo() {}


        int16_t getIndex() { return lm_signal.index; }
        void setName(std::string s) { lm_signal.name = s; }
        std::string getName() { return lm_signal.name; }
        void setValue(uint64_t s) { lm_signal.value = s; }
        uint64_t getValue() { return lm_signal.value; }
        void setDefaultValue(uint64_t s) { lm_signal.default_value = s; }
        uint64_t getDefaultValue() { return lm_signal.default_value; }
        uint8_t getBitLength() { return lm_signal.bit_length; }

        void printOutSignal(const char* title) {
            printf("%s<%d> %s (value: %lu, bit_length: %d)", title, lm_signal.index, lm_signal.name.c_str(), lm_signal.value, lm_signal.bit_length);
        }

    private:
        lm_signal_t lm_signal;
};

/*!
 * \brief Handles all TCM <--> ASP signal values.
 *
 * Handles all communication to/from vehicle ASPM and stores all TCM <--> ASP
 * signal values for referencing by \p RemoteDeviceHandler.  Also, carries out
 * several VDC specific processes such as verifiying received pin and key fob
 * ranging.  These processes are in general referenced via signals in the DCM
 * class.
 *
 */
class SignalHandler
{

public:

    SignalHandler( );

    ~SignalHandler( );

    /*!
     * stop all running event loops and close all connections
     */
    virtual void stop( );

    /*!
     * public method for returning SignalHandler std::mutex for synchronicity
     */
    std::mutex& getMutex( );

    /*!
     * Initializes threaded event loops outside of constructor for safe override
     */
    virtual void initiateEventLoops( );

    /*!
     *  \brief Used to parse a MANOUEVRE code from a std::string
     *
     * \param maneuver  input maneuver in std::string format
     * \returns  output maneuver in MANOUEVRE format
     */
    static MANOUEVRE parseManeuverString( const std::string& maneuver );

    /*!
     * \brief Set appropriate TCM signals for a desired maneuver.
     *
     * \param   Orientation of desired maneuver
     */
    void setInputManeuverSignals( const MANOUEVRE& maneuver );

    /*!
     * \brief Sets the 'DeviceControlMode_RD' ASP signal to certain mode.
     * Assigns corresponding signals according to sequence diagram logic.
     *
     * \warning DeviceControlMode signal usage is deprecated.
     * \param mode TCM::DeviceControlMode  Desired signal value to tell to the ASP
     */
    virtual void setDeviceControlMode( const TCM::DeviceControlMode& mode );

    /*!
     * \brief Sets the 'ManeuverButtonPress_RD' ASP signal to certain mode.
     * Assigns corresponding signals according to sequence diagram logic.  Starts
     * time = 0 for button press timeout per sequence diagrams (missing req).
     *
     * \param mode TCM::ManeuverButtonPress  Desired signal value to tell to the ASP
     */
    virtual void setManeuverButtonPress( const TCM::ManeuverButtonPress& mode );

    /*!
     * \brief Sets the 'ManeuverEnableInput_RD' ASP signal to certain mode.
     * Assigns corresponding DMH signals IAW SSRD-ASPM-XXXX, SSRD-ASPM-XXXX,
     * SSRD-ASPM-XXXX, and sequence diagram logic:
     *    - x and y coordinates to ASPM as TCM::AppSliderPosX, TCM::AppSliderPosY
     *    - slider percent to ASPM as TCM::AppAccelerationZ
     *    - sets TCM::ManeuverEnableInput to a valid gesture
     *
     * \param xCoord  current x coordinate from RemoteDeviceHandler
     * \param yCoord  current y coordinate from RemoteDeviceHandler
     * \param gesturePercent  current gesture progress from RemoteDeviceHandler
     */
    void setManeuverEnableInput(
            const uint16_t& xCoord,
            const uint16_t& yCoord,
            const int64_t& gesturePercent,
            const bool& validGesture );

    /*!
     * \brief Sets the 'InControlRemotePin_RD' ASP signal to certain mode.
     * Sets PIN by parsing SHA256 hashes IAW EE-SWRD-RCP-XXXX.
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \param pin  Desired PIN value to be stored on the ASP
     */
    virtual void setInControlRemotePin( const std::string& pin );

    /*!
     * \brief Sets the 'ConnectionApproval_RD' ASP signal to certain mode.
     * Sets corresponding signals if proper PIN is passed IAW [REQ NAME HERE].
     * Advanced logic to determine device compatibility should be added at a
     * later time.
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \param mode TCM::ConnectionApproval  Desired signal value to tell to the ASP
     */
    virtual void setConnectionApproval( const TCM::ConnectionApproval& mode );

    /*!
     * \brief Set the rate for checking the key fob range
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \return void
     * \param rate DCM::FobRangeRequestRate  Desired ranging rate
     *
     */
    void setFobRangeRequestRate( const DCM::FobRangeRequestRate& rate );

    /*!
     * Sets engine and locks states.  TODO: actually send request(s) to CCM instead
     * of updating temporary flag variables
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \param engine_off  true if desired engine state is off
     * \param doors_locked  true if desired door lock state is off
     */
    void setCabinCommands( bool engine_off, bool doors_locked );

    /*!
     * Accessor for temporary engine state variable
     * TODO: obsolete once CCM communication is implemented
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \returns true if engine is currently off
     */
    bool getEngineOff( ) const;

    /*!
     * Accessor for temporary engine state variable
     * TODO: obsolete once CCM communication is implemented
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \returns true if doors are currently locked
     */
    bool getDoorsLocked( ) const;

    /*!
     * \brief Method for converting ASP signal combinations to JSON-ready
     * maneuvers
     *
     * \return std::string  corresponding maneuver value
     *
     * To return a maneuver to the remote device, a combination of ASP signals
     * need to be leveraged.  This methods parses the various combinations and
     * returns a string value based on the results.
     *
     * \note: RtnToOgn maneuver is not currently supported
     *
     * "StrFwd", or "Straight Forward", will be returned if:
     *    - ASP::ActiveParkingType = 4 (PushPull) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront)
     *
     * "StrRvs", or "Straight Reverse", will be returned if:
     *    - ASP::ActiveParkingType = 4 (PushPull) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear)
     *
     * "InLftFwd" or "In Left Forward", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront) AND
     *    - ASP::ActiveManeuverSide = 1 (Left)
     *
     * "InRgtFwd" or "In Right Forward", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront) AND
     *    - ASP::ActiveManeuverSide = 2 (Right)
     *
     * "InLftRvs" or "In Left Reverse", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ActiveManeuverSide = 1 (Left)
     *
     * "InRgtRvs" or "In Right Reverse", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ActiveManeuverSide = 2 (Right)
     *
     * "OutLftFwd" or "Out Left Forward", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront) AND
     *    - ASP::ActiveManeuverSide = 1 (Left)
     *
     * "OutRgtFwd" or "Out Right Forward", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront) AND
     *    - ASP::ActiveManeuverSide = 2 (Right)
     *
     * "OutLftRvs" or "Out Left Reverse", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ActiveManeuverSide = 1 (Left)
     *
     * "OutRgtRvs" or "Out Right Reverse", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ActiveManeuverSide = 2 (Right)
     *
     * "OutLftPrl" or "Out Left Parallel", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 1 (Parallel) AND
     *    - ASP::ActiveManeuverSide = 1 (Left)
     *
     * "OutRgtPrl" or "Out Right Parallel", will be returned if:
     *    - ASP::ActiveParkingType = 3 (Remote) AND
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 1 (Parallel) AND
     *    - ASP::ActiveManeuverSide = 2 (Right)
     *
     * "NdgFwd", or "Nudge Forward", will be returned if:
     *    - ASP::ActiveParkingType = 5 (LongitudinalAssist) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront)
     *
     * "NdgRvs", or "Nudge Reverse", will be returned if:
     *    - ASP::ActiveParkingType = 5 (LongitudinalAssist) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear)
    */
    std::string getManeuverFromASP( );

    /*!
     * Method for returning specific node value for ASPMFrontSegDistxxRMT
     *
     * \param sensorID  Sensor ID for desired node [0-15]
     */
    uint8_t getASPMFrontSegDistxxRMT( int& sensorID );

    /*!
     * Method for returning specific node value for ASPMRearSegDistxxRMT
     *
     * \param sensorID  Sensor ID for desired node [0-15]
     */
    uint8_t getASPMRearSegDistxxRMT( int& sensorID );

    /*!
     * Method for returning specific node value for ASPMFrontSegTypexxRMT
     *
     * \param sensorID  Sensor ID for desired node [0-15]
     */
    uint8_t getASPMFrontSegTypexxRMT( int& sensorID );

    /*!
     * Method for returning specific node value for ASPMRearSegTypexxRMT
     *
     * \param sensorID  Sensor ID for desired node [0-15]
     */
    uint8_t getASPMRearSegTypexxRMT( int& sensorID );

    /*!
     * checks whether a countdown has timed out and is no longer valid
     *
     * \param tv  time to be checked
     * \param maxVal  upper limit to be compared against
     * \return bool  boolean value for whether countdown is valid
     */
    bool checkTimeout( timeval& tv, const unsigned int& maxVal );

    /*!
     * Helper function for returning time difference between two timeval.
     *
     * \param t1  first value for time to be compared
     * \param t2  second value for time to be compared
     * \return unsigned int  time difference
     */
    unsigned int diffTimeval( timeval& t1, timeval& t2 );

    /*!
     * Helper function for checking whether a timeval is zero and thus reset.
     *
     * \param tv  timeval check if zero
     * \return bool  true if zero
     */
    bool isTimevalZero( timeval& tv );

    // Used for sending message from TCM to ASP.
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm; //!< Ordered signals in TCM_LM PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_remotecontrol; //!< Ordered signals in TCM_RemoteControl PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm_session; //!< Ordered signals in TCM_LM_Session PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_transportkey; //!< Ordered signals in TCM_TransportKey PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm_app; //!< Ordered signals in TCM_LM_App PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm_keyid; //!< Ordered signals in TCM_LM_KeyID PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm_keyalpha; //!< Ordered signals in TCM_LM_KeyAlpha PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm_keybeta; //!< Ordered signals in TCM_LM_KeyBeta PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_TCM_lm_keygamma; //!< Ordered signals in TCM_KeyGamma PDU
    /*!
     * Packs all the TCM signals into a UDP buffer to be sent to the ASP
     *
     * \param[out] buffer the UDP datagram will be written to this byte array
     * \returns size of buffer to be sent
     */
    uint16_t encodeTCMSignalData(uint8_t* buffer);

    // Used for receiving message from ASP to TCM.
    std::vector<std::shared_ptr<LMSignalInfo>> vt_ASPM_lm; //!< Ordered signals in ASPM_LM PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_ASPM_lm_objsegment; //!< Ordered signals in ASPM_LM_ObjSegment PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_ASPM_remotetarget; //!< Ordered signals in ASPM_RemoteTarget PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_ASPM_lm_session; //!< Ordered signals in ASPM_LM_Session PDU
    std::vector<std::shared_ptr<LMSignalInfo>> vt_ASPM_lm_trunc; //!< Ordered signals in ASPM_LM_Trunc PDU
    /*!
     * Decodes ASP signals from incoming UDP message and sets member variables accordingly
     *
     * \param[in] buffer the UDP datagram to decode
     */
    void decodeASPMSignalData(uint8_t* buffer);

    /*!
     * Fetches TCM signal based on PDU header ID and signal ID
     * Note: Not all TCM signals have been implemented in this class, but they still need to be a part of the datagram,
     * therefore, if a signal is not implemented, it will return a constant placeholder value.
     *
     * \param header_id ID of signal's PDU group
     * \param sigid ID of signal (defined in constants.h)
     * \returns value of signal
     */
    uint64_t getTCMSignal( uint8_t header_id, const uint16_t& sigid );
    /*!
     * Sets a specific ASP signal to a value
     * Note: This will sometimes be a no-op if you attempt to set a signal that hasn't been implemented
     *
     * \param header_id ID of signal's PDU group
     * \param sigId ID of signal (defined in constants.h)
     * \param value value to set ASP signal to
     */
    void setASPSignal(uint8_t header_id, const int16_t& sigid, uint64_t value );

    /*!
     * Fetches all signals for a specific TCM PDU
     *
     * \param header_id ID of signal's PDU group
     * \returns list of all signals for the PDU group with associated info
     */
    std::vector<std::shared_ptr<LMSignalInfo>>& get_TCM_vector (int header_id);
    /*!
     * Fetches all signals for a specific ASPM PDU
     *
     * \param header_id ID of signal's PDU group
     * \returns list of all signals for the PDU group with associated info
     */
    std::vector<std::shared_ptr<LMSignalInfo>>& get_ASP_vector (int header_id);

    /*!
     * mtx member for locking thread computation.
     */
    std::mutex mtx;

    /*!
     * indicates whether the vehicle has physically moved
     */
    std::atomic<bool> hasVehicleMoved;

    /*
    *** WRITE SIGNALS TO ASP ***
    */

    /*!
     * Holds the current signal value for reference; DCM to ASP
     */
    TCM::ConnectionApproval ConnectionApproval;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::DeviceControlMode DeviceControlMode;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ManeuverButtonPress ManeuverButtonPress;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ManeuverTypeSelect ManeuverTypeSelect;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ManeuverDirectionSelect ManeuverDirectionSelect;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ManeuverGearSelect ManeuverGearSelect;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ManeuverSideSelect ManeuverSideSelect;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ManeuverEnableInput ManeuverEnableInput;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::ExploreModeSelect ExploreModeSelect;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::NudgeSelect NudgeSelect;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     * X coordinate from mobile to be passed to DMH
     * PhysicalRange 0 - 2047 1 0
     */
    TCM::AppSliderPosX AppSliderPosX;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     * X coordinate from mobile to be passed to DMH
     * PhysicalRange 0 - 4095 1 0
     */
    TCM::AppSliderPosY AppSliderPosY;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::RemoteDeviceBatteryLevel RemoteDeviceBatteryLevel;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::AppCalcCheck AppCalcCheck;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::MobileChallengeReply MobileChallengeReply;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::AppAccelerationX AppAccelerationX;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::AppAccelerationY AppAccelerationY;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     */
    TCM::AppAccelerationZ AppAccelerationZ;

    /*
    *** READ SIGNALS FROM ASP ***
    */

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::InfoMsg InfoMsg;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ActiveAutonomousFeature ActiveAutonomousFeature;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ActiveParkingType ActiveParkingType;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ActiveParkingMode ActiveParkingMode;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ManeuverStatus ManeuverStatus;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::NoFeatureAvailableMsg NoFeatureAvailableMsg;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::CancelMsg CancelMsg;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::PauseMsg1 PauseMsg1;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::PauseMsg2 PauseMsg2;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::InstructMsg InstructMsg;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ExploreModeAvailability ExploreModeAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::RemoteDriveAvailability RemoteDriveAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ManeuverSideAvailability ManeuverSideAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::DirectionChangeAvailability DirectionChangeAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ActiveManeuverSide ActiveManeuverSide;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ActiveManeuverOrientation ActiveManeuverOrientation;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ParkTypeChangeAvailability ParkTypeChangeAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ManeuverDirectionAvailability ManeuverDirectionAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ManeuverAlignmentAvailability ManeuverAlignmentAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ConfirmAvailability ConfirmAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ResumeAvailability ResumeAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::ReturnToStartAvailability ReturnToStartAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::LongitudinalAdjustAvailability LongitudinalAdjustAvailability;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     * Longitudinal adjustment reamaining distance from target
     * PhysicalRange 0 - 1023 (mm)
     */
    ASP::LongitudinalAdjustLength LongitudinalAdjustLength;

    /*!
     * Indicates the progress of each manouevre in the form of a progress bar
     * for the remote device Holds the current ASP signal value for reference;
     * read from ASP
     * PhysicalRange 0 - 100 1 0 %
     * LogicalValue 101 Hide Progress Bar
     * PhysicalRange 102 - 127 1 0 Reserved
     */
    std::atomic<ASP::ManeuverProgressBar> ManeuverProgressBar;

    /*!
     * Vector of uint to store threat distance data, front; read from ASP
     */
    std::vector<uint8_t> ASPMFrontSegDistxxRMT;

    /*!
     * Vector of uint to store threat distance data, rear; read from ASP
     */
    std::vector<uint8_t> ASPMRearSegDistxxRMT;

    /*!
     * Vector of uint to store threat type data, front; read from ASP
     */
    std::vector<uint8_t> ASPMFrontSegTypexxRMT;

    /*!
     * Vector of uint to store threat type data, rear; read from ASP
     */
    std::vector<uint8_t> ASPMRearSegTypexxRMT;

    /*!
     * struct of surrounding threat data to render application splines.
     */
    struct ASPMxxSegDistxxRMT
    {
        uint8_t ASPMFrontSegDist1RMT;
        uint8_t ASPMFrontSegDist2RMT;
        uint8_t ASPMFrontSegDist3RMT;
        uint8_t ASPMFrontSegDist4RMT;
        uint8_t ASPMFrontSegDist5RMT;
        uint8_t ASPMFrontSegDist6RMT;
        uint8_t ASPMFrontSegDist7RMT;
        uint8_t ASPMFrontSegDist8RMT;
        uint8_t ASPMFrontSegDist9RMT;
        uint8_t ASPMFrontSegDist10RMT;
        uint8_t ASPMFrontSegDist11RMT;
        uint8_t ASPMFrontSegDist12RMT;
        uint8_t ASPMFrontSegDist13RMT;
        uint8_t ASPMFrontSegDist14RMT;
        uint8_t ASPMFrontSegDist15RMT;
        uint8_t ASPMFrontSegDist16RMT;

        uint8_t ASPMRearSegDist1RMT;
        uint8_t ASPMRearSegDist2RMT;
        uint8_t ASPMRearSegDist3RMT;
        uint8_t ASPMRearSegDist4RMT;
        uint8_t ASPMRearSegDist5RMT;
        uint8_t ASPMRearSegDist6RMT;
        uint8_t ASPMRearSegDist7RMT;
        uint8_t ASPMRearSegDist8RMT;
        uint8_t ASPMRearSegDist9RMT;
        uint8_t ASPMRearSegDist10RMT;
        uint8_t ASPMRearSegDist11RMT;
        uint8_t ASPMRearSegDist12RMT;
        uint8_t ASPMRearSegDist13RMT;
        uint8_t ASPMRearSegDist14RMT;
        uint8_t ASPMRearSegDist15RMT;
        uint8_t ASPMRearSegDist16RMT;
    } threatDistanceData;

    /*!
     * struct of surrounding threat data types to render application splines.
     */
    struct ASPMxxSegTypexxRMT
    {
        uint8_t ASPMFrontSegType1RMT;
        uint8_t ASPMFrontSegType2RMT;
        uint8_t ASPMFrontSegType3RMT;
        uint8_t ASPMFrontSegType4RMT;
        uint8_t ASPMFrontSegType5RMT;
        uint8_t ASPMFrontSegType6RMT;
        uint8_t ASPMFrontSegType7RMT;
        uint8_t ASPMFrontSegType8RMT;
        uint8_t ASPMFrontSegType9RMT;
        uint8_t ASPMFrontSegType10RMT;
        uint8_t ASPMFrontSegType11RMT;
        uint8_t ASPMFrontSegType12RMT;
        uint8_t ASPMFrontSegType13RMT;
        uint8_t ASPMFrontSegType14RMT;
        uint8_t ASPMFrontSegType15RMT;
        uint8_t ASPMFrontSegType16RMT;

        uint8_t ASPMRearSegType1RMT;
        uint8_t ASPMRearSegType2RMT;
        uint8_t ASPMRearSegType3RMT;
        uint8_t ASPMRearSegType4RMT;
        uint8_t ASPMRearSegType5RMT;
        uint8_t ASPMRearSegType6RMT;
        uint8_t ASPMRearSegType7RMT;
        uint8_t ASPMRearSegType8RMT;
        uint8_t ASPMRearSegType9RMT;
        uint8_t ASPMRearSegType10RMT;
        uint8_t ASPMRearSegType11RMT;
        uint8_t ASPMRearSegType12RMT;
        uint8_t ASPMRearSegType13RMT;
        uint8_t ASPMRearSegType14RMT;
        uint8_t ASPMRearSegType15RMT;
        uint8_t ASPMRearSegType16RMT;
    } threatTypeData;

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     */
    ASP::MobileChallengeSend MobileChallengeSend;

    /*
    *** SIGNALS ORIGINATING TO/FROM DCM TO RD ***
    */

    /*!
     * Holds the current ASP signal value for reference; DCM to Mobile
     */
    DCM::AcknowledgeRemotePIN AcknowledgeRemotePIN;

    /*!
     * Holds the current ASP signal value for reference; DCM to Mobile
     */
    DCM::ErrorMsg ErrorMsg;


private:
    /*!
     * Sets the PIN from a value stored in VDC memory to evaluate incoming PIN
     * authorisation requests from remote device
     *
     * \warning
     * This method is constructed to be used in a bench test setup and may
     * require additional functionality once deployed to a vehicle VDC.
     *
     * \return void
     */
    virtual void getPinFromVDC_( );

    /*!
     * Loop for sending/receiving UDP packets and updating signals
     */
    virtual void updateSignalEventLoop_( );

    /*!
     * Loop for generating a range request according to variable request rate
     */
    void rangingRequestEventLoop_( );

    /*!
     * Loop for checking if ManeuverButtonPress has expired IAW BUTTON_TIMEOUT_RATE
     */
    void buttonPressEventLoop_( );

    /*
    *** Private Members ***
    */

    /*!
     * Holds the current ASP signal value for reference; DCM to Mobile
     */
    DCM::FobRangeRequestRate rangingRequestRate_;

    /*!
     * Holds the current integer value for remote PIN as a SHA256 string
     */
    std::string InControlRemotePIN_;

    /*!
     * Holds the current integer value for VDC PIN as a SHA256 string
     */
    std::string pinStoredInVDC_;

    /*!
     * Holds the starting time to countdown maneuver button reset
     */
    timeval maneuverButtonPressTime_;

    /*!
     * Holds the time limit to countdown allowable PIN entries again
     */
    timeval pinEntryLockoutTime_;

    /*!
     * Holds the counter for current incorrect PIN entries
     */
    uint8_t pinIncorrectCount_;

    /*!
     * Holds the starting time to countdown allowable PIN entries again
     */
    unsigned int pinLockoutLimit_;

    /*!
     * SocketHandler object for handling all socket communications.
     */
    SocketHandler socketHandler_;

    /*!
     * loopHandler for running signal update loop on separate thread.
     */
    std::thread signalLoopHandler_;

    /*!
     * loopHandler for running signal update loop on separate thread.
     */
    std::thread rangingLoopHandler_;

    /*!
     * loopHandler for timing out the ManeuverButtonPress signal after 240ms.
     */
    std::thread buttonPressLoopHandler_;

    /*!
     * indicates that the event loops are currently "spinning"
     */
    std::atomic<bool> running_;

    /*!
     * temporary state variable to indicate engine state; TODO: obsolete this once CCM
     * communication is implemented
     */
    bool engine_off_;

    /*!
     * temporary state variable to indicate state of door locks; TODO: obsolete this
     * once CCM communcation is implemented
     */
    bool doors_locked_;
};


#endif //SIGNALHANDLER_HPP
