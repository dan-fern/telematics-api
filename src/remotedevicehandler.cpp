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
 * \file Class definition for \p RemoteDeviceHandler.
 *
 * \author fdaniel, trice2
 */

#include "remotedevicehandler.hpp"

using json = nlohmann::json;


RemoteDeviceHandler::RemoteDeviceHandler( std::shared_ptr <SignalHandler> TCM )
        :
        TCM_( TCM ),
        templates_( ),
        running_( true ),
        eventLoopHandler_( &RemoteDeviceHandler::statusUpdateEventLoop_, this )
{

    // Generate client sockets
    SocketHandler socketHandler_;

    // Populate assigned signal values.
    prevSig_.ManeuverStatus = TCM_->ManeuverStatus;
    prevSig_.NoFeatureAvailableMsg = TCM_->NoFeatureAvailableMsg;
    prevSig_.CancelMsg = TCM_->CancelMsg;
    prevSig_.PauseMsg1 = TCM_->PauseMsg1;
    prevSig_.PauseMsg2 = TCM_->PauseMsg2;
    prevSig_.InfoMsg = TCM_->InfoMsg;
    prevSig_.InstructMsg = TCM_->InstructMsg;
    prevSig_.AcknowledgeRemotePIN = TCM_->AcknowledgeRemotePIN;
    prevSig_.ErrorMsg = TCM_->ErrorMsg;
    prevSig_.MobileChallengeSend = TCM_->MobileChallengeSend;

    // Populate assigned signal values.
    mapMsgText_( sigText_.ManeuverStatus, rawTextManeuverStatus );
    mapMsgText_( sigText_.NoFeatureAvailableMsg, rawTextNoFeatureAvailableMsg );
    mapMsgText_( sigText_.CancelMsg, rawTextCancelMsg );
    mapMsgText_( sigText_.PauseMsg1, rawTextPauseMsg1 );
    mapMsgText_( sigText_.PauseMsg2, rawTextPauseMsg2 );
    mapMsgText_( sigText_.InfoMsg, rawTextLMInfoRMT );
    mapMsgText_( sigText_.InstructMsg, rawTextInstructMsg );
    mapMsgText_( sigText_.AcknowledgeRemotePIN, rawAcknowledgeRemotePIN );
    mapMsgText_( sigText_.ErrorMsg, rawErrorMsg );


    TCM_->initiateEventLoops( );

    std::cout << "Using API version " << API_DOC_VERSION << std::endl;

}


RemoteDeviceHandler::~RemoteDeviceHandler( )
{
    eventLoopHandler_.join();
}


bool RemoteDeviceHandler::checkMessageReadability_( const ssize_t& receiptVal )
{

    if( receiptVal == -1 )
    {
        std::cout << "---" << std::endl;
        std::cout << "Message received is unreadable." << std::endl;

        return false;
    }
    if( receiptVal == 0 )
    {
        std::cout << "---" << std::endl;
        std::cout << "Message received is empty." << std::endl;

        return false;
    }

    return true;
}


bool RemoteDeviceHandler::checkClientConnection_( const std::string& msg )
{
    if( socketHandler_.checkClientConnection( ) != 0 )
    {
        std::cout << "---" << std::endl;
        std::cout << "Mobile device link to API severed." << std::endl;

        return false;
    }

    if( msg.find( "disconnectMobile" ) != std::string::npos )
    {

        std::cout << "---" << std::endl;
        std::cout << "Mobile device disconnecting from API."<< std::endl;

        return false;

    }

    return true;

}


void RemoteDeviceHandler::setConnectionApproved_(
        const TCM::ConnectionApproval& mode )
{
    // For FDJ, assume that if WiFi connection is established, then mobile
    // device is compatible for executing RPA.
    TCM_->setConnectionApproval( mode );
}


void RemoteDeviceHandler::setKeyFobRangingRate_(
        const DCM::FobRangeRequestRate& range )
{
    // Pass request rate for key fob ranging to SignalHandler
    TCM_->setFobRangeRequestRate( range );
}


void RemoteDeviceHandler::messageEvent_(
        const std::string& rawMsgIn,
        const uint32_t& headerLen,
        const uint32_t& bodyLen )
{

    json msgInHeader;
    json msgInBody;

    std::string rawHeader( rawMsgIn.substr( 0, headerLen ) );
    std::string rawBody( rawMsgIn.substr( headerLen, bodyLen ) );

    // leave per commonly-used state debugging statements.
    // std::cout << std::setw(4) << rawMsgIn << std::endl;
    // std::cout << "headerLen = " << (int)headerLen << std::endl;
    // std::cout << "rawHeader: " << rawHeader << std::endl;
    // std::cout << "bodyLen = " << (int)bodyLen << std::endl;
    // std::cout << "rawBody: " << rawBody << std::endl;

    try
    {
        msgInHeader = json::parse( rawHeader );
        if( !rawBody.empty( ) )
        {
            msgInBody = json::parse( rawBody );
        }
    }
    catch( std::exception& e )
    {

        std::cout << "Exception thrown parsing JSON message: " << e.what( );

        sendMsg_( "Parsing error; check JSON input.", RD::DEBUG );

        return;

    }

    if( msgInHeader.contains( "group" ) )
    {

        auto& msgGroup = msgInHeader[ "group" ];

        std::cout << "---" << std::endl;
        std::cout << "Message read: " << msgGroup << std::endl;

        // Parse messages by group
        if( msgGroup == RD::GET_API_VERSION )
        {
            sendVehicleAPIVersion_( );
        }
        else if( msgGroup == RD::SEND_PIN )
        {

            auto& msgPIN = msgInBody[ "pin" ];

            try
            {
                setRemoteControlPIN_( msgPIN );

                while(  TCM_->AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::None ||
                        TCM_->AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::ExpiredPIN )
                {

                    usleep( ASP_REFRESH_RATE );
                    setRemoteControlPIN_( msgPIN );

                }
            }
            catch( std::exception& e )
            {
                sendMsg_( "JSON template mismatch; Check input format.", RD::DEBUG );

                std::cout << "Exception thrown parsing JSON message: " << e.what( );

                return;
            }

            sendVehicleStatus_( );
            prevSig_.AcknowledgeRemotePIN = TCM_->AcknowledgeRemotePIN;

        }
        else if( msgGroup == RD::MOBILE_INIT )
        {
            // std::lock_guard<std::mutex> lock( TCM_->getMutex( ) );
            auto& msgTerms = msgInBody[ "terms_accepted" ];
            if( msgTerms == true && checkAuthenticatedPIN_( ) )
            {
                setConnectionApproved_( TCM::ConnectionApproval::AllowedDevice );
                if( checkDeviceCompatibility_( ) )
                {
                    if( !checkManeuversInProgress_( ) )
                    {
                        std::cout << "PIN Authenticated. Passing RCMainMenu to ASP."
                        << std::endl;

                        loadMainMenu_( );
                    }
                    else if( TCM_->ActiveParkingMode == ASP::ActiveParkingMode::ParkIn )
                    {
                        parkInSelected_( TCM_->getManeuverFromASP( ) );
                    }
                }
            }
            else
            {
                std::string error;
                if( !msgTerms )
                {
                    error += "* Message terms not accepted *";
                }
                if( !checkAuthenticatedPIN_( ) )
                {
                    error += "* PIN is invalid. Try send_pin again. *";
                }
                if( !checkDeviceCompatibility_( ) )
                {
                    error += "* There is no compatible device connected. *";
                }
                if( checkManeuversInProgress_( ) )
                {
                    error += "* Maneuvers in progress. *";
                }
                std::cout << error + " * bypassing RCMainMenu *" << std::endl;
            }

            // for mobile_init, return vehicle init.
            sendVehicleInit_( );
        }

        else if( msgGroup == RD::GET_THREAT_DATA )
        {

            // This if statement should be eventually be removed.  App currently
            // calls for cancel_maneuver at the end of a maneuver; simulated ASP
            // must therefore be reset
            if( TCM_->ManeuverStatus == ASP::ManeuverStatus::Cancelled )
            {
                loadMainMenu_( );
            }

            while( TCM_->ManeuverStatus == ASP::ManeuverStatus::Scanning )
            {
                usleep( ASP_REFRESH_RATE );
            }

            sendThreatData_( );
        }

        else if( msgGroup == RD::LIST_MANEUVERS )
        {
            // This if statement should be eventually be removed.  App currently
            // calls for cancel_maneuver at the end of a maneuver; simulated ASP
            // must therefore be reset
            if(     TCM_->ManeuverStatus == ASP::ManeuverStatus::Cancelled ||
                    TCM_->ManeuverStatus == ASP::ManeuverStatus::Finishing ||
                    TCM_->ManeuverStatus == ASP::ManeuverStatus::Ended )
            {
                loadMainMenu_( );
                usleep( ASP_REFRESH_RATE );
            }

            while(  TCM_->ManeuverStatus == ASP::ManeuverStatus::Scanning )
            {
                usleep( ASP_REFRESH_RATE );
            }

            if( !checkAuthenticatedPIN_( ) || !checkDeviceCompatibility_( ) )
            {

                // Per FDJ demo requirements, device compatability is implied
                // if connection is made, so PIN must be bad.
                sendMsg_( "Incorrect PIN entered.  Send mobile_init.", RD::DEBUG );

                return;

            }

            if( TCM_->ManeuverProgressBar == 0 || TCM_->ManeuverProgressBar == 100 )
            {
                loadSpaceSelection_( );
            }

            sendAvailableManeuvers_( );
        }

        else if( msgGroup == RD::MANEUVER_INIT )
        {

            if( msgInBody.contains("maneuver") )
            {

                if( !checkAuthenticatedPIN_( ) || !checkDeviceCompatibility_( ) )
                {

                    // Per FDJ demo requirements, device compatability is implied
                    // if connection is made, so PIN must be bad.
                    sendMsg_( "Incorrect PIN entered.  Send mobile_init.", RD::DEBUG );

                    return;

                }

                auto& msgManeuver = msgInBody[ "maneuver" ];

                // check if maneuver in progress is the same as inbound request
                if( checkManeuversInProgress_( ) )
                {
                    if( msgManeuver == TCM_->getManeuverFromASP( ) )
                    {
                        std::cout << "Duplicate maneuver_init received." << std::endl;
                        sendManeuverStatus_( );

                        return;
                    }

                    // **TECH DEBT** else reset the ASPM to space selection
                    else
                    {
                        loadSpaceSelection_( );
                        while(  TCM_->ConfirmAvailability == ASP::ConfirmAvailability::OfferEnabled ||
                                TCM_->ResumeAvailability == ASP::ResumeAvailability::OfferEnabled )
                        {
                            usleep( ASP_REFRESH_RATE );
                        }
                    }
                }

                if( msgManeuver == "StrFwd" || msgManeuver == "StrRvs" )
                {
                    pushPullSelected_( msgManeuver );
                }
                else if( msgManeuver == "InLftFwd" || msgManeuver == "InLftRvs" ||
                         msgManeuver == "InRgtFwd" || msgManeuver == "InRgtRvs" )
                {
                    parkInSelected_( msgManeuver );
                }
                else if( msgManeuver == "OutLftFwd" || msgManeuver == "OutLftRvs" ||
                         msgManeuver == "OutRgtFwd" || msgManeuver == "OutRgtRvs" ||
                         msgManeuver == "OutLftPrl" || msgManeuver == "OutRgtPrl" )
                {
                    parkOutSelected_( msgManeuver );
                }
                else if ( msgManeuver == "NdgFwd" || msgManeuver == "NdgRvs" )
                {
                    adjustSelected_( msgManeuver );
                }
                else if ( msgManeuver == "RtnToOgn" )
                {
                    returnToOriginSelected_( );
                }
                else
                {
                    std::cout << "Maneuver " << msgManeuver;
                    std::cout << " is not currently supported." << std::endl;

                    sendMsg_( "Received Maneuver not supported.", RD::DEBUG );

                    return;
                }

                // after 150ms of attempting to send the maneuver_init, drop out
                int maneuverInitAttemptCount( 0 );

                while(  ( TCM_->ConfirmAvailability == ASP::ConfirmAvailability::None
                        || TCM_->getManeuverFromASP( ) != msgManeuver )
                        && maneuverInitAttemptCount < 5 )
                {
                    // ++maneuverInitAttemptCount;
                    std::cout << "Default maneuver mismatch, attempt "
                    << ++maneuverInitAttemptCount << ":\tmsgManeuver: "
                    << msgManeuver << "\tgetManeuverFromASP: "
                    << TCM_->getManeuverFromASP( ) << std::endl;

                    usleep( ASP_REFRESH_RATE );
                }

                sendManeuverStatus_( );
            }
            else
            {
                sendMsg_( "Read error: No maneuver in 'data' available.", RD::DEBUG );

                return;
            }
        }

        else if( msgGroup == RD::MOBILE_RESPONSE )
        {
            try
            {
                std::string responseVal( msgInBody[ "response_to_challenge" ] );
                (std::istringstream)responseVal >> TCM_->MobileChallengeReply;
            }
            catch( std::exception& e )
            {
                std::cout << "Exception parsing mobile_challenge: " << e.what( );
                sendMsg_( "Parsing error; mobile_challenge format.", RD::DEBUG );
            }
        }


        else if( msgGroup == RD::DEADMANS_HANDLE )
        {

            auto& msgGestureEnabled = msgInBody[ "enable_vehicle_motion" ];
            auto& msgGestureProgress = msgInBody[ "dmh_gesture_progress" ];
            auto& msgAppSliderPosX = msgInBody[ "dmh_horizontal_touch" ];
            auto& msgAppSliderPosY = msgInBody[ "dmh_vertical_touch" ];
            auto& msgCRCValue = msgInBody[ "crc_value" ];

            updateDMH_(
                    msgAppSliderPosX,
                    msgAppSliderPosY,
                    msgGestureProgress,
                    msgGestureEnabled,
                    msgCRCValue );

        }

        else if( msgGroup == RD::CANCEL_DRIVE_ON )
        {
            // Unclear what to send beyond ManeuverButtonPress; it is assumed
            // that this only arrives if the vehicle is in some paused state.
            TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::ResumeSelected );
        }

        else if( msgGroup == RD::CANCEL_MANEUVER )
        {
            //  **TODO** What to send here??

            while( TCM_->ManeuverStatus != ASP::ManeuverStatus::Cancelled )
            {
                TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::CancellationSelected );
                usleep( ASP_REFRESH_RATE );
            }

        }

        else if( msgGroup == RD::VEHICLE_STATUS )
        {
            sendVehicleStatus_( );
        }

        else if( msgGroup == RD::MANEUVER_STATUS )
        {
            sendManeuverStatus_( );
        }

        else if ( msgGroup == RD::GET_CABIN_STATUS )
        {
            sendCabinStatus_( );
        }

        else if ( msgGroup == RD::CABIN_COMMANDS )
        {
            TCM_->setCabinCommands(msgInBody["engine_off"], msgInBody["doors_locked"]);

            // **TODO** The below is semi-deprecated until use case for
            // TCM::ManeuverButtonPress::EndManouevre is properly defined.
            // if (msgInBody["engine_off"] && msgInBody["doors_locked"])
            // {
            //     while( TCM_->ManeuverStatus != ASP::ManeuverStatus::Ended )
            //     {
            //         TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::EndManouevre );
            //         usleep( ASP_REFRESH_RATE );
            //     }
            // }
            sendCabinStatus_( );
        }

        else if( msgGroup == RD::MOBILE_CHALLENGE )
        {
            sendMsg_( "Message type not currently supported.", RD::DEBUG );
        }

        else
        {
            sendMsg_( "Read in error: Unknown group in msg.", RD::DEBUG );
        }

    }
    else
    {
        sendMsg_( "Read in error: No message group available.", RD::DEBUG );
    }
    return;
}


void RemoteDeviceHandler::sendMsg_(
        const json& msgOut,
        const std::string& msgGroup )
{

    json header = templates_.getRawHeaderTemplate();
    header[ "group" ] = msgGroup;
    std::string headerOut = header.dump( );

    if( socketHandler_.checkClientConnection( ) != 0 )
    {
        std::cout << "---" << std::endl;
        std::cout << "Mobile device not connected -- message of type "
        << headerOut << " generated but not sent."<< std::endl;

        return;
    }

    std::string bodyOut;

    // check if json in or just a debug msg as string.
    if ( msgGroup == RD::DEBUG )
    {
        json body = templates_.getRawVehicleStatusTemplate();

        body[ "status_9xx" ][ "status_code" ] = 999;
        body[ "status_9xx" ][ "status_code" ] = msgOut;

        // Populate response to client
        bodyOut = body.dump( );

        // TODO: the app can't seem handle the "debug" message group anymore without crashing,
        // we should either wait until it can before re-enabling these, or just remove the
        // group and just replace them with local error messages
        // For now we'll just print the debug message instead of sending it
        std::cout << "DEBUG - " << bodyOut << std::endl;
        return;
    }
    else
    {
        // Populate response to client
        bodyOut = msgOut.dump( );
    }

    // Send reply back to client
    socketHandler_.sendTCP( headerOut, bodyOut );

    return;
}


void RemoteDeviceHandler::sendVehicleAPIVersion_( )
{

    json msgOut = templates_.getRawVehicleAPIVersionTemplate();
    auto& msgVersion = msgOut[ "api_version" ];
    msgVersion = API_DOC_VERSION;

    sendMsg_( msgOut, RD::VEHICLE_API_VERSION );


    return;

}


void RemoteDeviceHandler::sendVehicleStatus_( )
{

    json msgOut = templates_.getRawVehicleStatusTemplate( );

    auto& msgStatusCode1 = msgOut[ "status_1xx" ][ "status_code" ];
    auto& msgStatusCode2 = msgOut[ "status_2xx" ][ "status_code" ];
    auto& msgStatusCode3 = msgOut[ "status_3xx" ][ "status_code" ];
    auto& msgStatusCode4 = msgOut[ "status_4xx" ][ "status_code" ];
    auto& msgStatusCode5 = msgOut[ "status_5xx" ][ "status_code" ];
    auto& msgStatusCode6 = msgOut[ "status_6xx" ][ "status_code" ];
    auto& msgStatusCode7 = msgOut[ "status_7xx" ][ "status_code" ];
    auto& msgStatusCode8 = msgOut[ "status_8xx" ][ "status_code" ];
    auto& msgStatusCode9 = msgOut[ "status_9xx" ][ "status_code" ];

    auto& msgStatusText1 = msgOut[ "status_1xx" ][ "status_text" ];
    auto& msgStatusText2 = msgOut[ "status_2xx" ][ "status_text" ];
    auto& msgStatusText3 = msgOut[ "status_3xx" ][ "status_text" ];
    auto& msgStatusText4 = msgOut[ "status_4xx" ][ "status_text" ];
    auto& msgStatusText5 = msgOut[ "status_5xx" ][ "status_text" ];
    auto& msgStatusText6 = msgOut[ "status_6xx" ][ "status_text" ];
    auto& msgStatusText7 = msgOut[ "status_7xx" ][ "status_text" ];
    auto& msgStatusText8 = msgOut[ "status_8xx" ][ "status_text" ];
    auto& msgStatusText9 = msgOut[ "status_9xx" ][ "status_text" ];

    msgStatusCode1 = prefixStatus_( VehicleStatusPrefix::NoFeatureAvailableMsg );
    msgStatusCode2 = prefixStatus_( VehicleStatusPrefix::CancelMsg );
    msgStatusCode3 = prefixStatus_( VehicleStatusPrefix::PauseMsg1 );
    msgStatusCode4 = prefixStatus_( VehicleStatusPrefix::PauseMsg2 );
    msgStatusCode5 = prefixStatus_( VehicleStatusPrefix::InfoMsg );
    msgStatusCode6 = prefixStatus_( VehicleStatusPrefix::InstructMsg );
    msgStatusCode7 = prefixStatus_( VehicleStatusPrefix::AcknowledgeRemotePIN );
    msgStatusCode8 = prefixStatus_( VehicleStatusPrefix::ErrorMsg );
    msgStatusCode9 = prefixStatus_( VehicleStatusPrefix::ManeuverStatus );

    msgStatusText1 = sigText_.NoFeatureAvailableMsg[ (int)TCM_->NoFeatureAvailableMsg ];
    msgStatusText2 = sigText_.CancelMsg[ (int)TCM_->CancelMsg ];
    msgStatusText3 = sigText_.PauseMsg1[ (int)TCM_->PauseMsg1 ];
    msgStatusText4 = sigText_.PauseMsg2[ (int)TCM_->PauseMsg2 ];
    msgStatusText5 = sigText_.InfoMsg[ (int)TCM_->InfoMsg ];
    msgStatusText6 = sigText_.InstructMsg[ (int)TCM_->InstructMsg ];
    msgStatusText7 = sigText_.AcknowledgeRemotePIN[ (int)TCM_->AcknowledgeRemotePIN ];
    msgStatusText8 = sigText_.ErrorMsg[ (int)TCM_->ErrorMsg ];
    msgStatusText9 = sigText_.ManeuverStatus[ (int)TCM_->ManeuverStatus ];

    sendMsg_( msgOut, RD::VEHICLE_STATUS );

    return;
}


void RemoteDeviceHandler::sendVehicleInit_( )
{

    json msgOut = templates_.getRawVehicleInitTemplate();
    auto& msgReady = msgOut[ "ready" ];
    auto& msgActiveManeuver = msgOut[ "active_maneuver" ];

    // **TODO** verify that "ready" in JSON struct is validated accordingly.
    // If PIN and device are good, send true, even before scan is complete.
    msgReady = false;
    if( TCM_->ConnectionApproval == TCM::ConnectionApproval::AllowedDevice
            && TCM_->AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::CorrectPIN
            && TCM_->ActiveAutonomousFeature == ASP::ActiveAutonomousFeature::Parking
            // || TCM_->InfoMsg == ASP::InfoMsg::RemoteManeuverReady )
            )
    {
        msgReady = true;
    }

    // Mobile only checks for this flag when it is returning from an
    // mid-maneuver state.
    msgActiveManeuver = false;
    switch( TCM_->ManeuverStatus )
    {
        case ASP::ManeuverStatus::Selecting:
        {
            if( TCM_->ConfirmAvailability != ASP::ConfirmAvailability::OfferEnabled )
            {
                break;
            }
        }
        case ASP::ManeuverStatus::Confirming:
        case ASP::ManeuverStatus::Maneuvering:
        case ASP::ManeuverStatus::Interrupted:
        case ASP::ManeuverStatus::Finishing:   // <--??
        {
            msgActiveManeuver = true;

            sendManeuverStatus_( );

            break;
        }
        default:
        {
            break;
        }

    }

    sendMsg_( msgOut, RD::VEHICLE_INIT );

    return;

}


void RemoteDeviceHandler::sendThreatData_( )
{

    json msgOut = templates_.getRawThreatDataTemplate();
    auto& msgThreats = msgOut[ "threats" ];

    /*  Send back threat data based on the front and rear integer vectors.
    //  Since the pattern starts at the driver side door and makes a
    //  continuous circle, the rear values must be inverted when populating.
    */
    for( int i = 0; i < TCM_->ASPMFrontSegDistxxRMT.size( ); i++ )
    {
        if (TCM_->getASPMFrontSegTypexxRMT(i) == 1) { // 1 indicates a threat is detected
            msgThreats[ i ] = TCM_->getASPMFrontSegDistxxRMT(i);
        }
        else { // no threat detected
            msgThreats[ i ] = 19;
        }
        if (TCM_->getASPMRearSegTypexxRMT(i) == 1) { // 1 indicates a threat is detected
            msgThreats[ 31-i ] = TCM_->getASPMRearSegDistxxRMT(i);
        }
        else { // no threat detected
            msgThreats[ 31-i ] = 19;
        }
    }

    sendMsg_( msgOut, RD::THREAT_DATA );


    return;

}


void RemoteDeviceHandler::sendAvailableManeuvers_( )
{

    json msgOut = templates_.getRawAvailableManeuversTemplate();
    auto& msgDefaultManeuver = msgOut[ "default" ];
    auto& msgContinueExplore = msgOut[ "continue_exploring" ];
    auto& msgManeuversPOLF = msgOut[ "maneuvers" ][ "OutLftFwd" ];
    auto& msgManeuversPOLR = msgOut[ "maneuvers" ][ "OutLftRvs" ];
    auto& msgManeuversPORF = msgOut[ "maneuvers" ][ "OutRgtFwd" ];
    auto& msgManeuversPORR = msgOut[ "maneuvers" ][ "OutRgtRvs" ];
    auto& msgManeuversPILF = msgOut[ "maneuvers" ][ "InLftFwd" ];
    auto& msgManeuversPILR = msgOut[ "maneuvers" ][ "InLftRvs" ];
    auto& msgManeuversPIRF = msgOut[ "maneuvers" ][ "InRgtFwd" ];
    auto& msgManeuversPIRR = msgOut[ "maneuvers" ][ "InRgtRvs" ];
    auto& msgManeuversNF = msgOut[ "maneuvers" ][ "NdgFwd" ];
    auto& msgManeuversNR = msgOut[ "maneuvers" ][ "NdgRvs" ];
    auto& msgManeuversSF = msgOut[ "maneuvers" ][ "StrFwd" ];
    auto& msgManeuversSR = msgOut[ "maneuvers" ][ "StrRvs" ];
    auto& msgManeuversPOLP = msgOut[ "maneuvers" ][ "OutLftPrl" ];
    auto& msgManeuversPORP = msgOut[ "maneuvers" ][ "OutRgtPrl" ];
    auto& msgManeuversRTS = msgOut[ "maneuvers" ][ "RtnToOgn" ];

    /*  The logic below is for populating the Push/Pull maneuvers.  First,
    //  the ExploreModeAvailability signal is referenced to see whether
    //  Push/Pull is offered.  In the case that it is, ManeuverDirectionAvailability
    //  references in which direction push/pull can be carried out.
    */
    switch( TCM_->ExploreModeAvailability )
    {

        // int value of 0, 2, 3, --> false
        case ASP::ExploreModeAvailability::None:              break;
        case ASP::ExploreModeAvailability::OfferDisabled:     break;
        case ASP::ExploreModeAvailability::Reserved1:         break;

        // int value of 1 --> true
        case ASP::ExploreModeAvailability::OfferEnabled:
        {
            // Check forward, backward, or both
            switch( TCM_->ManeuverDirectionAvailability )
            {

                // int value of 0 --> ** UNKNOWN **
                case ASP::ManeuverDirectionAvailability::None:        break;

                // int value of 1 --> offer straight forward ("StrFwd")
                case ASP::ManeuverDirectionAvailability::OfferForward:
                {
                    msgManeuversSF = true;
                    break;
                }

                // int value of 2 --> offer straight rear ("StrRvs")
                case ASP::ManeuverDirectionAvailability::OfferBackward:
                {
                    msgManeuversSR = true;
                    break;
                }

                // int value of 3 --> offer "StrFwd" and "StrRvs"
                case ASP::ManeuverDirectionAvailability::OfferForwardAndBackward:
                {
                    msgManeuversSF = true;
                    msgManeuversSR = true;
                    break;
                }

                default: break;
            }
            break;
        }

        // scaffolding to parse improperly initialized signals if needed
        case ASP::ExploreModeAvailability::InvalidSig:        break;
        default:                                            break;
    }

    /*  The logic below is for populating the nudge maneuvers.  First, the
    //  LongitudinalAdjustAvailability signal is referenced to see whether nudge is
    //  offered.  In the case that it is, ManeuverDirectionAvailability is referenced
    //  to check in which direction nudging can be carried out.
    */
    switch( TCM_->LongitudinalAdjustAvailability )
    {

        // int value of 0, 2, 3, --> false
        case ASP::LongitudinalAdjustAvailability::None:              break;
        case ASP::LongitudinalAdjustAvailability::OfferDisabled:     break;
        case ASP::LongitudinalAdjustAvailability::Reserved1:         break;

        // int value of 1 --> true
        case ASP::LongitudinalAdjustAvailability::OfferEnabled:
        {
            // Check forward, backward, or both
            switch( TCM_->ManeuverDirectionAvailability )
            {

                // int value of 0 --> ** UNKNOWN **
                case ASP::ManeuverDirectionAvailability::None:        break;

                // int value of 1 --> offer nudge forward ("NdgFwd")
                case ASP::ManeuverDirectionAvailability::OfferForward:
                {
                    msgManeuversNF = true;
                    break;
                }

                // int value of 2 --> offer nudge rear ("NdgRvs")
                case ASP::ManeuverDirectionAvailability::OfferBackward:
                {
                    msgManeuversNR = true;
                    break;
                }

                // int value of 3 --> offer "NdgFwd" and "NdgRvs"
                case ASP::ManeuverDirectionAvailability::OfferForwardAndBackward:
                {
                    msgManeuversNF = true;
                    msgManeuversNR = true;
                    break;
                }

                default: break;
            }
            break;
        }

        // scaffolding to parse improperly initialized signals if needed
        case ASP::LongitudinalAdjustAvailability::InvalidSig:        break;
        default:                                            break;
    }

    /*  The logic below is for populating the return to start maneuver.
    //  This leverages the ASP::ReturnToStartAvailability signal and returns true if
    //  it is enabled (int val of 1)
    */
    switch( TCM_->ReturnToStartAvailability )
    {

        // int value of 0, 2, 3, --> false
        case ASP::ReturnToStartAvailability::None:               break;
        case ASP::ReturnToStartAvailability::OfferDisabled:      break;
        case ASP::ReturnToStartAvailability::Reserved1:          break;

        // int value of 1 --> true
        case ASP::ReturnToStartAvailability::OfferEnabled:
        {

            msgManeuversRTS = true;
            break;
        }

        // scaffolding to parse improperly initialized signals if needed
        case ASP::ReturnToStartAvailability::InvalidSig:         break;
        default:                                            break;
    }

    /*  The logic below is for populating the ParkIn and ParkOut maneuvers.
    //  First, the ActiveParkingMode referenced to see whether ParkIn or ParkOut
    //  is offered.  In the case that it is, ActiveManeuverOrientation and
    //  ManeuverSideAvailability is referenced to check in which direction the maneuver
    //  can be carried out.
    */
    switch( TCM_->ActiveParkingMode )
    {

        // int value of 0, 3 --> false
        case ASP::ActiveParkingMode::None:                   break;
        case ASP::ActiveParkingMode::Reserved1:              break;

        // int value of 1 --> ParkIn
        case ASP::ActiveParkingMode::ParkIn:
        {
            // Check parallel, perpendicular rear or front
            switch( TCM_->ActiveManeuverOrientation )
            {

                // int value of 0 --> ** UNKNOWN **
                case ASP::ActiveManeuverOrientation::None:           break;

                // int value of 1 --> Parallel is descoped from FDJ
                case ASP::ActiveManeuverOrientation::Parallel:       break;

                // int value of 2 --> PerpendicularRear
                case ASP::ActiveManeuverOrientation::PerpendicularRear:
                {
                    // Check left, right, or both
                    switch( TCM_->ManeuverSideAvailability )
                    {

                        // int value of 0 --> ** UNKNOWN **
                        case ASP::ManeuverSideAvailability::None:  break;

                        // int value of 1 --> offer in, left, rear ("InLftRvs")
                        case ASP::ManeuverSideAvailability::OfferLeftOnly:
                        {
                            msgManeuversPILR = true;
                            break;
                        }

                        // int value of 2 --> offer in, right, rear ("InRgtRvs")
                        case ASP::ManeuverSideAvailability::OfferRightOnly:
                        {
                            msgManeuversPIRR = true;
                            break;
                        }

                        // int value of 3 --> offer both "InLftRvs" and "InRgtRvs"
                        case ASP::ManeuverSideAvailability::OfferLeftAndRight:
                        {
                            msgManeuversPILR = true;
                            msgManeuversPIRR = true;
                            break;
                        }
                        // scaffolding to parse improperly initialized sig
                        case ASP::ManeuverSideAvailability::InvalidSig:    break;
                        default:                                    break;
                    }
                    break;
                }

                // int value of 3 --> PerpendicularFront
                case ASP::ActiveManeuverOrientation::PerpendicularFront:
                {
                    // Check left, right, or both
                    switch( TCM_->ManeuverSideAvailability )
                    {

                        // int value of 0 --> ** UNKNOWN **
                        case ASP::ManeuverSideAvailability::None:  break;

                        // int value of 1 --> offer in, left, front ("InLftFwd")
                        case ASP::ManeuverSideAvailability::OfferLeftOnly:
                        {
                            msgManeuversPILF = true;
                            break;
                        }

                        // int value of 2 --> offer in, right, front ("InRgtFwd")
                        case ASP::ManeuverSideAvailability::OfferRightOnly:
                        {
                            msgManeuversPIRF = true;
                            break;
                        }

                        // int value of 3 --> offer both "InLftFwd" and "InRgtFwd"
                        case ASP::ManeuverSideAvailability::OfferLeftAndRight:
                        {
                            msgManeuversPILF = true;
                            msgManeuversPIRF = true;
                            break;
                        }
                        // scaffolding to parse improperly initialized sig
                        case ASP::ManeuverSideAvailability::InvalidSig:    break;
                        default:                                    break;
                    }
                    break;
                }

                // scaffolding to parse improperly initialized sig if needed
                case ASP::ActiveManeuverOrientation::InvalidSig:     break;
                default:                                    break;
            }
            break;
        }

        // int value of 2 --> ParkOut
        case ASP::ActiveParkingMode::ParkOut:
        {
            // Conditional for if the vehicle has not yet physically moved and
            // therefore has no confidence that available maneuvers are actually
            // available.  In that condition, send all ParkOut maneuvers.
            if( TCM_->hasVehicleMoved == false )
            {
                std::cout << "---" << std::endl
                << "Vehicle has not yet moved so ignoring ASPM-generated ParkOut maneuvers."
                << std::endl;

                msgManeuversPOLP = true;
                msgManeuversPORP = true;
                msgManeuversPOLF = true;
                msgManeuversPORF = true;
                msgManeuversPOLR = true;
                msgManeuversPORR = true;

                break;
            }
            else
            {
                std::cout << "---" << std::endl
                << "Vehicle has moved so passing along ASPM-generated ParkOut maneuvers."
                << std::endl;
            }

            // Check parallel, perpendicular rear or front
            switch( TCM_->ActiveManeuverOrientation )
            {

                // int value of 0 --> ** UNKNOWN **
                case ASP::ActiveManeuverOrientation::None:           break;

                // int value of 1 --> Parallel
                case ASP::ActiveManeuverOrientation::Parallel:
                {
                    // Check left, right, or both
                    switch( TCM_->ManeuverSideAvailability )
                    {

                        // int value of 0 --> ** UNKNOWN **
                        case ASP::ManeuverSideAvailability::None:  break;

                        // int value of 1 --> offer out, left, parallel ("OutLftPrl")
                        case ASP::ManeuverSideAvailability::OfferLeftOnly:
                        {
                            msgManeuversPOLP = true;
                            break;
                        }

                        // int value of 2 --> offer out, right, parallel ("OutRgtPrl")
                        case ASP::ManeuverSideAvailability::OfferRightOnly:
                        {
                            msgManeuversPORP = true;
                            break;
                        }

                        // int value of 3 --> offer both "OutLftPrl" and "OutRgtPrl"
                        case ASP::ManeuverSideAvailability::OfferLeftAndRight:
                        {
                            msgManeuversPOLP = true;
                            msgManeuversPORP = true;
                            break;
                        }
                        // scaffolding to parse improperly initialized sig
                        case ASP::ManeuverSideAvailability::InvalidSig:    break;
                        default:                                    break;
                    }
                    break;
                }

                // int value of 3 --> PerpendicularFront
                case ASP::ActiveManeuverOrientation::PerpendicularFront:
                {
                    // Check left, right, or both
                    switch( TCM_->ManeuverSideAvailability )
                    {

                        // int value of 0 --> ** UNKNOWN **
                        case ASP::ManeuverSideAvailability::None:  break;

                        // int value of 1 --> offer out, left, front ("OutLftFwd")
                        case ASP::ManeuverSideAvailability::OfferLeftOnly:
                        {
                            msgManeuversPOLF = true;
                            break;
                        }

                        // int value of 2 --> offer out, right, front ("OutRgtFwd")
                        case ASP::ManeuverSideAvailability::OfferRightOnly:
                        {
                            msgManeuversPORF = true;
                            break;
                        }

                        // int value of 3 --> offer both "OutLftFwd" and "OutRgtFwd"
                        case ASP::ManeuverSideAvailability::OfferLeftAndRight:
                        {
                            msgManeuversPOLF = true;
                            msgManeuversPORF = true;
                            break;
                        }
                        // scaffolding to parse improperly initialized sig
                        case ASP::ManeuverSideAvailability::InvalidSig:    break;
                        default:                                    break;
                    }
                    break;
                }

                // int value of 2 --> PerpendicularRear
                case ASP::ActiveManeuverOrientation::PerpendicularRear:
                {
                    // Check left, right, or both
                    switch( TCM_->ManeuverSideAvailability )
                    {

                        // int value of 0 --> ** UNKNOWN **
                        case ASP::ManeuverSideAvailability::None:  break;

                        // int value of 1 --> offer out, left, rear ("OutLftRvs")
                        case ASP::ManeuverSideAvailability::OfferLeftOnly:
                        {
                            msgManeuversPOLR = true;
                            break;
                        }

                        // int value of 2 --> offer out, right, rear ("OutRgtRvs")
                        case ASP::ManeuverSideAvailability::OfferRightOnly:
                        {
                            msgManeuversPORR = true;
                            break;
                        }

                        // int value of 3 --> offer both "OutLftRvs" and "OutRgtRvs"
                        case ASP::ManeuverSideAvailability::OfferLeftAndRight:
                        {
                            msgManeuversPOLR = true;
                            msgManeuversPORR = true;
                            break;
                        }
                        // scaffolding to parse improperly initialized sig
                        case ASP::ManeuverSideAvailability::InvalidSig:    break;
                        default:                                    break;
                    }
                    break;
                }

                // scaffolding to parse improperly initialized sig if needed
                case ASP::ActiveManeuverOrientation::InvalidSig:     break;
                default:                                    break;
            }
            break;
        }

        // scaffolding to parse improperly initialized signals if needed
        case ASP::ActiveParkingMode::InvalidSig:             break;
        default:                                            break;
    }

    msgDefaultManeuver = TCM_->getManeuverFromASP( );
    msgContinueExplore = checkContinueExploratoryMode_( );

    // std::cout << std::setw(4) << msgOut << std::endl;

    sendMsg_( msgOut, RD::AVAILABLE_MANEUVERS );


    return;

}


void RemoteDeviceHandler::sendMobileChallenge_( )
{

    json msgOut = templates_.getRawMobileChallengeTemplate();
    auto& msgBytes = msgOut[ "packed_bytes" ];

    std::ostringstream challengeVal;
    challengeVal << TCM_->MobileChallengeSend;
    msgBytes = challengeVal.str( );

    sendMsg_( msgOut, RD::MOBILE_CHALLENGE );


    return;

}


void RemoteDeviceHandler::sendManeuverStatus_( )
{

    json msgOut = templates_.getRawManeuverStatusTemplate();
    auto& msgManeuver = msgOut[ "maneuver" ];
    auto& msgProgress = msgOut[ "progress" ];
    auto& msgStatus = msgOut[ "status" ];

    msgManeuver = TCM_->getManeuverFromASP( );
    msgProgress = (int)TCM_->ManeuverProgressBar;
    msgStatus = prefixStatus_( VehicleStatusPrefix::ManeuverStatus );

    sendMsg_( msgOut, RD::MANEUVER_STATUS );


    return;

}

void RemoteDeviceHandler::sendCabinStatus_()
{
    json msgOut = templates_.getRawCabinStatusTemplate();
    auto& msgEngineOff = msgOut[ "power_status" ];
    auto& msgDoorsLocked = msgOut[ "lock_status" ];
    msgEngineOff = TCM_->getEngineOff();
    msgDoorsLocked = TCM_->getDoorsLocked();
    sendMsg_( msgOut, RD::CABIN_STATUS );
}


void RemoteDeviceHandler::toggleEngineOnOff_( )
{
    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::None );
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCStartStop );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}


void RemoteDeviceHandler::loadMainMenu_( )
{
    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::None );
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCMainMenu );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}


void RemoteDeviceHandler::loadSpaceSelection_( )
{
    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::None );
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCParkOutSpaceSlctn );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}

void RemoteDeviceHandler::pushPullSelected_( const std::string& direction )
{

    TCM_->setInputManeuverSignals( TCM_->parseManeuverString( direction ) );

    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCPushPull );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}

void RemoteDeviceHandler::parkInSelected_( const std::string& direction )
{

    TCM_->setInputManeuverSignals( TCM_->parseManeuverString( direction ) );

    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCParkIn );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}

void RemoteDeviceHandler::parkOutSelected_( const std::string& direction )
{

    TCM_->setInputManeuverSignals( TCM_->parseManeuverString( direction ) );

    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCParkOut );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}

void RemoteDeviceHandler::adjustSelected_( const std::string& direction )
{

    TCM_->setInputManeuverSignals( TCM_->parseManeuverString( direction ) );

    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::RCAdjust );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}

void RemoteDeviceHandler::returnToOriginSelected_( )
{
    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */
    TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::ReturnToStart );
    TCM_->setDeviceControlMode( TCM::DeviceControlMode::NoMode );
    TCM_->setManeuverEnableInput( 0000, 0000, 0000, false );

}

void RemoteDeviceHandler::updateDMH_(
        const float& x,
        const float& y,
        const int& percent,
        const bool& gesture,
        const int& crc )
{
    TCM_->setManeuverEnableInput(
            static_cast<uint16_t>( x ),
            static_cast<uint16_t>( y ),
            static_cast<int64_t>( percent ),
            gesture );

    TCM_->AppCalcCheck = static_cast<uint16_t>( crc );

    switch( TCM_->ManeuverStatus )
    {
        case ASP::ManeuverStatus::Interrupted:
        {
            TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::ResumeSelected );
            break;
        }
        case ASP::ManeuverStatus::Selecting:
        {
            TCM_->setManeuverButtonPress( TCM::ManeuverButtonPress::ConfirmationSelected );
            break;
        }
        default:{ break; }

    }

    // sendMsg_( "DMH received.", RD::DEBUG );

    return;
}


void RemoteDeviceHandler::setRemoteControlPIN_( const std::string& pin )
{

    if(     pin == DCM::PIN_NOT_SET &&
            TCM_->AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::None )
    {
        return;
    }
    else if( pin == DCM::PIN_NOT_SET )
    {
        std::cout << "---" << std::endl << "Resetting InControlRemotePIN in VDC.";
        std::cout << std::endl;
    }
    else
    {
        std::cout << "---" << std::endl << "Passing PIN " << pin;
        std::cout << " to TCM for Authentication." << std::endl;
    }

    TCM_->setInControlRemotePin( pin );

    return;

}


bool RemoteDeviceHandler::checkAuthenticatedPIN_( )
{

    if( TCM_->AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::CorrectPIN )
    {
        return true;
    }

    return false;

}


bool RemoteDeviceHandler::checkDeviceCompatibility_( )
{
    if( TCM_->ConnectionApproval == TCM::ConnectionApproval::AllowedDevice )
    {
        return true;
    }

    return false;
}


bool RemoteDeviceHandler::checkManeuversInProgress_( )
{
    switch( TCM_->ManeuverStatus )
    {
        case ASP::ManeuverStatus::Selecting:
        {
            if( TCM_->ConfirmAvailability != ASP::ConfirmAvailability::OfferEnabled )
            {
                break;
            }
        }
        case ASP::ManeuverStatus::Confirming:
        case ASP::ManeuverStatus::Maneuvering:
        case ASP::ManeuverStatus::Interrupted:
        case ASP::ManeuverStatus::Finishing:
        {
            return true;
        }
        default: break;
    }

    return false;
}


bool RemoteDeviceHandler::checkContinueExploratoryMode_( )
{
    if( TCM_->ExploreModeAvailability != ASP::ExploreModeAvailability::OfferDisabled )
    {
        return true;
    }

    return false;
}


bool RemoteDeviceHandler::checkForNewStatusSignals_( std::atomic<bool>& statusChanged )
{

    while( prevSig_.ManeuverStatus != TCM_->ManeuverStatus )
    {
        prevSig_.ManeuverStatus = TCM_->ManeuverStatus;

        switch( TCM_->ManeuverStatus )
        {
            case ASP::ManeuverStatus::Ended:
            {
                statusChanged = true;
                break;
            }
            default:
            {
                sendManeuverStatus_( );
            }
        }
    }

    while( prevSig_.NoFeatureAvailableMsg != TCM_->NoFeatureAvailableMsg )
    {
        prevSig_.NoFeatureAvailableMsg = TCM_->NoFeatureAvailableMsg;
        statusChanged = true;
    }

    while( prevSig_.CancelMsg != TCM_->CancelMsg )
    {
        prevSig_.CancelMsg = TCM_->CancelMsg;
        statusChanged = true;
    }

    while( prevSig_.PauseMsg1 != TCM_->PauseMsg1 )
    {
        prevSig_.PauseMsg1 = TCM_->PauseMsg1;
        statusChanged = true;
    }

    while( prevSig_.PauseMsg2 != TCM_->PauseMsg2 )
    {
        prevSig_.PauseMsg2 = TCM_->PauseMsg2;
        statusChanged = true;
    }

    while( prevSig_.InfoMsg != TCM_->InfoMsg )
    {
        prevSig_.InfoMsg = TCM_->InfoMsg;
        statusChanged = true;
    }

    while( prevSig_.InstructMsg != TCM_->InstructMsg )
    {
        prevSig_.InstructMsg = TCM_->InstructMsg;
        statusChanged = true;
    }

    while( prevSig_.AcknowledgeRemotePIN != TCM_->AcknowledgeRemotePIN )
    {
        prevSig_.AcknowledgeRemotePIN = TCM_->AcknowledgeRemotePIN;
        statusChanged = true;
    }

    while( prevSig_.ErrorMsg != TCM_->ErrorMsg )
    {
        prevSig_.ErrorMsg = TCM_->ErrorMsg;
        statusChanged = true;
    }

    while( prevSig_.MobileChallengeSend != TCM_->MobileChallengeSend )
    {

        sendMobileChallenge_( );
        prevSig_.MobileChallengeSend = TCM_->MobileChallengeSend;

    }

    if( statusChanged == true )
    {
        return true;
    }

    return false;
}


int RemoteDeviceHandler::prefixStatus_( const VehicleStatusPrefix& prefix )
{
    switch( prefix )
    {
        case VehicleStatusPrefix::NoFeatureAvailableMsg:
        {
            return (int)prefix + (int)TCM_->NoFeatureAvailableMsg;
        }
        case VehicleStatusPrefix::CancelMsg:
        {
            return (int)prefix + (int)TCM_->CancelMsg;
        }
        case VehicleStatusPrefix::PauseMsg1:
        {
            return (int)prefix + (int)TCM_->PauseMsg1;
        }
        case VehicleStatusPrefix::PauseMsg2:
        {
            return (int)prefix + (int)TCM_->PauseMsg2;
        }
        case VehicleStatusPrefix::InfoMsg:
        {
            return (int)prefix + (int)TCM_->InfoMsg;
        }
        case VehicleStatusPrefix::InstructMsg:
        {
            return (int)prefix + (int)TCM_->InstructMsg;
        }
        case VehicleStatusPrefix::AcknowledgeRemotePIN:
        {
            return (int)prefix + (int)TCM_->AcknowledgeRemotePIN;
        }
        case VehicleStatusPrefix::ErrorMsg:
        {
            return (int)prefix + (int)TCM_->ErrorMsg;
        }
        case VehicleStatusPrefix::ManeuverStatus:
        {
            return (int)prefix + (int)TCM_->ManeuverStatus;
        }

        default: break;
    }

    return 0;
}


void RemoteDeviceHandler::mapMsgText_(
        std::vector<std::string>& sigText,
        const char* data )
{

    const char* ptr = data;
    sigText.clear( );

    try
    {

        std::string newLine;
        while( *ptr != '\0' )
        {
            newLine = "";
            while( *ptr != ' ' )
            {
                // if( *pointer != '\r' )
                newLine += *ptr;
                ++ptr;
            }

            sigText.push_back( newLine );
            ++ptr;

        }

        return;

    }
    catch( std::exception& e )
    {

        std::cout << "Error populating signal text values: " << e.what( );

        return;

    }

}


void RemoteDeviceHandler::statusUpdateEventLoop_( )
{

    std::atomic<bool> hasVehicleStatusChanged( false );

    while( running_ )
    {

        if( checkForNewStatusSignals_( hasVehicleStatusChanged ) == true )
        {

            hasVehicleStatusChanged = false;

            while( checkForNewStatusSignals_( hasVehicleStatusChanged ) == true )
            {

                hasVehicleStatusChanged = false;

            }

            sendVehicleStatus_( );

        }

        usleep( ASP_REFRESH_RATE );

    }

}


void RemoteDeviceHandler::spin( )
{
    ssize_t readVal = 0;
    std::string receivedMsg = "";
    uint32_t headerLen = 0;
    uint32_t bodyLen = 0;

    socketHandler_.connectServer(
            (int32_t)SOCK_STREAM,
            (uint64_t)TCP_ADDR,
            (uint16_t)TCP_PORT );

    while( running_ )
    {
        socketHandler_.connectClient( );

        setConnectionApproved_( TCM::ConnectionApproval::NotAllowedDevice );
        setKeyFobRangingRate_( DCM::FobRangeRequestRate::DefaultRate );

        while( socketHandler_.isClientConnected == true )
        {
            receivedMsg.clear( );
            socketHandler_.receiveTCP( readVal, receivedMsg, headerLen, bodyLen );

            if( checkClientConnection_( receivedMsg.data( ) ) == false
                || checkMessageReadability_( readVal ) == false )
                {
                    setConnectionApproved_( TCM::ConnectionApproval::NoDevice );
                    setKeyFobRangingRate_( DCM::FobRangeRequestRate::None );
                    setRemoteControlPIN_( DCM::PIN_NOT_SET );

                    socketHandler_.disconnectClient( );

                    continue;
                }

            messageEvent_( receivedMsg.data( ), headerLen, bodyLen );

            if( TCM_->AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::NotSetInDCM )
            {
                sendVehicleStatus_( );
            }
        }
    }
    socketHandler_.disconnectServer( );
}

void RemoteDeviceHandler::stop( )
{
    TCM_->stop();
    running_ = false;
    socketHandler_.disconnectClient(false);
    socketHandler_.disconnectServer();
}
