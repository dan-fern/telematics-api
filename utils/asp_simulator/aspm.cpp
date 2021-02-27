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
 */

#include "aspm.hpp"

using namespace std::placeholders;

// Constructor initializes all member variables.
ASPM::ASPM( SIMULATOR type, MANOUEVRE testManeuver )
        :
        isManeuveringWithValidDMH( false ),
        isProgressHandledExternally_( false ),
        scanStartTime_( (struct timeval){0} ),
        dmhStartTime_( (struct timeval){0} ),
        socketHandler_( ),
        initialized_( false ),
        running_( true ),
        challenge_idx_( 0 ),
        challenge_warning_( false ),
        explore_idx_( 1 ),
        explore_limit_reached_( false ),
        lastExploreManeuver_( MANOUEVRE::NADA ),
        preferredSimManeuver_( testManeuver ),
        oppositeSimManeuver_( MANOUEVRE::NADA )
{
    simType_ = type;
    changePreferredSimMove( testManeuver );
    ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
    if( simType_ == SIMULATOR::EV3 )
    {
        std::cout << "Instantiating ASP Simulator in EV3 Mode." << std::endl;
        isProgressHandledExternally_ = true;
    }
    srand( time( NULL ) );
}


ASPM::~ASPM( )
{
    if (initialized_) {
        signalLoopHandler_.join( );
        progressBarLoopHandler_.join( );
    }
}

void ASPM::stop() {
    running_ = false;
    socketHandler_.disconnectClient( false );
    socketHandler_.disconnectServer( );
    signalLoopHandler_.join( );
    progressBarLoopHandler_.join( );
}

void ASPM::initiateEventLoops( )
{
    // Generate a new UDP socket
    socketHandler_.connectServer(
            (int32_t)SOCK_DGRAM,
            (uint64_t)UDP_ADDR,
            (uint16_t)UDP_PORT );
    initialized_ = true;
    signalLoopHandler_ = std::thread( &ASPM::updateSignalEventLoop_, this );
    progressBarLoopHandler_ = std::thread( &ASPM::progressBarEventLoop_, this );
}


void ASPM::changePreferredSimMove( const std::string& maneuver )
{

    changePreferredSimMove( parseManeuverString( maneuver ) );

    return;
}


void ASPM::changePreferredSimMove( const MANOUEVRE& maneuver )
{
    preferredSimManeuver_ = maneuver;

    switch( maneuver )
    {
        case MANOUEVRE::PILF:
        case MANOUEVRE::PIRF:
        case MANOUEVRE::PILR:
        case MANOUEVRE::PIRR:
        {
            // This is tech debt: DeviceControlMode doesn't make sense to be
            // changeable in this subclass.
            DeviceControlMode = TCM::DeviceControlMode::RCParkIn;
            setDeviceControlMode( TCM::DeviceControlMode::NoMode );
        }
        default:                                            break;

    }

    return;
}



void ASPM::setDeviceControlMode( const TCM::DeviceControlMode& mode )
{

    /*  Use of DeviceControlMode is deprecated and should be removed once
    //  testing against a production ASPM is possible.
    */

    if( mode == DeviceControlMode )
    {
        return;
    }

    // // leave per commonly-used state debugging statements.
    // std::cout << "---" << std::endl << "Current ASP value:" << std::endl;
    // std::cout << "DeviceControlMode = " << (int)mode << std::endl;

    MANOUEVRE chosenExploreDir( MANOUEVRE::NADA );

    // This switch statement is used to assign different cases depending on the
    // sequence diagram logic
    switch( DeviceControlMode )
    {

        // NoMode
        case TCM::DeviceControlMode::NoMode: break;

        // RCStartStop
        case TCM::DeviceControlMode::RCStartStop: break;

        // RCDrive
        case TCM::DeviceControlMode::RCDrive: break;

        // RCMainMenu
        case TCM::DeviceControlMode::RCMainMenu:
        {

            /* =================================================================
            * @LG: Code below is for demo purposes only.
            * =============================================================== */
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
            ActiveParkingType = ASP::ActiveParkingType::None;
            ActiveParkingMode = ASP::ActiveParkingMode::None;
            ManeuverStatus = ASP::ManeuverStatus::Scanning;
            ActiveManeuverSide = ASP::ActiveManeuverSide::None;
            ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::None;
            InfoMsg = ASP::InfoMsg::SearchingForSpaces;
            CancelMsg = ASP::CancelMsg::None;
            PauseMsg1 = ASP::PauseMsg1::None;
            PauseMsg2 = ASP::PauseMsg2::None;
            InstructMsg = ASP::InstructMsg::None;
            ParkTypeChangeAvailability = ASP::ParkTypeChangeAvailability::None;
            ManeuverSideAvailability = ASP::ManeuverSideAvailability::None;
            DirectionChangeAvailability = ASP::DirectionChangeAvailability::None;
            ExploreModeAvailability = ASP::ExploreModeAvailability::None;
            ManeuverDirectionAvailability = ASP::ManeuverDirectionAvailability::None;
            ManeuverAlignmentAvailability = ASP::ManeuverAlignmentAvailability::None;
            LongitudinalAdjustAvailability = ASP::LongitudinalAdjustAvailability::None;
            ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;
            ConfirmAvailability = ASP::ConfirmAvailability::None;
            ResumeAvailability = ASP::ResumeAvailability::None;
            LongitudinalAdjustLength = 0;
            ManeuverProgressBar = 0;

            // placeholder to reset ExploreModeSelect
            ExploreModeSelect = TCM::ExploreModeSelect::NotPressed;

            break;

        }

        // RCParkOutSpaceSlctn
        case TCM::DeviceControlMode::RCParkOutSpaceSlctn:
        {
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            // ActiveParkingType = ASP::ActiveParkingType::None;
            // ActiveParkingMode = ASP::ActiveParkingMode::None;
            // ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::None;
            // ActiveManeuverSide = ASP::ActiveManeuverSide::None;

            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ManeuverStatus = ASP::ManeuverStatus::Selecting;
            ConfirmAvailability = ASP::ConfirmAvailability::None;
            ResumeAvailability = ASP::ResumeAvailability::None;
            InstructMsg = ASP::InstructMsg::None;
            ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;

            ManeuverProgressBar = 0;

            break;
        }

        // RCParkIn
        case TCM::DeviceControlMode::RCParkIn:
        {
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
            InstructMsg = ASP::InstructMsg::None;
            CancelMsg = ASP::CancelMsg::None;
            ActiveParkingType = ASP::ActiveParkingType::Remote;
            ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
            ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;

            chosenExploreDir = MANOUEVRE::RTS;

            switch( preferredSimManeuver_ )
            {
                case MANOUEVRE::PILF:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
                    ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
                    oppositeSimManeuver_ = MANOUEVRE::POLR;
                    break;
                }
                case MANOUEVRE::PIRF:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
                    ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
                    oppositeSimManeuver_ = MANOUEVRE::PORR;
                    break;
                }
                case MANOUEVRE::PILR:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
                    ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
                    oppositeSimManeuver_ = MANOUEVRE::POLF;
                    break;
                }
                case MANOUEVRE::PIRR:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
                    ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
                    oppositeSimManeuver_ = MANOUEVRE::PORF;
                    break;
                }
                default:                                    break;
            }

            ManeuverStatus = ASP::ManeuverStatus::Selecting;
            ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
            ResumeAvailability = ASP::ResumeAvailability::None;
            ManeuverProgressBar = 0;

            break;
        }

        // RCAdjust
        case TCM::DeviceControlMode::RCAdjust: break;

        // RCParkOut
        case TCM::DeviceControlMode::RCParkOut:
        {
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
            ActiveParkingType = ASP::ActiveParkingType::Remote;
            InstructMsg = ASP::InstructMsg::None;
            ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
            ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;

            // create stub var to filter forward, reverse, or parallel maneuvers
            // side is irrelevant.
            MANOUEVRE ForwardReverseOrParallel( MANOUEVRE::NADA );
            chosenExploreDir = MANOUEVRE::RTS;

            // check parallel vs perpendicular
            switch( ManeuverTypeSelect )
            {

                // int value of 1 --> Parallel
                case TCM::ManeuverTypeSelect::Parallel:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
                    ForwardReverseOrParallel = MANOUEVRE::POLP;
                    break;
                }

                // int value of 2 --> Perpendicular
                case TCM::ManeuverTypeSelect::Perpendicular:
                {

                    // check rear vs forward
                    switch( ManeuverDirectionSelect )
                    {

                        // int value of 1 --> NoseFirst
                        case TCM::ManeuverDirectionSelect::NoseFirst:
                        {
                            ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
                            ForwardReverseOrParallel = MANOUEVRE::POLF;
                            break;
                        }

                        // int value of 2 --> RearFirst
                        case TCM::ManeuverDirectionSelect::RearFirst:
                        {
                            ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
                            ForwardReverseOrParallel = MANOUEVRE::POLR;
                            break;
                        }

                        // int value of 0 --> None
                        case TCM::ManeuverDirectionSelect::None:
                        case TCM::ManeuverDirectionSelect::InvalidSig:
                        case TCM::ManeuverDirectionSelect::Reserved1:
                        default:
                        {
                            ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::None;
                            break;
                        }
                    }

                    break;
                }

                // int value of 0 --> None
                case TCM::ManeuverTypeSelect::None:
                case TCM::ManeuverTypeSelect::InvalidSig:
                case TCM::ManeuverTypeSelect::Reserved1:
                default:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::None;
                    break;
                }
            }

            switch( ManeuverGearSelect )
            {
                // ActiveManeuverOrientation already parsed via ManeuverTypeSelect
                // this switch statement used to populate oppositeSimManeuver_
                case TCM::ManeuverGearSelect::Forward:
                case TCM::ManeuverGearSelect::Reverse:
                case TCM::ManeuverGearSelect::Park:
                default:                break;
            }

            // Side selection
            switch( ManeuverSideSelect )
            {

                // int value of 0 --> None
                case TCM::ManeuverSideSelect::None:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::None;
                    break;
                }

                // int value of 1 --> Left
                case TCM::ManeuverSideSelect::Left:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::Left;
                    switch( ForwardReverseOrParallel )
                    {
                        case MANOUEVRE::POLF:
                        {
                            oppositeSimManeuver_ = MANOUEVRE::PILR;
                            break;
                        }
                        case MANOUEVRE::POLR:
                        {
                            oppositeSimManeuver_ = MANOUEVRE::PILF;
                            break;
                        }
                        case MANOUEVRE::POLP:
                        case MANOUEVRE::NADA:
                        default:
                        {
                            std::cout << "Error setting oppositeSimManeuver_" << std::endl;
                        }
                    }
                    break;
                }

                // int value of 2 --> Right
                case TCM::ManeuverSideSelect::Right:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::Right;
                    switch( ForwardReverseOrParallel )
                    {
                        case MANOUEVRE::POLF:
                        {
                            oppositeSimManeuver_ = MANOUEVRE::PIRR;
                            break;
                        }
                        case MANOUEVRE::POLR:
                        {
                            oppositeSimManeuver_ = MANOUEVRE::PIRF;
                            break;
                        }
                        case MANOUEVRE::POLP:
                        case MANOUEVRE::NADA:
                        default:
                        {
                            std::cout << "Error setting oppositeSimManeuver_" << std::endl;
                        }
                    }
                    break;
                }

                // int value of 3 --> Center
                case TCM::ManeuverSideSelect::Center:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::Center;
                    break;
                }

                // scaffolding to parse improperly initialized sig if needed
                case TCM::ManeuverSideSelect::InvalidSig:         break;
                default:                                        break;
            }

            ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
            ResumeAvailability = ASP::ResumeAvailability::None;
            ManeuverProgressBar = 0;
            break;
        }

        // RCPushPull
        case TCM::DeviceControlMode::RCPushPull:
        {
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
            ActiveParkingType = ASP::ActiveParkingType::PushPull;
            InstructMsg = ASP::InstructMsg::None;
            ActiveParkingMode = ASP::ActiveParkingMode::None;
            ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;

            // Check check forward or reverse
            switch( ManeuverGearSelect )
            {

                // int value of 0 --> None
                case TCM::ManeuverGearSelect::Park:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::None;
                    break;
                }

                // int value of 1 --> NoseFirst
                case TCM::ManeuverGearSelect::Forward:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
                    chosenExploreDir = MANOUEVRE::SF;
                    oppositeSimManeuver_ = MANOUEVRE::SR;
                    break;
                }

                // int value of 2 --> RearFirst
                case TCM::ManeuverGearSelect::Reverse:
                {
                    ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
                    chosenExploreDir = MANOUEVRE::SR;
                    oppositeSimManeuver_ = MANOUEVRE::SF;
                    break;
                }

                // scaffolding to parse improperly initialized sig if needed
                case TCM::ManeuverGearSelect::InvalidSig:      break;
                case TCM::ManeuverGearSelect::Reserved1:       break;
                default:                                        break;
            }

            // Side selection
            switch( ManeuverSideSelect )
            {

                // int value of 0 --> None
                case TCM::ManeuverSideSelect::None:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::None;
                    break;
                }

                // int value of 1 --> Left
                case TCM::ManeuverSideSelect::Left:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::Left;
                    break;
                }

                // int value of 2 --> Right
                case TCM::ManeuverSideSelect::Right:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::Right;
                    break;
                }

                // int value of 3 --> Center
                case TCM::ManeuverSideSelect::Center:
                {
                    ActiveManeuverSide = ::ASP::ActiveManeuverSide::Center;
                    break;
                }

                // scaffolding to parse improperly initialized sig if needed
                case TCM::ManeuverSideSelect::InvalidSig:         break;
                default:                                        break;
            }

            ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
            ResumeAvailability = ASP::ResumeAvailability::None;
            ManeuverProgressBar = 0;

            break;
        }

        // RCIntMenu
        case TCM::DeviceControlMode::RCIntMenu: break;

        // RCExitMenu
        case TCM::DeviceControlMode::RCExitMenu: break;

        // RCDriveAdv1
        case TCM::DeviceControlMode::RCDriveAdv1: break;

        // 12-15 - Reserved
        case TCM::DeviceControlMode::Reserved4: break;
        case TCM::DeviceControlMode::Reserved3: break;
        case TCM::DeviceControlMode::Reserved2: break;
        case TCM::DeviceControlMode::Reserved1: break;

        default: break;

    }

    if( chosenExploreDir != MANOUEVRE::NADA )
    {
        explore_limit_reached_ = isExploratoryModeDisabled_( chosenExploreDir );
    }

    // leave per commonly-used state debugging statements.
    std::cout << "---" << std::endl << "ASP change confirmation:" << std::endl;
    std::cout << "DeviceControlMode = " << (int)DeviceControlMode << std::endl;


    return;

}


void ASPM::setManeuverButtonPress( const TCM::ManeuverButtonPress& mode )
{
    ManeuverButtonPress = mode;

    // For sim purposes, we do not need to act unless cancel request is passed
    if( mode != TCM::ManeuverButtonPress::CancellationSelected )
    {
        return;
    }

    // For cancellation, wait until the TCM confirms that mode has been set.
    while( ManeuverButtonPress != mode )
    {
        usleep( ASP_REFRESH_RATE );
    }

    // // leave per commonly-used state debugging statements.
    // std::cout << "---" << std::endl << "ASP change confirmation:" << std::endl;
    // std::cout << "ManeuverButtonPress = " << (int)ManeuverButtonPress << std::endl;

    // placeholder to reset ExploreModeSelect
    ExploreModeSelect = TCM::ExploreModeSelect::NotPressed;


    return;
}


void ASPM::setManeuverProgressBar( const uint8_t& percentComplete )
{

    std::lock_guard<std::mutex> lock( getMutex( ) );

    ManeuverProgressBar = percentComplete;
    // // leave per commonly-used state debugging statements.
    // std::cout << "ManeuverProgressBar: " << ManeuverProgressBar << std::endl;

    return;
}


void ASPM::setConnectionApproval( const TCM::ConnectionApproval& mode )
{

    ConnectionApproval = TCM::ConnectionApproval::Reserved1;

    // This switch statement is used to assign different cases depending on the
    // sequence diagram logic.  For FDJ purposes, these signal values are
    // expected; in regular practice, these would be set and referenced at the
    // DCM or ASP level.
    switch( mode )
    {

        case TCM::ConnectionApproval::AllowedDevice:
        {
            InfoMsg = ASP::InfoMsg::RemoteDeviceConnectedAndReady;
        }
        case TCM::ConnectionApproval::NoDevice:
        case TCM::ConnectionApproval::NotAllowedDevice:
        case TCM::ConnectionApproval::Reserved1:
        {
            // This signal is managed at the DCM level so it's set for reference
            // at simulated ASP.
            ConnectionApproval = mode;
        }
        default:        break;

    }

    // Wait until the TCM confirms that ConnectionApproval has been set.
    while( ConnectionApproval != mode )
    {
        usleep( ASP_REFRESH_RATE );
    }

    return;
}


bool ASPM::areWeDoneScanning_( const ASP::ManeuverStatus& status )
{

    if( status == ASP::ManeuverStatus::Scanning )
    {

        if( isTimevalZero( scanStartTime_ ) )
        {
            gettimeofday( &scanStartTime_, NULL );
        }
        else
        {
            if( checkTimeout( scanStartTime_, SIM_SCAN_TIME ) )
            {
                std::cout << "**SIM** Sim vehicle proximity scan complete." << std::endl;
                scanStartTime_ = (struct timeval){0};

                // Once scan is complete, generate placeholder obstacle info
                // which makes sense for StrFwd/StrRvs.
                threatDistanceData.ASPMFrontSegDist1RMT = 4;
                threatDistanceData.ASPMFrontSegDist2RMT = 4;
                threatDistanceData.ASPMFrontSegDist3RMT = 4;
                threatDistanceData.ASPMFrontSegDist4RMT = 4;
                threatDistanceData.ASPMFrontSegDist5RMT = 4;
                threatDistanceData.ASPMFrontSegDist6RMT = 10;
                threatDistanceData.ASPMFrontSegDist7RMT = 19;
                threatDistanceData.ASPMFrontSegDist8RMT = 19;
                threatDistanceData.ASPMFrontSegDist9RMT = 19;
                threatDistanceData.ASPMFrontSegDist10RMT = 19;
                threatDistanceData.ASPMFrontSegDist11RMT = 10;
                threatDistanceData.ASPMFrontSegDist12RMT = 4;
                threatDistanceData.ASPMFrontSegDist13RMT = 4;
                threatDistanceData.ASPMFrontSegDist14RMT = 4;
                threatDistanceData.ASPMFrontSegDist15RMT = 4;
                threatDistanceData.ASPMFrontSegDist16RMT = 4;
                threatDistanceData.ASPMRearSegDist1RMT = 4;
                threatDistanceData.ASPMRearSegDist2RMT = 4;
                threatDistanceData.ASPMRearSegDist3RMT = 4;
                threatDistanceData.ASPMRearSegDist4RMT = 4;
                threatDistanceData.ASPMRearSegDist5RMT = 4;
                threatDistanceData.ASPMRearSegDist6RMT = 10;
                threatDistanceData.ASPMRearSegDist7RMT = 19;
                threatDistanceData.ASPMRearSegDist8RMT = 19;
                threatDistanceData.ASPMRearSegDist9RMT = 19;
                threatDistanceData.ASPMRearSegDist10RMT = 19;
                threatDistanceData.ASPMRearSegDist11RMT = 10;
                threatDistanceData.ASPMRearSegDist12RMT = 4;
                threatDistanceData.ASPMRearSegDist13RMT = 4;
                threatDistanceData.ASPMRearSegDist14RMT = 4;
                threatDistanceData.ASPMRearSegDist15RMT = 4;
                threatDistanceData.ASPMRearSegDist16RMT = 4;
                threatTypeData.ASPMFrontSegType1RMT = 1;
                threatTypeData.ASPMFrontSegType2RMT = 1;
                threatTypeData.ASPMFrontSegType3RMT = 1;
                threatTypeData.ASPMFrontSegType4RMT = 1;
                threatTypeData.ASPMFrontSegType5RMT = 1;
                threatTypeData.ASPMFrontSegType6RMT = 1;
                threatTypeData.ASPMFrontSegType7RMT = 1;
                threatTypeData.ASPMFrontSegType8RMT = 1;
                threatTypeData.ASPMFrontSegType9RMT = 1;
                threatTypeData.ASPMFrontSegType10RMT = 1;
                threatTypeData.ASPMFrontSegType11RMT = 1;
                threatTypeData.ASPMFrontSegType12RMT = 1;
                threatTypeData.ASPMFrontSegType13RMT = 1;
                threatTypeData.ASPMFrontSegType14RMT = 1;
                threatTypeData.ASPMFrontSegType15RMT = 1;
                threatTypeData.ASPMFrontSegType16RMT = 1;
                threatTypeData.ASPMRearSegType1RMT = 1;
                threatTypeData.ASPMRearSegType2RMT = 1;
                threatTypeData.ASPMRearSegType3RMT = 1;
                threatTypeData.ASPMRearSegType4RMT = 1;
                threatTypeData.ASPMRearSegType5RMT = 1;
                threatTypeData.ASPMRearSegType6RMT = 1;
                threatTypeData.ASPMRearSegType7RMT = 1;
                threatTypeData.ASPMRearSegType8RMT = 1;
                threatTypeData.ASPMRearSegType9RMT = 1;
                threatTypeData.ASPMRearSegType10RMT = 1;
                threatTypeData.ASPMRearSegType11RMT = 1;
                threatTypeData.ASPMRearSegType12RMT = 1;
                threatTypeData.ASPMRearSegType13RMT = 1;
                threatTypeData.ASPMRearSegType14RMT = 1;
                threatTypeData.ASPMRearSegType15RMT = 1;
                threatTypeData.ASPMRearSegType16RMT = 1;

                return true;
            }
        }
    }

    return false;

}


bool ASPM::areWeStillManeuvering_( const ASP::ManeuverStatus& status )
{
    // if we are in a paused state then do not transition to Maneuvering
    if(     PauseMsg1 != ASP::PauseMsg1::None ||
            PauseMsg2 != ASP::PauseMsg2::None )
    {
        return false;
    }

    switch( status )
    {

        case ASP::ManeuverStatus::Selecting:
        case ASP::ManeuverStatus::Confirming:
        case ASP::ManeuverStatus::Interrupted:
        {
            // if the ASP has not received a call to resume the maneuver, there
            // is nothing to check check so fall through
            switch( ManeuverButtonPress )
            {
                case TCM::ManeuverButtonPress::ConfirmationSelected:
                {
                    if( ManeuverStatus == ASP::ManeuverStatus::Interrupted ){ break; }
                }
                case TCM::ManeuverButtonPress::ResumeSelected:
                {
                    if( ResumeAvailability != ASP::ResumeAvailability::OfferEnabled ){ break; }
                    ManeuverStatus = ASP::ManeuverStatus::Confirming;
                }
                default:            break;

            }

            // if the ASP has not received a valid gesture, there is no DMH to
            // check so fall through
            if( ManeuverEnableInput != TCM::ManeuverEnableInput::ValidScrnInput )
            {
                break;
            }

            // ** TODO ** if valid, but obstacle --> break
        }

        case ASP::ManeuverStatus::Maneuvering:
        {

            // if valid gesture, reset the gesture flag and restart the DMH by
            // setting the DMH start time to the current time
            if( ManeuverEnableInput == TCM::ManeuverEnableInput::ValidScrnInput )
            {
                ManeuverEnableInput = TCM::ManeuverEnableInput::NoScrnInput;
                gettimeofday( &dmhStartTime_, NULL );
            }

            else
            {
                // if DMH has expired (timeout), reset the DMH start time
                // else, you are still in a maneuvering state
                if( checkTimeout( dmhStartTime_, DMH_TIMEOUT_RATE ) )
                {
                    std::cout << "DMH EXPIRED WHILE AWAITING NEW GESTURE." << std::endl;
                    dmhStartTime_ = (struct timeval){0};
                }

            }

            return true;

        }

        case ASP::ManeuverStatus::Finishing:
        case ASP::ManeuverStatus::Cancelled:
        {
            // // leave per commonly-used state debugging statements.
            // std::cout << "TCM reset via cancel_maneuver!!" << std::endl;
            // std::cout << "---" << std::endl;
        }

        default:    return false;
    }

    return false;
}


bool ASPM::isExploratoryModeDisabled_( const MANOUEVRE& maneuver )
{

    switch( maneuver )
    {
        case MANOUEVRE::SF:
        case MANOUEVRE::SR:
        {
            if( maneuver == lastExploreManeuver_ )
            {
                ++explore_idx_;
                break;
            }
        }
        default:
        {
            lastExploreManeuver_ = maneuver;
            explore_idx_ = 0;
        }
    }

    if( explore_idx_ >= EXPLORATORY_MODE_LIMIT ) { return true; }

    return false;


}


ASP::ActiveManeuverSide ASPM::generateActiveManeuverSide_( )
{
    int signal = rand( ) % 4;

    switch( signal )
    {

        // None
        case 0: return ASP::ActiveManeuverSide::None;

        // Left
        case 1: return ASP::ActiveManeuverSide::Left;

        // Right
        case 2: return ASP::ActiveManeuverSide::Right;

        // Center
        case 3: return ASP::ActiveManeuverSide::Center;

        // None
        default: return ASP::ActiveManeuverSide::None;

    }
}


ASP::ActiveManeuverOrientation ASPM::generateActiveManeuverOrientation_( )
{
    int signal = rand( ) % 4;

    switch( signal )
    {

        // None
        case 0: return ASP::ActiveManeuverOrientation::None;

        // Parallel
        case 1: return ASP::ActiveManeuverOrientation::Parallel;

        // PerpendicularRear
        case 2: return ASP::ActiveManeuverOrientation::PerpendicularRear;

        // PerpendicularFront
        case 3: return ASP::ActiveManeuverOrientation::PerpendicularFront;

        // None
        default: return ASP::ActiveManeuverOrientation::None;

    }
}


ASP::ParkTypeChangeAvailability ASPM::generateParkTypeChangeAvailability_( )
{
    int signal = rand( ) % 2;

    switch( signal )
    {

        // None
        case 0: return ASP::ParkTypeChangeAvailability::None;

        // OfferEnabled
        case 1: return ASP::ParkTypeChangeAvailability::OfferEnabled;

        // OfferDisabled
        default: return ASP::ParkTypeChangeAvailability::OfferDisabled;

    }
}


ASP::ManeuverSideAvailability ASPM::generateManeuverSideAvailability_( )
{
    int signal = rand( ) % 8;

    switch( signal )
    {

        // None
        case 0: return ASP::ManeuverSideAvailability::None;

        // OfferLeftOnly
        case 1: return ASP::ManeuverSideAvailability::OfferLeftOnly;

        // OfferRightOnly
        case 2: return ASP::ManeuverSideAvailability::OfferRightOnly;

        // OfferCenterOnly
        // case 3: return ASP::ManeuverSideAvailability::OfferCenterOnly;

        // OfferLeftAndRight
        case 3: return ASP::ManeuverSideAvailability::OfferLeftAndRight;

        // OfferLeftAndCenter
        // case 5: return ASP::ManeuverSideAvailability::OfferLeftAndCenter;

        // OfferCenterAndRight
        // case 6: return ASP::ManeuverSideAvailability::OfferCenterAndRight;

        // OfferLeftCenterAndRight
        // case 7: return ASP::ManeuverSideAvailability::OfferLeftCenterAndRight;

        // None
        default: return ASP::ManeuverSideAvailability::None;

    }
}


ASP::DirectionChangeAvailability ASPM::generateDirectionChangeAvailability_( )
{
    int signal = rand( ) % 2;

    switch( signal )
    {

        // None
        case 0: return ASP::DirectionChangeAvailability::None;

        // OfferEnabled
        case 1: return ASP::DirectionChangeAvailability::OfferEnabled;

        // OfferDisabled
        default: return ASP::DirectionChangeAvailability::OfferDisabled;

    }
}


ASP::ManeuverAlignmentAvailability ASPM::generateManeuverAlignmentAvailability_( )
{
    int signal = rand( ) % 8;

    switch( signal )
    {

        // None
        case 0: return ASP::ManeuverAlignmentAvailability::None;

        // OfferLeftOnly
        case 1: return ASP::ManeuverAlignmentAvailability::OfferLeftOnly;

        // OfferRightOnly
        case 2: return ASP::ManeuverAlignmentAvailability::OfferRightOnly;

        // OfferCenterOnly
        case 3: return ASP::ManeuverAlignmentAvailability::OfferCenterOnly;

        // OfferLeftAndRight
        case 4: return ASP::ManeuverAlignmentAvailability::OfferLeftAndRight;

        // OfferLeftAndCenter
        case 5: return ASP::ManeuverAlignmentAvailability::OfferLeftAndCenter;

        // OfferCenterAndRight
        case 6: return ASP::ManeuverAlignmentAvailability::OfferCenterAndRight;

        // OfferLeftCenterAndRight
        case 7: return ASP::ManeuverAlignmentAvailability::OfferLeftCenterAndRight;

        // None
        default: return ASP::ManeuverAlignmentAvailability::None;

    }
}


ASP::MobileChallengeSend ASPM::generateMobileChallengeSend_( )
{
    // generate random 24-bit unsigned integer with 8-bit index in MSB
    uint64_t challenge = ((uint64_t)(rand( ) % 0xFFFFFF) << (8 * 4)) | ((uint64_t)challenge_idx_ << (8 * 7));
    pending_challenges_[challenge_idx_++] = challenge;
    return challenge;
}

void ASPM::checkResponse_(TCM::MobileChallengeReply& response)
{
    uint8_t idx = ((uint64_t)response & 0xFF00000000000000) >> (8 * 7);
    uint64_t challenge;
    if(pending_challenges_.find(idx) != pending_challenges_.end()) {
        challenge = pending_challenges_[idx];
    }
    else {
        return; // challenge for this index already passed or was never issued
    }
    uint8_t part1 = ((uint64_t)challenge & 0x00FF000000000000) >> (8 * 6);
    uint8_t part2 = ((uint64_t)challenge & 0x0000FF0000000000) >> (8 * 5);
    uint8_t part3 = ((uint64_t)challenge & 0x000000FF00000000) >> (8 * 4);
    uint64_t expected = (uint64_t)calculateResponse_(part1, part2, part3, idx) << (8 * 4);
    if( expected == (uint64_t)response ) // response matches challenge
    {
        PauseMsg2 = ASP::PauseMsg2::None;
        challenge_warning_ = false;
    }
    else if( challenge_warning_ == false ) // allow one mismatch to pass
    {
        std::cout << "WARNING: initial challenge/response mismatch: expected "
                  << expected << ", received " << (uint64_t)response << std::endl;
        std::cout << "ASP will pause with one subsequent mismatch." << std::endl;
        challenge_warning_ = true;
    }
    else
    {
        std::cout << "FATAL: subsequent challenge/response mismatch: expected "
                  << expected << ", received " << (uint64_t)response << std::endl
                  << "RESTART ASP SIMULATOR TO CONTINUE." << std::endl;
        PauseMsg2 = ASP::PauseMsg2::ChallengeResponseMismatch;
    }
    pending_challenges_.erase(idx);
}

uint32_t ASPM::calculateResponse_(uint8_t part1, uint8_t part2, uint8_t part3, uint8_t idx)
{
    const uint32_t BITS_IN_BYTE = 8;
    uint32_t tempval1 = 0;
    uint32_t tempval2 = 0;
    uint32_t tempval3 = 0;
    uint32_t calresp = 0;

    tempval1 = (uint32_t)((uint32_t)part1 * (uint32_t)part2);
    tempval2 = (uint32_t)(part3 & 0x0F);
    tempval3 = tempval1 >> tempval2;
    tempval1 = (uint32_t)(part3 ^ part1);
    tempval2 = (uint32_t)((uint32_t)part2 - (uint32_t)part1);
    tempval2 = (uint32_t)(tempval2 & (uint32_t)0x07);
    tempval2 = (uint32_t) (tempval1 << tempval2);
    tempval3 = tempval3 + tempval2;
    calresp = (uint32_t) (tempval3/(uint32_t)0x03);
    calresp = (calresp | (((uint32_t)idx) << (BITS_IN_BYTE*3)));
    return calresp;
}


void ASPM::progressBarEventLoop_( )
{
    float progress = 0.1;
    int barWidth = 70;

    while( running_.load( ) )
    {
        switch( ManeuverStatus )
        {
            case ASP::ManeuverStatus::Maneuvering:
            case ASP::ManeuverStatus::Interrupted:
            {

                while( progress < 1.0 )
                {
                    usleep( ASP_REFRESH_RATE );

                    if( progress == ManeuverProgressBar / 100 ) { continue; }

                    std::lock_guard<std::mutex> lock( getMutex( ) );

                    progress = (float) ManeuverProgressBar / 100;

                    std::cout << "[";
                    int pos = barWidth * progress;
                    for( int i = 0; i < barWidth; ++i )
                    {
                        if( i < pos )                   std::cout << "=";
                        else if( i == pos )             std::cout << ">";
                        else                            std::cout << " ";
                    }
                    std::cout << "] " << int( progress * 100.0 ) << " %\r";
                    std::cout.flush( );

                }

                std::cout << std::endl;
                progress = 0.1;
                break;

            }
            default:
            {
                usleep( ASP_REFRESH_RATE );
                break;
            }
        }
    }
}

void ASPM::updateSignalEventLoop_( )
{
    int msgCounter(0);
    std::atomic<bool> isMobileResponseGood;

    uint8_t bufferToASP[ UDP_BUF_MAX ];
    uint8_t bufferToTCM[ UDP_BUF_MAX ];

    while( running_.load( ) )
    {
        // Pass last DeviceControlMode to check for a changed value
        TCM::DeviceControlMode lastDeviceControlMode( DeviceControlMode );
        TCM::MobileChallengeReply lastMobileChallengeReply ( MobileChallengeReply );

        bzero( bufferToASP, UDP_BUF_MAX );
        bzero( bufferToTCM, UDP_BUF_MAX );

        ssize_t bytes_received = socketHandler_.receiveUDP(
                bufferToASP,
                sizeof(bufferToASP) );
    {
        // while updating signals, stop actions on concurrent threads.
        std::lock_guard<std::mutex> lock( getMutex( ) );

        if (bytes_received == TCM_TOTAL_PACKET_SIZE) {
            decodeTCMSignalData(bufferToASP);
        }
        else if (bytes_received == 0) {
            std::cout << "Socket disconnected" << std::endl;
        }
        else {
            std::cout << "ERROR: received malformed UDP packet of length " << bytes_received << std::endl;
        }

        // // leave per commonly-used state debugging statements.
        // std::cout << "---" << std::endl << "Message received.\t";
        // std::cout << "DeviceControlMode = " << (int)DeviceControlMode << std::endl;

        setDeviceControlMode( lastDeviceControlMode );
        if (MobileChallengeReply != lastMobileChallengeReply) {
            checkResponse_(MobileChallengeReply);
        }
        updateSignalsAfterMsgFromTCM_( ++msgCounter );

        uint16_t outBufSize = encodeASPMSignalData(bufferToTCM);

        int sentLen = (int)socketHandler_.sendUDP(
                bufferToTCM,
                outBufSize );

        // Print sent data
        // printf( "** %i-Bytes of UDP data sent **\n", sentLen );
        // for( int k = 0; k < sentLen; ) {
        //     printf( "%02X ", bufferToTCM[k] );
        //     if(++k%20==0) printf("\n");
        // }
        // printf( "\n\n" );
    }
    }

    socketHandler_.disconnectServer( );

    return;
}


void ASPM::updateSignalsAfterMsgFromTCM_( int& msgCount )
{
    // Update signals every 30ms upon receiving new message from TCM.
    // **For LG** the below if statement is included to simulate a vehicle
    // scanning and should be obsoleted and replaced with whatever method(s)
    // used to return ManeuverStatus state.

    // Track whether mobile challenge was sent when DMH was active
    bool challengeNotYetSent( true );

    // Check if the mobile app is requesting a cancellation
    if( ManeuverButtonPress == TCM::ManeuverButtonPress::CancellationSelected )
    {
        // For FDJ purposes, these signal values are expected; in regular
        // practice, these would be set and referenced at the ASP level.
        ManeuverStatus = ASP::ManeuverStatus::Cancelled;
        InfoMsg = ASP::InfoMsg::None;
        CancelMsg = ASP::CancelMsg::ManoeuvreCancelledDriverRequest;
        ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;
        InstructMsg = ASP::InstructMsg::None;

        // maneuver was cancelled and is no longer underway
        isManeuveringWithValidDMH = false;

        // Sequence diagrams are not clear as to what happens upon cancellation,
        // so go ahead and reset everything.
        // ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::NoFeatureActive;
        // ActiveParkingType = ASP::ActiveParkingType::None;
        // ActiveParkingMode = ASP::ActiveParkingMode::None;

        // Sequence diagrams are not clear as to what happens upon cancellation,
        // so go ahead and reset everything.
        // ActiveManeuverSide = ASP::ActiveManeuverSide::None;
        // ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::None;
        // ParkTypeChangeAvailability = ASP::ParkTypeChangeAvailability::None;
        // ManeuverSideAvailability = ASP::ManeuverSideAvailability::None;
        // DirectionChangeAvailability = ASP::DirectionChangeAvailability::None;
        // ExploreModeAvailability = ASP::ExploreModeAvailability::None;
        // ManeuverDirectionAvailability = ASP::ManeuverDirectionAvailability::None;
        // ManeuverAlignmentAvailability = ASP::ManeuverAlignmentAvailability::None;
        // LongitudinalAdjustAvailability = ASP::LongitudinalAdjustAvailability::None;
        // LongitudinalAdjustLength = 0;

        // CancelMsg = 6

        // continue;

        return;

    }
    // check if user is selecting EndManouevre
    else if ( ManeuverButtonPress == TCM::ManeuverButtonPress::EndManouevre )
    {
        ManeuverStatus = ASP::ManeuverStatus::Ended;
        InfoMsg = ASP::InfoMsg::None;
        CancelMsg = ASP::CancelMsg::None;
        ReturnToStartAvailability = ASP::ReturnToStartAvailability::None;
        InstructMsg = ASP::InstructMsg::EngageParkBrake;

        // maneuver has ended and is no longer underway
        isManeuveringWithValidDMH = false;

    }
    // check if user is selecting ReturnToStart
    else if ( ManeuverButtonPress == TCM::ManeuverButtonPress::ReturnToStart )
    {
        setInputManeuverSignals( oppositeSimManeuver_ );
        switch( oppositeSimManeuver_ )
        {
            case MANOUEVRE::POLF:
            case MANOUEVRE::PORF:
            case MANOUEVRE::POLR:
            case MANOUEVRE::PORR:
            case MANOUEVRE::POLP:
            case MANOUEVRE::PORP:
            {
                // This is tech debt: DeviceControlMode doesn't make sense to
                // be changeable in this subclass.
                DeviceControlMode = TCM::DeviceControlMode::RCParkOut;
                setDeviceControlMode( TCM::DeviceControlMode::NoMode );

                return;
            }
            case MANOUEVRE::PILF:
            case MANOUEVRE::PIRF:
            case MANOUEVRE::PILR:
            case MANOUEVRE::PIRR:
            {
                // This is tech debt: DeviceControlMode doesn't make sense to
                // be changeable in this subclass.
                DeviceControlMode = TCM::DeviceControlMode::RCParkIn;
                setDeviceControlMode( TCM::DeviceControlMode::NoMode );

                return;
            }
            case MANOUEVRE::NF:
            case MANOUEVRE::NR:
            case MANOUEVRE::SF:
            case MANOUEVRE::SR:
            case MANOUEVRE::RTS:
            case MANOUEVRE::NADA:
            default:
            {
                std::cout << "Return to Origin not available." << std::endl;
                break;
            }
        }

    }

    // check if a simulated scan is complete and update simulated vars if so
    if( areWeDoneScanning_( ManeuverStatus ) )
    {

        // For FDJ purposes, these signal values are expected; in regular
        // practice, these would be set and referenced at the ASP level.
        ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
        ActiveParkingType = ASP::ActiveParkingType::Remote;
        ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
        ManeuverStatus = ASP::ManeuverStatus::Selecting;
        ManeuverProgressBar = 0;


        switch( preferredSimManeuver_ )
        {
            case MANOUEVRE::POLF:
            case MANOUEVRE::PORF:
            {
                ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
                ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
                ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
                break;
            }
            case MANOUEVRE::POLR:
            case MANOUEVRE::PORR:
            {
                ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
                ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
                ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
                break;
            }
            case MANOUEVRE::POLP:
            case MANOUEVRE::PORP:
            {
                ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
                ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
                ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
                break;
            }
            case MANOUEVRE::PILF:
            case MANOUEVRE::PIRF:
            {
                ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
                ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
                ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
                break;
            }
            case MANOUEVRE::PILR:
            case MANOUEVRE::PIRR:
            {
                ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
                ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
                ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
                break;
            }
            case MANOUEVRE::NF:
            case MANOUEVRE::NR:
            case MANOUEVRE::SF:
            case MANOUEVRE::SR:
            {
                ActiveParkingType = ASP::ActiveParkingType::PushPull;
                ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
                ExploreModeAvailability = ASP::ExploreModeAvailability::OfferEnabled;
                ManeuverDirectionAvailability = ASP::ManeuverDirectionAvailability::OfferForwardAndBackward;
                break;
            }
            case MANOUEVRE::RTS:
            case MANOUEVRE::NADA:
            default:
            {
                ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;

                // For FDJ purposes, Push/Pull is offered:
                ExploreModeAvailability = ASP::ExploreModeAvailability::OfferEnabled;
                ManeuverDirectionAvailability = ASP::ManeuverDirectionAvailability::OfferForwardAndBackward;

                // Generate random maneuvers for FDJ UI
                // ActiveManeuverSide = generateActiveManeuverSide_( );
                // ActiveManeuverOrientation = generateActiveManeuverOrientation_( );
                // ParkTypeChangeAvailability = generateParkTypeChangeAvailability_( );
                // ManeuverSideAvailability = generateManeuverSideAvailability_( );
                // DirectionChangeAvailability = generateDirectionChangeAvailability_( );
                // ManeuverAlignmentAvailability = generateManeuverAlignmentAvailability_( );

                break;
            }
        }

        // For FDJ purposes, RCD, Nudge is not offered:
        RemoteDriveAvailability = ASP::RemoteDriveAvailability::None;
        LongitudinalAdjustAvailability = ASP::LongitudinalAdjustAvailability::OfferDisabled;
        LongitudinalAdjustLength = 0;

        // For FDJ, offer confirm ability (may go unused)
        // ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;

        // For FDJ purposes, these signal values are expected; in regular
        // practice, these would be set and referenced at the ASP level.
        InfoMsg = ASP::InfoMsg::RemoteManeuverReady;

        // maneuver has been initialized but is not currently underway
        isManeuveringWithValidDMH = false;

        // if 3 consecutive exploratory maneuvers have been carried out, then
        // exploratory mode is disabled.
        if( explore_limit_reached_ == true )
        {
            ExploreModeAvailability = ASP::ExploreModeAvailability::OfferDisabled;
        }

    }

    // check if a maneuver is underway or interrupted
    if( areWeStillManeuvering_( ManeuverStatus ) )
    {
        // if the timer has been reset, then we should transition to an
        // interrupted state.
        if( isTimevalZero( dmhStartTime_ ) )
        {
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ManeuverStatus = ASP::ManeuverStatus::Interrupted;
            // ConfirmAvailability= ASP::ConfirmAvailability::OfferEnabled;
            ResumeAvailability = ASP::ResumeAvailability::OfferEnabled;

            // maneuver is no longer underway
            isManeuveringWithValidDMH = false;
        }
        else
        {
            // For FDJ purposes, these signal values are expected; in regular
            // practice, these would be set and referenced at the ASP level.
            ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
            ConfirmAvailability = ASP::ConfirmAvailability::None;
            ResumeAvailability = ASP::ResumeAvailability::None;
            ReturnToStartAvailability = ASP::ReturnToStartAvailability::OfferEnabled;
            InstructMsg = ASP::InstructMsg::RemoteManouevreInProgress;
            if( isProgressHandledExternally_ == false )
            {
                ++ManeuverProgressBar;
            }

            // maneuver remains underway
            isManeuveringWithValidDMH = true;

            // generate new MobileChallengeSend if during DMH
            if( simType_ == SIMULATOR::BENCH )
            {
                MobileChallengeSend = generateMobileChallengeSend_( );
                challengeNotYetSent = false;
            }
        }
    }

    // check if maneuver has completed
    if( ManeuverProgressBar == 100 )
    {
        ManeuverStatus = ASP::ManeuverStatus::Finishing;
        // ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
        ReturnToStartAvailability = ASP::ReturnToStartAvailability::OfferEnabled;
        InstructMsg = ASP::InstructMsg::None;
        InfoMsg = ASP::InfoMsg::None;
        CancelMsg = ASP::CancelMsg::None;

        // maneuver is complete and no longer underway
        isManeuveringWithValidDMH = false;

        // some tech debt to make sure that the DMH stops iterating
        ManeuverEnableInput = TCM::ManeuverEnableInput::NoScrnInput;

    }

    // generate new MobileChallengeSend if counter is 100
    if( challengeNotYetSent && msgCount % 100 == 0 )
    {
        MobileChallengeSend = generateMobileChallengeSend_( );
    }


    return;
}

uint64_t ASPM::getASPMSignal(uint8_t header_id, const int32_t& sigid)
{
    if (header_id == HRD_ID_OF_ASPM_LM) // ID:49
    {
        switch(sigid) {
            // 1
            case ASPM_LM::LMAppConsChkASPM: return 0xB20E;  // 16 bits
            case ASPM_LM::ActiveAutonomousFeature: return (uint64_t)ActiveAutonomousFeature;  // 4 bits
            case ASPM_LM::CancelAvailability: return 0x0;  // 2 bits
            case ASPM_LM::ConfirmAvailability: return (uint64_t)ConfirmAvailability;  // 2 bits
            case ASPM_LM::LongitudinalAdjustAvailability: return (uint64_t)LongitudinalAdjustAvailability;  // 2 bits
            case ASPM_LM::ManeuverDirectionAvailability: return (uint64_t)ManeuverDirectionAvailability;  // 2 bits
            case ASPM_LM::ManeuverSideAvailability: return (uint64_t)ManeuverSideAvailability;  // 2 bits
            case ASPM_LM::ActiveManeuverOrientation: return (uint64_t)ActiveManeuverOrientation;  // 2 bits
            case ASPM_LM::ActiveParkingMode: return (uint64_t)ActiveParkingMode;  // 2 bits
            case ASPM_LM::DirectionChangeAvailability: return (uint64_t)DirectionChangeAvailability;  // 2 bits
            // 11
            case ASPM_LM::ParkTypeChangeAvailability: return (uint64_t)ParkTypeChangeAvailability;  // 2 bits
            case ASPM_LM::ExploreModeAvailability: return (uint64_t)ExploreModeAvailability;  // 2 bits
            case ASPM_LM::ActiveManeuverSide: return (uint64_t)ActiveManeuverSide;  // 2 bits
            case ASPM_LM::ManeuverStatus: return (uint64_t)ManeuverStatus;  // 4 bits
            case ASPM_LM::RemoteDriveOverrideState: return 0x0;  // 2 bits
            case ASPM_LM::ActiveParkingType: return (uint64_t)ActiveParkingType;  // 3 bits
            case ASPM_LM::ResumeAvailability: return (uint64_t)ResumeAvailability;  // 2 bits
            case ASPM_LM::ReturnToStartAvailability: return (uint64_t)ReturnToStartAvailability;  // 2 bits
            case ASPM_LM::NNNNNNNNNN: return 0x0;  // 2 bits, unused
            case ASPM_LM::KeyFobRange: return 0x0;  // 3 bits
            // 21
            case ASPM_LM::LMDviceAliveCntAckRMT: return 0x0;  // 4 bits
            case ASPM_LM::NoFeatureAvailableMsg: return (uint64_t)NoFeatureAvailableMsg;  // 4 bits
            case ASPM_LM::LMFrwdCollSnsType1RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMFrwdCollSnsType2RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMFrwdCollSnsType3RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMFrwdCollSnsType4RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMFrwdCollSnsZone1RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMFrwdCollSnsZone2RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMFrwdCollSnsZone3RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMFrwdCollSnsZone4RMT: return 0x0;  // 4 bits
            // 31
            case ASPM_LM::InfoMsg: return (uint64_t)InfoMsg;  // 6 bits
            case ASPM_LM::InstructMsg: return (uint64_t)InstructMsg;  // 5 bits
            case ASPM_LM::LateralControlInfo: return 0x0;  // 3 bits
            case ASPM_LM::LongitudinalAdjustLength: return (uint64_t)LongitudinalAdjustLength;  // 10 bits
            case ASPM_LM::LongitudinalControlInfo: return 0x0;  // 3 bits
            case ASPM_LM::ManeuverAlignmentAvailability: return (uint64_t)ManeuverAlignmentAvailability;  // 3 bits
            case ASPM_LM::RemoteDriveAvailability: return (uint64_t)RemoteDriveAvailability;  // 2 bits
            case ASPM_LM::PauseMsg2: return (uint64_t)PauseMsg2;  // 4 bits
            case ASPM_LM::PauseMsg1: return (uint64_t)PauseMsg1;  // 4 bits
            case ASPM_LM::LMRearCollSnsType1RMT: return 0x0;  // 1 bit
            // 41
            case ASPM_LM::LMRearCollSnsType2RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMRearCollSnsType3RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMRearCollSnsType4RMT: return 0x0;  // 1 bit
            case ASPM_LM::LMRearCollSnsZone1RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMRearCollSnsZone2RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMRearCollSnsZone3RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMRearCollSnsZone4RMT: return 0x0;  // 4 bits
            case ASPM_LM::LMRemoteFeatrReadyRMT: return 0x0;  // 2 bits
            case ASPM_LM::CancelMsg: return (uint64_t)CancelMsg;  // 4 bits
            case ASPM_LM::LMVehMaxRmteVLimRMT: return 0x0;  // 6 bits
            // 51
            case ASPM_LM::ManueverPopupDisplay: return 0x0;  // 1 bit
            case ASPM_LM::ManeuverProgressBar: return (uint64_t)ManeuverProgressBar;  // 7 bits
            case ASPM_LM::MobileChallengeSend: return (uint64_t)MobileChallengeSend;  // 64 bits
            case ASPM_LM::LMRemoteResponseASPM: return 0x1234567890abcdef;  // 64 bits
            default:
                printf("ERROR: received unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_LM_ObjSegment) // ID:73
    {
        switch(sigid) {
            // 1
            case ASPM_LM_ObjSegment::ASPMXXXXX: return 0x0;  // 1 bit, unused
            case ASPM_LM_ObjSegment::ASPMFrontSegType1RMT: return (uint64_t)threatTypeData.ASPMFrontSegType1RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist1RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist1RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType2RMT: return (uint64_t)threatTypeData.ASPMFrontSegType2RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist2RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist2RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType3RMT: return (uint64_t)threatTypeData.ASPMFrontSegType3RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist3RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist3RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType4RMT: return (uint64_t)threatTypeData.ASPMFrontSegType4RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist4RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist4RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType5RMT: return (uint64_t)threatTypeData.ASPMFrontSegType5RMT;  // 2 bits
            // 11
            case ASPM_LM_ObjSegment::ASPMFrontSegDist5RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist5RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType6RMT: return (uint64_t)threatTypeData.ASPMFrontSegType6RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist6RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist6RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType7RMT: return (uint64_t)threatTypeData.ASPMFrontSegType7RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist7RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist7RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType8RMT: return (uint64_t)threatTypeData.ASPMFrontSegType8RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist8RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist8RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType9RMT: return (uint64_t)threatTypeData.ASPMFrontSegType9RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist9RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist9RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType10RMT: return (uint64_t)threatTypeData.ASPMFrontSegType10RMT;  // 2 bits
            // 21
            case ASPM_LM_ObjSegment::ASPMFrontSegDist10RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist10RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType11RMT: return (uint64_t)threatTypeData.ASPMFrontSegType11RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist11RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist11RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType12RMT: return (uint64_t)threatTypeData.ASPMFrontSegType12RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist12RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist12RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType13RMT: return (uint64_t)threatTypeData.ASPMFrontSegType13RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist13RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist13RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType14RMT: return (uint64_t)threatTypeData.ASPMFrontSegType14RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist14RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist14RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType15RMT: return (uint64_t)threatTypeData.ASPMFrontSegType15RMT;  // 2 bits
            // 31
            case ASPM_LM_ObjSegment::ASPMFrontSegDist15RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist15RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegType16RMT: return (uint64_t)threatTypeData.ASPMFrontSegType16RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMFrontSegDist16RMT: return (uint64_t)threatDistanceData.ASPMFrontSegDist16RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType1RMT: return (uint64_t)threatTypeData.ASPMRearSegType1RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist1RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist1RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType2RMT: return (uint64_t)threatTypeData.ASPMRearSegType2RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist2RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist2RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType3RMT: return (uint64_t)threatTypeData.ASPMRearSegType3RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist3RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist3RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType4RMT: return (uint64_t)threatTypeData.ASPMRearSegType4RMT;  // 2 bits
            // 41
            case ASPM_LM_ObjSegment::ASPMRearSegDist4RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist4RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType5RMT: return (uint64_t)threatTypeData.ASPMRearSegType5RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist5RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist5RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType6RMT: return (uint64_t)threatTypeData.ASPMRearSegType6RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist6RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist6RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType7RMT: return (uint64_t)threatTypeData.ASPMRearSegType7RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist7RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist7RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType8RMT: return (uint64_t)threatTypeData.ASPMRearSegType8RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist8RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist8RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType9RMT: return (uint64_t)threatTypeData.ASPMRearSegType9RMT;  // 2 bits
            // 51
            case ASPM_LM_ObjSegment::ASPMRearSegDist9RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist9RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType10RMT: return (uint64_t)threatTypeData.ASPMRearSegType10RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist10RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist10RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType11RMT: return (uint64_t)threatTypeData.ASPMRearSegType11RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist11RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist11RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType12RMT: return (uint64_t)threatTypeData.ASPMRearSegType12RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist12RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist12RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType13RMT: return (uint64_t)threatTypeData.ASPMRearSegType13RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist13RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist13RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType14RMT: return (uint64_t)threatTypeData.ASPMRearSegType14RMT;  // 2 bits
            // 61
            case ASPM_LM_ObjSegment::ASPMRearSegDist14RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist14RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType15RMT: return (uint64_t)threatTypeData.ASPMRearSegType15RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist15RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist15RMT;  // 5 bits
            case ASPM_LM_ObjSegment::ASPMRearSegType16RMT: return (uint64_t)threatTypeData.ASPMRearSegType16RMT;  // 2 bits
            case ASPM_LM_ObjSegment::ASPMRearSegDist16RMT: return (uint64_t)threatDistanceData.ASPMRearSegDist16RMT;  // 5 bits
            default:
                printf("ERROR: received unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_RemoteTarget) // ID:50
    {
        switch(sigid) {
            // 1
            case ASPM_RemoteTarget::TCMRemoteTarget: return 0x9876543210FEDCBA;  // 64 bits
            default:
                printf("ERROR: received unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_LM_Session) // ID:66
    {
        switch(sigid) {
            // 1
            case ASPM_LM_Session::LMEncrptSessionCntASPM_1: return 0xfedcba9876543210;  // 64 bits
            case ASPM_LM_Session::LMEncrptSessionCntASPM_2: return 0x1234567890abcdef;  // 64 bits
            case ASPM_LM_Session::LMEncryptSessionIDASPM_1: return 0xfedcba9876543210;  // 64 bits
            case ASPM_LM_Session::LMEncryptSessionIDASPM_2: return 0x1234567890abcdef;  // 64 bits
            case ASPM_LM_Session::LMTruncMACASPM: return 0xfedcba9876543210;  // 64 bits
            case ASPM_LM_Session::LMTruncSessionCntASPM: return 0xAB;  // 8 bits
            case ASPM_LM_Session::LMSessionControlASPM: return 0x0;  // 3 bits
            case ASPM_LM_Session::LMSessionControlASPMExt: return 0x0;  // 5 bits
            default:
                printf("ERROR: received unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_LM_Trunc) // ID:76
    {
        switch(sigid) {
            // 1
            case ASPM_LM_Trunc::LMTruncEnPsPrasRotASPM_1: return 0xfedcba9876543210;  // 64 bits
            case ASPM_LM_Trunc::LMTruncEnPsPrasRotASPM_2: return 0x1234567890abcdef;  // 64 bits
            default:
                printf("ERROR: received unknown signal:%d", sigid);
                return 0;
        }
    }
    else {
        printf("ERROR: invalid header_id: %d", header_id);
        return 0x0;
    }
}

void ASPM::setTCMSignal(uint8_t header_id, const int16_t& sigid, uint64_t value)
{
    //printf("Header: %d, Sig: %d, Val: %lx\n", header_id, sigid, value);
    if (header_id == HRD_ID_OF_TCM_LM) {
        switch(sigid) {
            // 1
            case TCM_LM::AppCalcCheck: AppCalcCheck = (TCM::AppCalcCheck)value; return;
            case TCM_LM::LMDviceAliveCntRMT: return;
            case TCM_LM::ManeuverEnableInput: ManeuverEnableInput = (TCM::ManeuverEnableInput)value; return;
            case TCM_LM::ManeuverGearSelect: ManeuverGearSelect = (TCM::ManeuverGearSelect)value; return;
            case TCM_LM::NudgeSelect: NudgeSelect = (TCM::NudgeSelect)value; return;
            case TCM_LM::RCDOvrrdReqRMT: return;
            case TCM_LM::AppSliderPosY: AppSliderPosY = (TCM::AppSliderPosY)value; return;
            case TCM_LM::AppSliderPosX: AppSliderPosX = (TCM::AppSliderPosX)value; return;
            case TCM_LM::RCDSpeedChngReqRMT: return;
            case TCM_LM::RCDSteWhlChngReqRMT: return;
            // 11
            case TCM_LM::ConnectionApproval: ConnectionApproval = (TCM::ConnectionApproval)value; return;
            case TCM_LM::ManeuverButtonPress: ManeuverButtonPress = (TCM::ManeuverButtonPress)value; return;
            case TCM_LM::DeviceControlMode: DeviceControlMode = (TCM::DeviceControlMode)value; return;
            case TCM_LM::ManeuverTypeSelect: ManeuverTypeSelect = (TCM::ManeuverTypeSelect)value; return;
            case TCM_LM::ManeuverDirectionSelect: ManeuverDirectionSelect = (TCM::ManeuverDirectionSelect)value; return;
            case TCM_LM::ExploreModeSelect: ExploreModeSelect = (TCM::ExploreModeSelect)value; return;
            case TCM_LM::RemoteDeviceBatteryLevel: RemoteDeviceBatteryLevel = (TCM::RemoteDeviceBatteryLevel)value; return;
            case TCM_LM::PairedWKeyId: return;
            case TCM_LM::ManeuverSideSelect: ManeuverSideSelect = (TCM::ManeuverSideSelect)value; return;
            case TCM_LM::LMDviceRngeDistRMT: return;
            // 21
            case TCM_LM::TTTTTTTTTT: return;
            case TCM_LM::LMRemoteChallengeVDC: return;
            case TCM_LM::MobileChallengeReply: MobileChallengeReply = (TCM::MobileChallengeReply)value; return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_Session) {
        switch(sigid) {
            // 1
            case TCM_LM_Session::LMEncrptSessionCntVDC_1: return;
            case TCM_LM_Session::LMEncrptSessionCntVDC_2: return;
            case TCM_LM_Session::LMEncryptSessionIDVDC_1: return;
            case TCM_LM_Session::LMEncryptSessionIDVDC_2: return;
            case TCM_LM_Session::LMTruncMACVDC: return;
            case TCM_LM_Session::LMTruncSessionCntVDC: return;
            case TCM_LM_Session::LMSessionControlVDC: return;
            case TCM_LM_Session::LMSessionControlVDCExt: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_RemoteControl) {
        switch(sigid) {
            // 1
            case TCM_RemoteControl::TCMRemoteControl: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_TransportKey) {
        switch(sigid) {
            // 1
            case TCM_TransportKey::LMSessionKeyIDVDC: return;
            case TCM_TransportKey::LMHashEnTrnsportKeyVDC: return;
            case TCM_TransportKey::LMEncTransportKeyVDC_1: return;
            case TCM_TransportKey::LMEncTransportKeyVDC_2: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_App) {
        switch(sigid) {
            // 1
            case TCM_LM_App::LMAppTimeStampRMT: return;
            case TCM_LM_App::AppAccelerationX: return;
            case TCM_LM_App::AppAccelerationY: return;
            case TCM_LM_App::AppAccelerationZ: AppAccelerationZ = (TCM::AppAccelerationZ)value; return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyID) {
        switch(sigid) {
            // 1
            case TCM_LM_KeyID::NOT_USED_ONE_BIT: return;
            case TCM_LM_KeyID::LMRotKeyChkACKVDC: return;
            case TCM_LM_KeyID::LMSessionKeyIDVDCExt: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyAlpha) {
        switch(sigid) {
            // 1
            case TCM_LM_KeyAlpha::LMHashEnRotKeyAlphaVDC: return;
            case TCM_LM_KeyAlpha::LMEncRotKeyAlphaVDC_1: return;
            case TCM_LM_KeyAlpha::LMEncRotKeyAlphaVDC_2: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyBeta) {
        switch(sigid) {
            // 1
            case TCM_LM_KeyBeta::LMHashEncRotKeyBetaVDC: return;
            case TCM_LM_KeyBeta::LMEncRotKeyBetaVDC_1: return;
            case TCM_LM_KeyBeta::LMEncRotKeyBetaVDC_2: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyGamma) {
        switch(sigid) {
            // 1
            case TCM_LM_KeyGamma::LMHashEncRotKeyGamaVDC: return;
            case TCM_LM_KeyGamma::LMEncRotKeyGammaVDC_1: return;
            case TCM_LM_KeyGamma::LMEncRotKeyGammaVDC_2: return;
            default:
                printf("ERROR: received unknown signal: %d", sigid);
                return;
        }
    }
    else {
        printf("ERROR: invalid header_id: %d", header_id);
    }
}

uint16_t ASPM::encodeASPMSignalData(uint8_t* buffer)
{
    int8_t ii, jj, kk = 0;
    uint16_t size_total = 0;
    uint8_t* curr_packet = buffer;
    uint8_t length = 0;
    uint8_t length_new = 0;
    uint8_t currValue = 0;
    int8_t loop = 0;
    int8_t startBit = 0;
    int8_t remainBits = 0;

    int pdu_header[NUMBER_OF_ASPM_PDU] = {HRD_ID_OF_ASPM_LM, HRD_ID_OF_ASPM_RemoteTarget, HRD_ID_OF_ASPM_LM_Session,
                                          HRD_ID_OF_ASPM_LM_ObjSegment, HRD_ID_OF_ASPM_LM_Trunc};

    memset(buffer, 0x00, UDP_BUF_MAX);

    for (kk = 0; kk<NUMBER_OF_ASPM_PDU ; ++kk) {
        uint16_t index = 0;

        std::shared_ptr<SomePacket> packet = std::make_shared<SomePacket>(reinterpret_cast<char*>(curr_packet), UdpPacketType::SendASPMPacket);
        packet->putHeaderID(pdu_header[kk]);

        char* data = (char *)packet->getPayloadStartAddress();
        std::vector<std::shared_ptr<LMSignalInfo>>& vt_signal = get_ASP_vector(pdu_header[kk]);

        for (ii=0; ii < vt_signal.size(); ii++)
        {
            length = vt_signal.at(ii)->getBitLength();
            loop = (length-1)/8 ;

            for (jj = loop; jj >= 0; jj--)
            {
                unsigned long int x = 0xff; // 0xFF was int value in original code, this is not allowed to shift as int's max bit. So 0xFF should be defined as 8bytes(64bits)
                currValue = ( (getASPMSignal(packet->getHeaderID(), vt_signal.at(ii)->getIndex()) ) & (x << (8 * jj))) >> (8 * jj);
                if (length > 8) {
                    length_new = length - (8 * jj);
                    length -= length_new;
                } else {
                    length_new = length;
                    currValue = (currValue & ((1 << length_new)-1));
                }
                remainBits = 8 - startBit;

                if (length_new <= remainBits) {
                    data[index] |= currValue << (remainBits - length_new);
                    startBit = (startBit + length_new) % 8;
                    if (startBit == 0) index++;
                } else {
                    startBit = length_new - remainBits;
                    data[index] |= currValue >> startBit;
                    data[++index] |= currValue << (8 - startBit);
                }
            }
        }

        uint16_t move_len = sizeof(pdu_header_t) + packet->getPayloadLength();
        size_total += move_len;
        curr_packet += move_len;
    }

    return size_total;
}

void ASPM::decodeTCMSignalData(uint8_t* buffer)
{
    int8_t ii = 0, jj = 0;
    uint8_t length = 0;
    uint8_t bits_read = 0, bits_to_read = 0;
    uint8_t lmask = 0, rmask = 0, value_byte = 0;
    uint8_t bits_remaining = 0;
    uint64_t value = 0;
    uint8_t* curr_packet = buffer;
    uint16_t index = 0;

    for (ii = 0; ii < NUMBER_OF_TCM_PDU; ++ii) {
        std::shared_ptr<SomePacket> packet = std::make_shared<SomePacket>(reinterpret_cast<char*>(curr_packet), UdpPacketType::ReadTCMPacket);
        char* data = (char *)packet->getPayloadStartAddress();
        index = 0;
        std::vector<std::shared_ptr<LMSignalInfo>>& vt_signal = get_TCM_vector(packet->getHeaderID());
        for (jj=0; jj < vt_signal.size(); ++jj) {
            length = vt_signal.at(jj)->getBitLength();
            bits_read = value = 0;
            while(bits_read < length) {
                if (bits_remaining) {
                    lmask = (uint8_t)0xFF >> (8 - bits_remaining);
                    bits_to_read = std::min((uint8_t)(length - bits_read), bits_remaining);
                    rmask = (uint8_t)0xFF << (bits_remaining - bits_to_read);
                    bits_remaining -= bits_to_read;
                    value_byte = (data[index] & lmask & rmask) >> bits_remaining;
                }
                else {
                    bits_to_read = std::min((uint8_t)(length - bits_read), (uint8_t)8);
                    rmask = (uint8_t)0xFF << (8 - bits_to_read);
                    bits_remaining = 8 - bits_to_read;
                    value_byte = (data[index] & rmask) >> bits_remaining;
                }
                bits_read += bits_to_read;
                value |= (uint64_t)value_byte << (length - bits_read);
                if (!bits_remaining) {
                    ++index;
                }
            }
            setTCMSignal(packet->getHeaderID(), vt_signal[jj]->getIndex(), value);
        }
        curr_packet += sizeof(pdu_header_t) + packet->getPayloadLength();
    }
}
