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
 * \file Provides an object for simulating lower-level ASP signals; to be obsoleted.
 *
 * \author fdaniel
 */

 #if !defined( ASPM_HPP )
 #define ASPM_HPP

#include <constants.h>
#include <signalhandler.hpp>
#include <sockethandler.hpp>

#include <sys/time.h>
#include <map>

constexpr auto SIM_SCAN_TIME = 3000000;     // μs, variable

typedef enum
{
    BENCH,
    EV3
} SIMULATOR;


class ASPM : public SignalHandler
{

public:

    ASPM( SIMULATOR type = SIMULATOR::BENCH, MANOUEVRE maneuver = MANOUEVRE::NADA );

    ~ASPM( );

    /*!
     * Initializes threaded event loops outside of constructor for safe override
     */
    virtual void initiateEventLoops( );

    /*!
     * stop all running event loops and close all connections
     */
    virtual void stop( );

    /*!
     *  Used for external handlers to change preferred simulator maneuvers.
     */
    void changePreferredSimMove( const std::string& maneuver );

    /*!
     *  Used for external handlers to change preferred simulator maneuvers.
     */
    void changePreferredSimMove( const MANOUEVRE& maneuver );

    /*!
     * Sets the 'DeviceControlMode_RD' ASP signal to certain mode, assigns
     * corresponding signals according to sequence diagram logic.  For PoC
     * purposes, this signal operates as the main mode set signal.
     *
     * <b>NOTE</b>: Use of deprecated signal allowable; unused by ASPM in prd.
     *
     * \param TCM::DeviceControlMode  Desired signal value to tell to the ASP
     */
    virtual void setDeviceControlMode( const TCM::DeviceControlMode& mode );

    /*!
     * Sets the 'ManeuverButtonPress_RD' ASP signal to certain mode, assigns
     * corresponding signals according to sequence diagram logic.
     *
     * \param TCM::ManeuverButtonPress  Desired signal value to tell to the ASP
     */
    virtual void setManeuverButtonPress( const TCM::ManeuverButtonPress& mode );

    /*!
     * Sets the 'ManeuverProgressBar' ASP signal to certain percent complete.
     *
     * \param uint8_t  Desired percent value to set ManeuverProgressBar to
     */
    void setManeuverProgressBar( const uint8_t& percentComplete );

    /*!
     * Sets the 'ConnectionApproval_RD' ASP signal to certain mode, assigns
     * corresponding signals if proper PIN is passed. Advanced logic to
     * determine device compatibility should be added at a later time.
     *
     * \param TCM::ConnectionApproval  Desired signal value to tell to the ASP
     */
    virtual void setConnectionApproval( const TCM::ConnectionApproval& mode );

    /*!
     * Packs all the ASPM signals into a UDP buffer to be sent to the TCM
     *
     * \param[out] buffer the UDP datagram will be written to this byte array
     * \returns size of buffer to be sent
     */
    uint16_t encodeASPMSignalData(uint8_t* buffer);

    /*!
     * Decodes TCM signals from incoming UDP message and sets member variables accordingly
     *
     * \param[in] buffer the UDP datagram to decode
     */
    void decodeTCMSignalData(uint8_t* buffer);

    /*!
     * Fetches ASPM signal based on PDU header ID and signal ID
     * Note: Not all ASPM signals have been implemented in this class, but they still need to be a part of the datagram,
     * therefore, if a signal is not implemented, it will return a constant placeholder value.
     *
     * \param header_id ID of signal's PDU group
     * \param sigid ID of signal (defined in constants.h)
     * \returns value of signal
     */
    uint64_t getASPMSignal( uint8_t header_id, const int32_t& sigid );

    /*!
     * Sets a specific TCM signal to a value
     * Note: This will sometimes be a no-op if you attempt to set a signal that hasn't been implemented
     *
     * \param header_id ID of signal's PDU group
     * \param sigId ID of signal (defined in constants.h)
     * \param value value to set TCM signal to
     */
    void setTCMSignal(uint8_t header_id, const int16_t& sigid, uint64_t value );

    /*!
     * return var for whether maneuver is underway and DMH is still valid
     */
    bool isManeuveringWithValidDMH;

    private:

    /*!
     * Simulates a proximity scan upon initialization and countsdown to complete
     *
     * \return bool  boolean value for whether proximity scan is complete
     * \param ASP::ManeuverStatus  check for status change since last check
     */
    bool areWeDoneScanning_( const ASP::ManeuverStatus& status );

    /*!
     * Simulates a countdown until the DMH is no longer valid
     *
     * \return bool  boolean value for whether dmh is valid
     * \param ASP::ManeuverStatus  check for status change since last check
     */
    bool areWeStillManeuvering_( const ASP::ManeuverStatus& status );

    /*!
     * Simulates a countdown until the DMH is no longer valid
     *
     * \return  boolean value for whether exploratory mode has been disabled.
     * \param MANOUEVRE  last exploratory maneuver carried out
     */
    bool isExploratoryModeDisabled_( const MANOUEVRE& maneuver );

    /*!
     * Generate a random value for ActiveManeuverSide.
     *
     * \return ASP::ActiveManeuverSide  randomly-generated signal
     */
    ASP::ActiveManeuverSide generateActiveManeuverSide_( );

    /*!
     * Generate a random value for ActiveManeuverOrientation.
     *
     * \return ASP::ActiveManeuverOrientation  randomly-generated signal
     */
    ASP::ActiveManeuverOrientation generateActiveManeuverOrientation_( );

    /*!
     * Generate a random value for ParkTypeChangeAvailability.
     *
     * \return ASP::ParkTypeChangeAvailability  randomly-generated signal
     */
    ASP::ParkTypeChangeAvailability generateParkTypeChangeAvailability_( );

    /*!
     * Generate a random value for ManeuverSideAvailability.
     *
     * \return ASP::ManeuverSideAvailability  randomly-generated signal
     */
    ASP::ManeuverSideAvailability generateManeuverSideAvailability_( );

    /*!
     * Generate a random value for DirectionChangeAvailability.
     *
     * \return ASP::DirectionChangeAvailability  randomly-generated signal
     */
    ASP::DirectionChangeAvailability generateDirectionChangeAvailability_( );

    /*!
     * Generate a random value for ManeuverAlignmentAvailability.
     *
     * \return ASP::ManeuverAlignmentAvailability  randomly-generated signal
     */
    ASP::ManeuverAlignmentAvailability generateManeuverAlignmentAvailability_( );

    /*!
     * Generate a random value for MobileChallengeSend.
     */
    ASP::MobileChallengeSend generateMobileChallengeSend_( );

    /*!
     * checks whether response matches a pending challenge
     * if there's a mismatch, it will set the appropriate PauseMsg2 status
     * TODO: timeout logic for if the ASPM doesn't receive a response to a challenge in time
     * TODO: tolerate a number of failures before setting pause state based on API
     *
     * \param response current received value for mobile challenge
     */
    void checkResponse_(TCM::MobileChallengeReply& response);

    /*!
     * Function to calculate the response for given challenge.
     * It expects three uint8_t parts and one index as parameter.
     *
     * The equation to calculate the response is:
     * Response = ((((Part1 * Part2) >> (Part3 && 0x0F)) + ((Part3 ^ Part1) << ((Part2 � Part1) && 0x07)))) / (0x03)).

     * Function will return the last three bytes of response and index as MSB
     */
    uint32_t calculateResponse_(uint8_t part1, uint8_t part2, uint8_t part3, uint8_t idx);

    /*!
    * Loop for listening for new UDP packets (TCM->ASP)
     */
    virtual void updateSignalEventLoop_( );

    /*!
    * Loop for updating maneuver progress bar to console
     */
    void progressBarEventLoop_( );

    /*!
     * Loop for updating ASP signals according to ASP_REFRESH_RATE
     */
    void updateSignalsAfterMsgFromTCM_( int& msgCount );

    /*!
     * Helper function for setting \p ParkIn signals.
     */
    // void updateSignalsAfterMsgFromTCM_( int& msgCount );

    /*!
     * check for ASPM class to iterate progress bar or allow it handled externally
     */
    bool isProgressHandledExternally_;

    /*!
     * Holds the starting time to countdown simulated scan
     */
    timeval scanStartTime_;

    /*!
     * Holds the starting time to countdown simulated dmh
     */
    timeval dmhStartTime_;

    /*!
     * SocketHandler object for handling all socket communications.
     */
    SocketHandler socketHandler_;

    /*!
     * loopHandler for ASP simulator thread.
     */
    std::thread signalLoopHandler_;

    /*!
     * loopHandler for progress bar thread.
     */
    std::thread progressBarLoopHandler_;

    /*!
     * True if initiateEventLoops() has been called
     */
    bool initialized_;

    /*!
     * indicates that the event loops are currently "spinning"
     */
    std::atomic<bool> running_;

    /*!
     * stores challenges that haven't been responded to yet by index
     */
    std::map<uint8_t, uint64_t> pending_challenges_;

    /*!
     * counter used to increment index in remote challenges
     */
    uint8_t challenge_idx_;

    /*!
     * indicates that the event loops are currently "spinning"
     */
    std::atomic<bool> challenge_warning_;

    /*!
     * counter used to increment subsequent exploratory maneuvers
     */
    uint8_t explore_idx_;

    /*!
     * indicates whether a subsequent explore maneuver can be carried out
     */
    std::atomic<bool> explore_limit_reached_;

    /*!
     * store the last exploratory maneuver carried out
     */
    MANOUEVRE lastExploreManeuver_;

    /*!
     * store the type of simulator that was instantiated
     */
    SIMULATOR simType_;

    /*!
     * store the preferred maneuver for testing that was instantiated
     */
    MANOUEVRE preferredSimManeuver_;

    /*!
     * store the preferred maneuver for testing that was instantiated
     */
    MANOUEVRE oppositeSimManeuver_;
};


#endif //ASPM_HPP
