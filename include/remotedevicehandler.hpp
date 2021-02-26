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
 * \file Header for \p RemoteDeviceHandler class.
 *
 * \author fdaniel, trice2
 */

#if !defined( REMOTEDEVICEHANDLER_HPP )
#define REMOTEDEVICEHANDLER_HPP

#include "signalhandler.hpp"
#include "sockethandler.hpp"
#include "templatehandler.hpp"

#include <sstream>
#include <iostream>
#include <fstream>
#include <atomic>


/*!
 * \brief Handles all communication in from a remote device.
 *
 * Handles all communication in from a remote device, parses JSON, informs
 * \p SignalHandler of user intent, and reports back any changes to remote
 * device.
 *
 */
class RemoteDeviceHandler
{

public:

    /*!
     * constructor
     */
    RemoteDeviceHandler( std::shared_ptr <SignalHandler> TCM );

    /*!
     * destructor
     */
    ~RemoteDeviceHandler( );

    /*!
     * main blocking call for waiting for a client to connect, then waiting for
     * a received message from a connected client.
     */
    void spin( );

    /*!
     * used for graceful deconstruction
     */
    void stop( );


private:

    /*!
     * Check if received message can be parsed into appropriate JSON.
     *
     * \return  boolean value for whether message is compatible
     * \param receiptVal  received value from incoming message
     */
    bool checkMessageReadability_( const ssize_t& receiptVal );

    /*!
     * Check if received message contains a client request for disconnecting.
     *
     * \return  boolean value for whether client is connected
     * \param  msg message for parsing in string format
     */
    bool checkClientConnection_( const std::string& msg );

    /*!
     * Set compatible device connection status for method owned by
     * \p SignalHandler.
     *
     * \param mode TCM::ConnectionApproval connection approval status
     */
    void setConnectionApproved_( const TCM::ConnectionApproval& mode );

    /*!
     * Set the rate for key fob ranging method owned by \p SignalHandler.
     *
     * \param rangingRate DCM::FobRangeRequestRate desired ranging rate
     */
    void setKeyFobRangingRate_( const DCM::FobRangeRequestRate& rangingRate );

    /*!
     * Main message processing pipeline; all possible JSON scenarios are here
     *
     * \param  msg message for parsing in string format
     * \param  headerLen message header length
     * \param  msgLen message body length
     *
     * \exception e
     * Exception thrown parsing JSON message; check input format for template
     * mismatch
     *
     * \sa setRemoteControlPIN_( )
     * \sa loadMainMenu_( )
     * \sa loadSpaceSelection_( )
     * \sa pushPullSelected_( )
     * \sa updateDMH_( )
     * \sa SignalHandler::setManeuverButtonPress( )
     * \sa SignalHandler::setCabinCommands( )
     *
     */
    void messageEvent_(
            const std::string& msg,
            const uint32_t& headerLen,
            const uint32_t& msgLen );

    /*!
     * Send JSON messages out via \p SocketHandler
     *
     * \param  msgOut json struct for sending
     * \param  msgGroup group name for populating outgoing header
     *
     * \sa TemplateHandler::getRawHeaderTemplate( )
     * \sa TemplateHandler::getRawVehicleStatusTemplate( )
     *
     */
    void sendMsg_( const nlohmann::json& msgOut, const std::string& msgGroup );

    /*!
     * \brief Format and send JSON message to return VEHICLE_API_VERSION
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/vehicle_api_version.json">
     * vehicle_api_version</a>
     *
     * Returns the telematics API version from the TCM to mobile device stored
     * in API_DOC_VERSION
     *
     */
    void sendVehicleAPIVersion_( );

    /*!
    * \brief Format and send JSON message to return VEHICLE_STATUS
    *
    * JSON message structure:
    * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/vehicle_status.json">
    * vehicle_status</a>
    *
    * Returns one of the following status messages according to the above
    * template and
    * <a href="./res/vehiclestatuscodes.md">\p vehiclestatuscodes.md</a>:
    *    - ASP::NoFeatureAvailableMsg
    *    - ASP::CancelMsg
    *    - ASP::PauseMsg1
    *    - ASP::PauseMsg2
    *    - ASP::InfoMsg
    *    - ASP::InstructMsg
    *    - DCM::AcknowledgeRemotePIN
    *    - DCM::PinNotSet
    *    - DCM::ErrorMsg
    *    - ASP::ManeuverStatus
    *
    * \sa TemplateHandler::getRawVehicleStatusTemplate( )
    *
    */
    void sendVehicleStatus_( );

    /*!
    * \brief Format and send JSON message to return VEHICLE_INIT
    *
    * JSON message structure:
    * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/vehicle_init.json">
    * vehicle_init</a>
    *
    * The vehicle_init is the response to the mobile_init call.  The "ready"
    * field evaluates true if:
    *    - TCM::ConnectionApproval = 2 (AllowedDevice) AND
    *    - DCM::AcknowledgeRemotePIN = 0 (CorrectPIN)
    *
    * The "active_maneuver" field indicates whether a maneuver is currently
    * underway and if so, sends the maneuver_status message alongside
    * vehicle_init via sendManeuverStatus_( ).  The "active_maneuver" field
    * evaluates true if:
    *    - ASP::ManeuverStatus = 3 (Confirming) OR
    *    - ASP::ManeuverStatus = 4 (Maneuvering) OR
    *    - ASP::ManeuverStatus = 5 (Interrupted) OR
    *    - ASP::ManeuverStatus = 6 (Finishing)
    *
    * \sa RemoteDeviceHandler::sendManeuverStatus_( )
    *
    */
    void sendVehicleInit_( );

    /*!
     * \brief Format and send JSON message to return THREAT_DATA
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/threat_data.json">
     * threat_data</a>
     *
     * For populating threat data, this method leverages the two
     * SignalHandler::ASPMFrontSegDistxxRMT and SignalHandler::ASPMRearSegDistxxRMT
     * arrays to fill the data fields.  Send back threat data based on the front
     * and rear integer vectors.  Since the pattern starts at the driver side door
     * and makes a continuous circle, the rear values must be inverted according
     * to the image below:
     *
     * <a href="./res/threat_data.png">Threat Data Nodes</a>
     *
     * SignalHandler::ASPMFrontSegDistxxRMT contains the following ASP signals:
     *    - ASPM::ASPMFrontSegDist1RMT
     *    - ASPM::ASPMFrontSegDist2RMT
     *    - ASPM::ASPMFrontSegDist3RMT
     *    - ASPM::ASPMFrontSegDist4RMT
     *    - ASPM::ASPMFrontSegDist5RMT
     *    - ASPM::ASPMFrontSegDist6RMT
     *    - ASPM::ASPMFrontSegDist7RMT
     *    - ASPM::ASPMFrontSegDist8RMT
     *    - ASPM::ASPMFrontSegDist9RMT
     *    - ASPM::ASPMFrontSegDist10RMT
     *    - ASPM::ASPMFrontSegDist11RMT
     *    - ASPM::ASPMFrontSegDist12RMT
     *    - ASPM::ASPMFrontSegDist13RMT
     *    - ASPM::ASPMFrontSegDist14RMT
     *    - ASPM::ASPMFrontSegDist15RMT
     *    - ASPM::ASPMFrontSegDist16RMT
     *
     * SignalHandler::ASPMRearSegDistxxRMT contains the following ASP signals:
     *    - ASPM::ASPMRearSegDist1RMT
     *    - ASPM::ASPMRearSegDist2RMT
     *    - ASPM::ASPMRearSegDist3RMT
     *    - ASPM::ASPMRearSegDist4RMT
     *    - ASPM::ASPMRearSegDist5RMT
     *    - ASPM::ASPMRearSegDist6RMT
     *    - ASPM::ASPMRearSegDist7RMT
     *    - ASPM::ASPMRearSegDist8RMT
     *    - ASPM::ASPMRearSegDist9RMT
     *    - ASPM::ASPMRearSegDist10RMT
     *    - ASPM::ASPMRearSegDist11RMT
     *    - ASPM::ASPMRearSegDist12RMT
     *    - ASPM::ASPMRearSegDist13RMT
     *    - ASPM::ASPMRearSegDist14RMT
     *    - ASPM::ASPMRearSegDist15RMT
     *    - ASPM::ASPMRearSegDist16RMT
     *
     * \sa SignalHandler::getASPMFrontSegDistxxRMT( )
     * \sa SignalHandler::getASPMRearSegDistxxRMT( )
     */
    void sendThreatData_( );

    /*!
     * \brief Format and send JSON message to return AVAILABLE_MANEUVERS
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/available_maneuvers.json">
     * available_maneuvers</a>
     *
     * <b>Push / Pull Availability</b>
     *
     * For Push/Pull maneuvers, the ASP::ExploreModeAvailability signal is
     * referenced to see whether Push/Pull is offered.  In the case that it is,
     * ASP::ManeuverDirectionAvailability references in which direction push/pull can be
     * carried out and populates the "SF" and/or "SR" fields true accordingly.
     *
     * "StrFwd", or "Straight Forward", will evaluate true if:
     *    - ASP::ExploreModeAvailability = 1 (Offer Enabled) AND
     *    - ASP::ManeuverDirectionAvailability = 1 (Offer Forward Only) OR
     *    ASP::ManeuverDirectionAvailability = 3 (Offer Forward and Backwards)
     *
     * "StrRvs", or "Straight Reverse" will evaluate true if:
     *    - ASP::ExploreModeAvailability = 1 (Offer Enabled) AND
     *    - ASP::ManeuverDirectionAvailability = 2 (Offer Backwards Only) OR
     *    ASP::ManeuverDirectionAvailability = 3 (Offer Forward and Backwards)
     *
     * <b>Nudge Availability</b>
     *
     *  For populating whether nudge maneuvers are available, first, the
     *  ASP::LongitudinalAdjustAvailability signal is referenced to see whether nudge is
     *  offered.  In the case that it is, ASP::ManeuverDirectionAvailability is referenced
     *  to check in which direction nudging can be carried out.
     *
     * "NdgFwd", or "Nudge Forward", will evaluate true if:
     *    - ASP::LongitudinalAdjustAvailability = 1 (Offer Enabled) AND
     *    - ASP::ManeuverDirectionAvailability = 1 (Offer Forward Only) OR
     *    ASP::ManeuverDirectionAvailability = 3 (Offer Forward and Backwards)
     *
     * "NdgRvs", or "Nudge Reverse" will evaluate true if:
     *    - ASP::LongitudinalAdjustAvailability = 1 (Offer Enabled) AND
     *    - ASP::ManeuverDirectionAvailability = 2 (Offer Backwards Only) OR
     *    ASP::ManeuverDirectionAvailability = 3 (Offer Forward and Backwards)
     *
     * <b>Return to Start Availability</b>
     *
     *  The logic below is for populating the return to start (RTS) maneuver.
     *  This leverages the ASP::ReturnToStartAvailability signal and returns true if
     *  enabled.
     *
     * "RtnToOgn", or "Return to Origin", will evaluate true if:
     *    - ASP::ReturnToStartAvailability = 1 (Offer Enabled)
     *
     * <b>Park In / Out Availability</b>
     *
     *  For populating whether park in / out maneuvers are available, first, the
     *  ASP::ActiveParkingMode signal is referenced for a ParkIn / ParkOut status.
     *  In the case that it is, ASP::ActiveManeuverOrientation is referenced to check
     *  for Parallel, PerpendicularRear, or PerpendicularFront.  Last,
     *  ASP::ManeuverSideAvailability is referenced to check left, right, or both.
     *
     * "InLftRvs", or "Park In, Left, Reverse", will evaluate true if:
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ManeuverSideAvailability = 1 (OfferLeftOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "InRgtRvs", or "Park In, Right, Reverse", will evaluate true if:
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ManeuverSideAvailability = 2 (OfferRightOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "InLftFwd", or "Park In, Left, Forward", will evaluate true if:
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront) AND
     *    - ASP::ManeuverSideAvailability = 1 (OfferLeftOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "InRgtFwd", or "Park In, Right, Forward", will evaluate true if:
     *    - ASP::ActiveParkingMode = 1 (ParkIn) AND
     *    - ASP::ActiveManeuverOrientation = 3 (PerpendicularFront) AND
     *    - ASP::ManeuverSideAvailability = 2 (OfferRightOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "OutLftRvs", or "Park Out, Left, Reverse", will evaluate true if:
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ManeuverSideAvailability = 1 (OfferLeftOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "OutRgtRvs", or "Park Out, Right, Reverse", will evaluate true if:
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularRear) AND
     *    - ASP::ManeuverSideAvailability = 2 (OfferRightOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "OutLftFwd", or "Park Out, Left, Forward", will evaluate true if:
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularFront) AND
     *    - ASP::ManeuverSideAvailability = 1 (OfferLeftOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "OutRgtFwd", or "Park Out, Right, Forward", will evaluate true if:
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 2 (PerpendicularFront) AND
     *    - ASP::ManeuverSideAvailability = 2 (OfferRightOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "OutLftPrl", or "Park Out, Left, Parallel", will evaluate true if:
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 1 (Parallel) AND
     *    - ASP::ManeuverSideAvailability = 1 (OfferLeftOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     * "OutRgtPrl", or "Park Out, Right, Parallel", will evaluate true if:
     *    - ASP::ActiveParkingMode = 2 (ParkOut) AND
     *    - ASP::ActiveManeuverOrientation = 1 (Parallel) AND
     *    - ASP::ManeuverSideAvailability = 2 (OfferRightOnly) OR
     *    ASP::ManeuverSideAvailability = 3 (OfferLeftAndRight)
     *
     */
    void sendAvailableManeuvers_( );

    /*!
     * \brief Format and send JSON message to return MOBILE_CHALLENGE
     *
     * This message is used for debug purposes and will be deprecated upon
     * production release as there is no return message for a received DMH.
     */
    void sendMobileChallenge_( );

    /*!
    * \brief Format and send JSON message to return MANEUVER_STATUS
    *
    * JSON message structure:
    * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/maneuver_status.json">
    * maneuver_status</a>
    *
    * The maneuver_status is sent at various times in the mobile app flow.  The
    * "maneuver" field is populated by polling the current maneuver underway as
    * reported by the combination signals in \p SignalHandler.  The "progress"
    * field is populated by the current value for ASP::ManeuverProgressBar.
    * The value for "status" is populated according to the following logic:
    *    - "status" = "confirmed" if ASP::ManeuverStatus = 3 (Confirming)
    *    - "status" = "initiated" if
    *        - ASP::ManeuverStatus = 4 (Maneuvering) OR
    *        - ASP::ManeuverStatus = 5 (Interrupted) OR
    *        - ASP::ManeuverStatus = 6 (Finishing) OR
    *        - ASP::ManeuverStatus = 9 (Holding)
    *    - "status" = "cancelled" if ASP::ManeuverStatus = 10 (Cancelled)
    *    - "status" = "default" if ASP::ManeuverStatus contains any other value.
    *
    * \sa SignalHandler::getManeuverFromASP( )
    * \sa SignalHandler::ManeuverProgressBar( )
    *
    */
    void sendManeuverStatus_( );

    /*!
     * \brief Format and send JSON message to return CABIN_STATUS
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/read/cabin_status.json">
     * cabin_status</a>
     *
     * \sa SignalHandler::getEngineOff( )
     * \sa SignalHandler::getDoorsLocked( )
     *
     */
    void sendCabinStatus_( );

    /*!
     * Method for setting DeviceControlMode::RCStartStop
     */
    void toggleEngineOnOff_( );

    /*!
     * Method for setting DeviceControlMode::RCMainMenu
     */
    void loadMainMenu_( );

    /*!
     * Method for setting DeviceControlMode::RCParkOutSpaceSlctn
     */
    void loadSpaceSelection_( );

    /*!
     * \brief Method for setting DeviceControlMode::RCPushPull IAW [REQ NAME HERE].
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/write/maneuver_init.json">
     * maneuver_init</a>
     *
     * \param direction  selected direction for push/pull
     *
     * For initialising Push/Pull maneuvers, the JSON maneuver read in will read
     * either "StrFwd" or "StrRvs" for forward or reverse, respectively.  The
     * following associated ASP signals are then set in the TCM object
     * accordingly.
     *
     * "StrFwd", or "Straight Forward", will set the following signals:
     *    - TCM::ExploreModeSelect = 1 (Pressed)
     *    - TCM::ManeuverTypeSelect = 0 (None)
     *    - TCM::ManeuverDirectionSelect = 0 (None)
     *    - TCM::ManeuverGearSelect = 1 (Forward)
     *    - TCM::ManeuverSideSelect = 3 (Centre)
     *
     * "StrRvs", or "Straight Reverse", will set the following signals:
     *    - TCM::ExploreModeSelect = 1 (Pressed)
     *    - TCM::ManeuverTypeSelect = 0 (None)
     *    - TCM::ManeuverDirectionSelect = 0 (None)
     *    - TCM::ManeuverGearSelect = 2 (Reverse)
     *    - TCM::ManeuverSideSelect = 3 (Centre)
     *
     */
    void pushPullSelected_( const std::string& direction );

    /*!
     * \brief Method for setting DeviceControlMode::RCParkIn.
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/write/maneuver_init.json">
     * maneuver_init</a>
     *
     * \param direction  selected maneuver
     *
     * For initialising ParkIn maneuvers, the JSON maneuver read in will read
     * "InLftFwd", "InLftRvs", "InRgtFwd", or "InRgtRvs".  The
     * following associated ASP signals are then set in the TCM object
     * accordingly.
     *
     * "InLftFwd", or "In Left Forward", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 1 (NoseFirst)
     *    - TCM::ManeuverGearSelect = 1 (Forward)
     *    - TCM::ManeuverSideSelect = 1 (Left)
     *
     * "InLftRvs", or "In Left Reverse", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 2 (RearFirst)
     *    - TCM::ManeuverGearSelect = 2 (Reverse)
     *    - TCM::ManeuverSideSelect = 1 (Left)
     *
     * "InRgtFwd", or "In Right Forward", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 1 (NoseFirst)
     *    - TCM::ManeuverGearSelect = 1 (Forward)
     *    - TCM::ManeuverSideSelect = 2 (Right)
     *
     * "InRgtRvs", or "In Right Reverse", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 2 (RearFirst)
     *    - TCM::ManeuverGearSelect = 2 (Reverse)
     *    - TCM::ManeuverSideSelect = 2 (Right)
     */
    void parkInSelected_( const std::string& direction );

    /*!
     * \brief Method for setting DeviceControlMode::RCParkOut.
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/write/maneuver_init.json">
     * maneuver_init</a>
     *
     * \param direction  selected maneuver
     *
     * For initialising ParkOut maneuvers, the JSON maneuver read in will read
     * "OutLftFwd", "OutLftRvs", "OutRgtFwd", "OutRgtRvs", "OutLftPrl", or "OutRgtPrl".
     * The following associated ASP signals are then set in the TCM object
     * accordingly.
     *
     * "OutLftFwd", or "Out Left Forward", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 1 (NoseFirst)
     *    - TCM::ManeuverGearSelect = 1 (Forward)
     *    - TCM::ManeuverSideSelect = 1 (Left)
     *
     * "OutLftRvs", or "Out Left Reverse", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 2 (RearFirst)
     *    - TCM::ManeuverGearSelect = 2 (Reverse)
     *    - TCM::ManeuverSideSelect = 1 (Left)
     *
     * "OutRgtFwd", or "Out Right Forward", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 1 (NoseFirst)
     *    - TCM::ManeuverGearSelect = 1 (Forward)
     *    - TCM::ManeuverSideSelect = 2 (Right)
     *
     * "OutRgtRvs", or "Out Right Reverse", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Perpendicular)
     *    - TCM::ManeuverDirectionSelect = 2 (RearFirst)
     *    - TCM::ManeuverGearSelect = 2 (Reverse)
     *    - TCM::ManeuverSideSelect = 2 (Right)
     *
     * "OutLftPrl", or "Out Left Parallel", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 2 (Parallel)
     *    - TCM::ManeuverDirectionSelect = 0 (None)
     *    - TCM::ManeuverGearSelect = 0 (None)
     *    - TCM::ManeuverSideSelect = 1 (Left)
     *
     * "OutRgtPrl", or "Out Right Parallel", will set the following signals:
     *    - TCM::ManeuverTypeSelect = 1 (Parallel)
     *    - TCM::ManeuverDirectionSelect = 0 (None)
     *    - TCM::ManeuverGearSelect = 0 (None)
     *    - TCM::ManeuverSideSelect = 2 (Right)
     */
    void parkOutSelected_( const std::string& direction );

    /*!
     * \brief Method for setting DeviceControlMode::RCAdjust
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/write/maneuver_init.json">
     * maneuver_init</a>
     *
     * \param direction  selected direction for nudge
     *
     * For initialising nudge maneuvers, the JSON maneuver read in will read
     * either "NdgFwd" or "NdgRvs" for forward or reverse, respectively.  The
     * following associated ASP signals are then set in the TCM object
     * accordingly.
     *
     * "NdgFwd", or "Nudge Forward", will set the following signals:
     *    - TCM::NudgeSelect = 1 (Pressed)
     *    - TCM::ManeuverTypeSelect = 0 (None)
     *    - TCM::ManeuverDirectionSelect = 0 (None)
     *    - TCM::ManeuverGearSelect = 1 (Forward)
     *    - TCM::ManeuverSideSelect = 3 (Centre)
     *
     * "NdgRvs", or "Nudge Reverse", will set the following signals:
     *    - TCM::NudgeSelect = 1 (Pressed)
     *    - TCM::ManeuverTypeSelect = 0 (None)
     *    - TCM::ManeuverDirectionSelect = 0 (None)
     *    - TCM::ManeuverGearSelect = 2 (Reverse)
     *    - TCM::ManeuverSideSelect = 3 (Centre)
     *
     */
    void adjustSelected_( const std::string& direction );

    /*!
     * \brief Method for setting TCM::ManeuverButtonPress::ReturnToStart
     *
     * JSON message structure:
     * <a href="https://git.sdo.jlrmotor.com/Connected_Technologies_and_Apps/MobileApps/CoPilot/CoPilotTelematicsComms/blob/devel/doc/api/write/maneuver_init.json">
     * maneuver_init</a>
     *
     * For returning to the original origin point, the JSON maneuver read in
     * will read "RtnToOgn".  The following associated ASP signals are then set
     * in the TCM object accordingly.
     *
     * "RtnToOgn", or "Return to Origin", will set the following signals:
     *    - TCM::NudgeSelect = 1 (Pressed)
     *
     */
    void returnToOriginSelected_( );

    /*!
     * Update \p SignalHandler with current x, y coordinates of mobile device
     * DMH and passes CRC data IAW [REQ NAME HERE].
     *
     * \param x  current x coordinate
     * \param y  current y coordinate
     * \param percent  current value for DMH gesture percentage
     * \param gesture  current validity of received gesture
     * \param crc  current computed value for CRC value
     */
    void updateDMH_(
            const float& x,
            const float& y,
            const int& percent,
            const bool& gesture,
            const int& crc );

    /*!
     * Method for passing PIN to TCM for verification IAW [REQ NAME HERE].
     *
     * \param  pin value to be sent to TCM
     */
    void setRemoteControlPIN_( const std::string& pin );

    /*!
     * Check if PIN has been authenticated IAW [REQ NAME HERE].
     *
     * \return bool  boolean value for whether PIN is currently authenticated
     */
    bool checkAuthenticatedPIN_( );

    /*!
     * Check if device is compatible for RPA.
     *
     * \return bool  boolean value for whether device is compatible
     */
    bool checkDeviceCompatibility_( );

    /*!
     * Check if TCM reports vehicle is mid-maneuver.
     *
     * \return bool  boolean value for whether maneuver is underway
     */
    bool checkManeuversInProgress_( );

    /*!
     * Check if exploratory maneuvers can be continued ( < 3 SF or SR )
     *
     * \return bool  boolean value for whether explore can continue
     */
    bool checkContinueExploratoryMode_( );

    /*!
     * Helper method to check which of the vehicle_status signals has changed.
     *
     * \param bool  atomic boolean value for whether status vals have changed
     * \return bool  boolean value for whether status vals have changed
     */
    bool checkForNewStatusSignals_( std::atomic<bool>& statusChanged );

    /*!
     * Add prefix to status code IAW
     * <a href="./res/vehiclestatuscodes.md">\p vehiclestatuscodes.md</a>.
     *
     * \param prefix  type of status code to prefix
     * \return updated status code with prefix
     */
    int prefixStatus_( const VehicleStatusPrefix& prefix );

    /*!
     * Map message text values to names for use in \p RemoteDeviceHandler
     *
     * \exception e
     * Error populating signal text values
     *
     * \param sigText  map of signal text values
     * \param data  raw text from CONSTANTS_H
     */
    void mapMsgText_( std::vector<std::string>& sigText, const char* data );

    /*!
     * Event loop for tracking changes in ASP state and reporting back to mobile
     * IAW [REQ NAME HERE]
     */
    void statusUpdateEventLoop_( );

    /*!
     * \p SignalHandler object for get/set methods and storing signal values.
     */
    std::shared_ptr <SignalHandler> TCM_;

    /*!
     * holds JSON message templates
     *
     * \return TemplateHandler  a group of accessors for JSON templates
     */
    TemplateHandler templates_;

    /*!
     * \p SocketHandler object for handling all socket communications.
     */
    SocketHandler socketHandler_;

    /*!
     * struct of signal text mapped to signal code.
     */
    struct signalText
    {

        /*!
         * String vector for assigning ManeuverStatus signals to text values.
         */
        std::vector<std::string> ManeuverStatus;

        /*!
         * String vector for assigning NoFeatureAvailableMsg signals to text values.
         */
        std::vector<std::string> NoFeatureAvailableMsg;

        /*!
         * String vector for assigning CancelMsg signals to text values.
         */
        std::vector<std::string> CancelMsg;

        /*!
         * String vector for assigning PauseMsg1 signals to text values.
         */
        std::vector<std::string> PauseMsg1;

        /*!
         * String vector for assigning PauseMsg2 signals to text values.
         */
        std::vector<std::string> PauseMsg2;

        /*!
         * String vector for assigning InfoMsg signals to text values.
         */
        std::vector<std::string> InfoMsg;

        /*!
         * String vector for assigning InstructMsg signals to text values.
         */
        std::vector<std::string> InstructMsg;

        /*!
         * String vector for assigning InstructMsg signals to text values.
         */
        std::vector<std::string> AcknowledgeRemotePIN;

        /*!
         * String vector for assigning InstructMsg signals to text values.
         */
        std::vector<std::string> ErrorMsg;

    } sigText_;

    /*!
     * holds all previous signal values to be checked on loop.
     */
     struct previousSignal
     {
        /*!
         * Previous ManeuverStatus value for tracking changes that should be reported.
         */
        ASP::ManeuverStatus ManeuverStatus;

        /*!
         * Previous NoFeatureAvailableMsg value for tracking changes that should be reported.
         */
        ASP::NoFeatureAvailableMsg NoFeatureAvailableMsg;

        /*!
         * Previous InfoMsg value for tracking changes that should be reported.
         */
        ASP::CancelMsg CancelMsg;

        /*!
         * Previous InfoMsg value for tracking changes that should be reported.
         */
        ASP::PauseMsg1 PauseMsg1;

        /*!
         * Previous InfoMsg value for tracking changes that should be reported.
         */
        ASP::PauseMsg2 PauseMsg2;

        /*!
         * Previous InfoMsg value for tracking changes that should be reported.
         */
        ASP::InfoMsg InfoMsg;

        /*!
         * Previous InstructMsg value for tracking changes that should be reported.
         */
        ASP::InstructMsg InstructMsg;

        /*!
         * Previous AcknowledgeRemotePIN value for tracking changes that should be reported.
         */
        DCM::AcknowledgeRemotePIN AcknowledgeRemotePIN;

        /*!
         * Previous ErrorMsg value for tracking changes that should be reported.
         */
        DCM::ErrorMsg ErrorMsg;

        /*!
         * Previous MobileChallengeSend value for tracking changes that should be reported.
         */
        ASP::MobileChallengeSend MobileChallengeSend;

    } prevSig_;

    /*!
     * indicates that the handler is currently "spinning"
     */
    std::atomic<bool> running_;

    /*!
     * loopHandler for MsgParser thread.
     */
    std::thread eventLoopHandler_;

};

// external handlers for status signal text
extern const char * rawTextManeuverStatus;
extern const char * rawTextNoFeatureAvailableMsg;
extern const char * rawTextCancelMsg;
extern const char * rawTextPauseMsg1;
extern const char * rawTextPauseMsg2;
extern const char * rawTextLMInfoRMT;
extern const char * rawTextInstructMsg;
extern const char * rawAcknowledgeRemotePIN;
extern const char * rawErrorMsg;

#endif //REMOTEDEVICEHANDLER_HPP
