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
 * \file Class definition for \p JsonTemplateHandler.
 *
 * \author fdaniel, trice2
 */

#include "templatehandler.hpp"

TemplateHandler::TemplateHandler( )
{

    rawHeaderJSON_ = {
        {"group", "group_name"},
    };

    rawAvailableManeuversJSON_ = {
        {"default", "ManeuverCode"},
        {"continue_exploring", false},
        { "maneuvers", {
            {"OutLftFwd", false},
            {"OutLftRvs", false},
            {"OutRgtFwd", false},
            {"OutRgtRvs", false},
            {"InLftFwd", false},
            {"InLftRvs", false},
            {"InRgtFwd", false},
            {"InRgtRvs", false},
            {"NdgFwd", false},
            {"NdgRvs", false},
            {"StrFwd", false},
            {"StrRvs", false},
            {"OutLftPrl", false},
            {"OutRgtPrl", false},
            {"RtnToOgn", false},
        } }
    };

    rawManeuverStatusJSON_ = {
        {"maneuver", "ManeuverCode"},
        {"progress", 0},
        {"status", 999}
    };

    rawThreatDataJSON_ = {
        {"threats", {5,5,8,10,19,19,19,19,15,10,11,12,13,14,10,10,10,18,19,19,19,19,19,19,19,19,5,5,5,5,5,5}}
    };

    rawVehicleAPIVersionJSON_ = {
        {"api_version", "v.xxx.xxx.xxx"}
    };

    rawVehicleInitJSON_ = {
        {"ready", false},
        {"active_maneuver", false}
    };

    rawVehicleStatusJSON_ =
    {
        { "status_1xx", {
            {"status_code", 199},
            {"status_text", "[any additional details]"},
        }},
        { "status_2xx", {
            {"status_code", 299},
            {"status_text", "[any additional details]"},
        }},
        { "status_3xx", {
            {"status_code", 399},
            {"status_text", "[any additional details]"},
        }},
        {"status_4xx", {
            {"status_code", 499},
            {"status_text", "[any additional details]"},
        }},
        {"status_5xx", {
            {"status_code", 599},
            {"status_text", "[any additional details]"},
        }},
        {"status_6xx", {
            {"status_code", 699},
            {"status_text", "[any additional details]"},
        }},
        {"status_7xx", {
            {"status_code", 799},
            {"status_text", "[any additional details]"},
        }},
        {"status_8xx", {
            {"status_code", 899},
            {"status_text", "[any additional details]"},
        }},
        {"status_9xx", {
            {"status_code", 999},
            {"status_text", "[any additional details]"},
        }}
    };

    rawMobileChallengeJSON_ = {
        {"packed_bytes", "999999999999999999"}
    };

    rawMobileResponseJSON_ = {
        {"response_to_challenge", "999999999999999999"}
    };

    rawDeadmansHandleJSON_ = {
        {"enable_vehicle_motion", false},
        {"dmh_gesture_progress", 123},
        {"dmh_horizontal_touch", 1234.4321},
        {"dmh_vertical_touch", 2468.8642},
        {"crc_value", 56789}
    };

    rawManeuverInitJSON_ = {
        {"maneuver", "ManeuverName"}
    };

    rawMobileInitJSON_ = {
        {"terms_accepted", false}
    };

    rawSendPINJSON_ = {
        {"pin", "[PIN]"}
    };

    rawCabinCommandsJSON_ = {
        {"engine_off", false},
        {"doors_locked", false}
    };

    rawCabinStatusJSON_ = {
        {"power_status", 99},
        {"door_open_driver", false},
        {"door_open_passenger", false},
        {"door_open_lr", false},
        {"door_open_rr", false},
        {"door_open_tailgate", false},
        {"door_open_engine_hood", false},
        {"lock_status", 99}
    };
}

const char * rawTextManeuverStatus = "\
NotActive \
Scanning \
Selecting \
Confirming \
Manoeuvring \
Interrupted \
Finishing \
Ended \
RCStartStop \
Holding \
Cancelled \
RESERVED4 \
RESERVED3 \
RESERVED2 \
RESERVED1 \
RESERVED \
";

const char * rawTextNoFeatureAvailableMsg = "\
None \
NotAvailableSystemFault \
NotAvailableSensorBlocked \
NotAvailableVehicleNotStarted \
NotAvailableTiltTooStrong \
NotAvailableTrailerConnected \
NotAvailableRideHeight \
NotAvailableSpeedToohigh \
NotAvailableATPC \
NotAvailableTowAssistOn \
NotAvailableWadeAssistOn \
NotAvailableACCOn \
NotAvailableTJPOn \
NotAvailableVehicleOnMotorway \
Reserved2 \
Reserved1 \
";

const char * rawTextCancelMsg = "\
None \
CancelledFromLossOfTraction \
CancelledMaxNumMovesReached \
CancelledPausedForTooLong \
CancelledInternalSystemFailure \
CancelledTrailerConnected \
ManoeuvreCancelledDriverRequest \
ManoeuvreCancelledVehicleDrivenOn \
CancelledSpeedTooHigh \
CancelledVehicleInMotorway \
Reserved6 \
Reserved5 \
Reserved4 \
Reserved3 \
Reserved2 \
Reserved1 \
";

const char * rawTextPauseMsg1 = "\
None \
PausedDriverReq \
PausedSteeringIntervention \
PausedManualGearChange \
PausedDriverBraked \
PausedParkBrake \
PausedAcceleratorPressed \
PausedEngineStalled \
PausedDoorOpen \
PausedBootOpen \
PausedBonnetOpen \
PausedObstacleDetected \
PauseRideHeightChanged \
PausedSensorPerformance \
RemoteCommunicationLost \
PausedPowerLow \
";

const char * rawTextPauseMsg2 = "\
None \
ActivityKeyInVehicle \
ActivityKeyOutsideLegalDistance \
ActivityKeyDistanceIndeterminate \
ActivityKeyMissing \
MaximumDistanceReached \
MaximumDurationForOperationReached \
ApplicationCRCFailure \
ChallengeResponseMismatch \
DMHInvalid \
TemporarySystemFailure \
";

const char * rawTextLMInfoRMT = "\
None \
SearchingForSpaces \
SlowDownSearchForSpaces \
SlowDownViewSpaces \
DriveForwardSpaceSearch \
MoveSuspensionToNormalHeight \
SpaceTooSmall \
NarrowSpaceAvailableThroughRemoteOnly \
SpaceOccupied \
CantManeuverIntoSpace \
BringVehicleToRestAndApplyBrake \
ConfirmAndReleaseBrakeToStart \
ReleaseBrakeToStart \
MoveIndicatorToChangeSelectionSide \
NoAvailableParkOut \
NotEnoughSpaceToParkOut \
SelectParkOutManeuver \
RemoteDeviceConnectedAndReady \
RemoteDeviceBatteryTooLow \
RemoteManeuverReady \
DriverMustBeOutsideOfVehicle \
PowerReservesLow \
MindOtherRoadUsersDisclaimer \
EngageReverseToStartManeuver \
ApproachingLegalDistanceLimit \
ApproachingMaximumDistance \
ApproachingMaximumDistanceForOperation \
";

const char * rawTextInstructMsg = "\
None \
BringVehicleToRest \
ReleaseBrakesToStart \
SelectR \
SelectD \
SelectFirstGear \
DeselctRtoDisplaySpaces \
DriveForward \
ContinueForward \
DriveBackward \
ContinueBackward \
Stop \
EngageParkBrake \
MonitorManouevreInProgress \
RemoteManouevreInProgress \
VehicleStopping \
VehicleStopped \
SystemOperationRestrictedCapabilityReached \
SystemOperationRestrictedOccupantMovement \
PressAcceleratorToResume \
PauseManuouevre \
PowerReservesLow \
MindOtherRoadUsersDisclaimer \
EngageReverseToStartManeuver \
ApproachingLegalDistanceLimit \
ApproachingMaximumDistance \
ApproachingMaximumDistanceForOperation \
";

const char * rawAcknowledgeRemotePIN = "\
None \
IncorrectPIN \
IncorrectPIN3xLock60s \
IncorrectPIN3xLock300s \
IncorrectPIN3xLock3600s \
IncorrectPIN3xLockIndefinite \
ExpiredPIN \
NotSetInDCM \
CorrectPIN \
";

const char * rawErrorMsg = "\
None \
ElectricChargerConnected \
StolenVehicleTrackingAlert \
RemoteStartMaxAttemptsReached \
VehicleCrashDetected \
LowFuelWarning \
ErrorFromCCM \
MultipleKeyFobsDetected \
LowBatteryWarning \
RemoteSessionExpirationWarning \
Reserved9 \
Reserved8 \
Reserved7 \
Reserved6 \
Reserved5 \
Reserved4 \
Reserved3 \
Reserved2 \
Reserved1 \
Reserved0 \
WiFiAdvertisingTimeoutIn30sec \
WiFiAdvertisingTimeoutIn1min \
WiFiAdvertisingTimeoutIn2min \
WiFiAdvertisingTimeoutIn3min \
WiFiAdvertisingTimeoutIn4min \
WiFiAdvertisingTimeoutIn5min \
";
