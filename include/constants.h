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
 * \file Centralized repository for all project constants, ASP, TCM, and DCM
 * class definitions.
 *
 * \author fdaniel, trice2
 */

// https://docs.google.com/spreadsheets/d/1efMraNG8MAilQG4Vwp-ifn3cpIAV3bN4fF4vKh1GHsM/edit#gid=0
#include <stdint.h>

#if !defined( CONSTANTS_H )
#define CONSTANTS_H

// hold the current API version here.
constexpr auto API_DOC_VERSION = "0.0.17";

// Limit subsequent exploratory mode maneuver executions per [REQ NAME HERE]
constexpr auto EXPLORATORY_MODE_LIMIT = 3;

// ASPM cycle rate per [REQ NAME HERE]
constexpr auto ASP_REFRESH_RATE = 30000;                    // μs,
constexpr auto MAX_REFRESH_RATE = 10000000;                 // μs,
constexpr auto DMH_TIMEOUT_RATE = 3 * ASP_REFRESH_RATE;     // μs,
constexpr auto TCM_TIMEOUT_RATE = 60000000;                 // μs,
constexpr auto BUTTON_TIMEOUT_RATE = 240000;                // μs,

// list of supported maneuvers as enum for reference in the codebase
typedef enum
{
    POLF,                   // Park Out Left Forward
    POLR,                   // Park Out Left Reverse
    PORF,                   // Park Out Right Forward
    PORR,                   // Park Out Right Reverse
    POLP,                   // Park Out Left Parallel
    PORP,                   // Park Out Right Parallel
    PILF,                   // Park In Left Forward
    PILR,                   // Park In Left Reverse
    PIRF,                   // Park In Right Forward
    PIRR,                   // Park In Right Reverse
    PILP,                   // Park In Left Parallel
    PIRP,                   // Park In Right Parallel
    NF,                     // Nudge Forward
    NR,                     // Nudge Reverse
    SF,                     // Explore Forward
    SR,                     // Explore Reverse
    RTS,                    // Return to Origin
    NADA                    // Nothing
} MANOUEVRE;


/*!
 * prefixes for all sent status codes IAW
 * <a href="./res/vehiclestatuscodes.md">\p vehiclestatuscodes.md</a>.
 */
enum class VehicleStatusPrefix : int
{
    NoFeatureAvailableMsg = 100,                /**<1xx */
    CancelMsg = 200,                            /**<2xx */
    PauseMsg1 = 300,                            /**<3xx */
    PauseMsg2 = 400,                            /**<4xx */
    InfoMsg = 500,                              /**<5xx */
    InstructMsg = 600,                          /**<6xx */
    AcknowledgeRemotePIN = 700,                 /**<7xx */
    ErrorMsg = 800,                             /**<8xx */
    ManeuverStatus = 900                        /**<9xx */
};


/*!
 * Signals that originate from TCM to ASP
 */
class TCM
{

public:

    /*
    *** WRITE SIGNALS FROM MOBILE TO ASP ***
    */

    // Remote Device control mode; pass through to the ASPM
    enum class DeviceControlMode : uint8_t
    {
        InvalidSig = 255,
        NoMode = 0,
        RCStartStop = 1,
        RCDrive = 2,
        RCMainMenu = 3,
        RCParkOutSpaceSlctn = 4,
        RCParkIn = 5,
        RCAdjust = 6,
        RCParkOut = 7,
        RCPushPull = 8,
        RCIntMenu = 9,
        RCExitMenu = 10,
        RCDriveAdv1 = 11,
        Reserved4 = 12,
        Reserved3 = 13,
        Reserved2 = 14,
        Reserved1 = 15
    };

    // Feature Maneuver state Selection; pass through to the ASPM
    enum class ManeuverButtonPress : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0  */
        ConfirmationSelected = 1,                   /**< 1  */
        CancellationSelected = 2,                   /**< 2  */
        ResumeSelected = 3,                         /**< 3  */
        ReturnToStart = 4,                          /**< 4  */
        NudgeSelected = 5,                          /**< 5  */
        EndManouevre = 6,                           /**< 6  */
        ContinueManouevre = 7,                      /**< 7  */
    };

    // Parking Side selection through remote; pass through to the ASPM
    enum class ManeuverTypeSelect : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        Parallel = 1,
        Perpendicular = 2,
        Reserved1 = 3
    };

    // Nose In or Nose Out Selection through Remote; pass through to the ASPM
    enum class ManeuverDirectionSelect : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        NoseFirst = 1,
        RearFirst = 2,
        Reserved1 = 3
    };

    // Remote Vehicle Gear selection request; pass through to the ASPM
    enum class ManeuverGearSelect : uint8_t
    {
        InvalidSig = 255,
        Park = 0,
        Forward = 1,
        Reverse = 2,
        Reserved1 = 3
    };

    // Parking Side selection through remote; pass through to the ASPM
    enum class ManeuverSideSelect : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        Left = 1,
        Right = 2,
        Center = 3
    };

    // DMH gesture status; pass through to the ASPM
    enum class ManeuverEnableInput : uint8_t
    {
        InvalidSig = 255,
        NoScrnInput = 0,
        ValidScrnInput = 1,
        InvalidScrnInput = 2,
        Reserved1 = 3
    };

    /*!
     * Push Pull Maneouver selection through remote interface; pass through to
     * the ASPM
    */
    enum class ExploreModeSelect : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        NotPressed = 0,                             /**< 0 */
        Pressed = 1                                 /**< 1 */
    };

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     * X coordinate from mobile to be passed to DMH
     * PhysicalRange 0 - 2047 1 0
     */
    typedef uint16_t AppSliderPosX;

    /*!
     * Holds the current ASP signal value for reference; write to ASP
     * X coordinate from mobile to be passed to DMH
     * PhysicalRange 0 - 4095 1 0
     */
    typedef uint16_t AppSliderPosY;

    /*!
     * Remote Nudge Control Request; pass through to the ASPM
    */
    enum class NudgeSelect : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        NudgeNotAvailable = 0,                      /**< 0 */
        NudgeNotPressed = 1,                        /**< 1 */
        NudgePressed = 2,                           /**< 2 */
        Reserved1 = 3,                              /**< 3 */
    };

    // The Telematics_VM shall report the status of Remote Device connection to
    // the ASPM; DCM to ASP
    enum class ConnectionApproval : uint8_t
    {
        InvalidSig = 255,
        NoDevice = 0,
        NotAllowedDevice = 1,
        AllowedDevice = 2,
        Reserved1 = 3
    };

    /*!
     * For Mobile app to inform ASP of app battery level; write to ASP
     * Physical Range: 0 - 100 (0.0 - 100.0)
     * Offset: [empty]
     * Scale factor: 1.0
     * Unit: %
     */
    typedef uint8_t RemoteDeviceBatteryLevel;

    /*!
     * (Application level) Safety CRC for remote features; write to ASP
     * Physical Range: 0 - 65535 (0.0 - 65535.0)
     * Offset: 0.0
     * Scale factor: 1.0
     * Unit: N/A
     */
    typedef uint16_t AppCalcCheck;

    /*!
     * (Application level)  Response from VDC for challenge; write to ASP
     */
    typedef uint64_t MobileChallengeReply;

    /*!
     * Acceleration in X direction from App
     */
    typedef int64_t AppAccelerationX;

    /*!
     * Acceleration in Y direction from App
     */
    typedef int64_t AppAccelerationY;

    /*!
     * Acceleration in Z direction from App; repurposed for DMH gesture progress
     */
    typedef int64_t AppAccelerationZ;

};

/*!
 * Signals that originate from ASP to TCM
 */
class ASP
{

public:

    /*
    *** READ SIGNALS FROM ASP TO MOBILE ***
    */

    // Remote Message - disappear only when condition no more exist and feature
    // is still active; pass through to remote device
    enum class InfoMsg : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        SearchingForSpaces = 1,                     /**< 1 */
        SlowDownSearchForSpaces = 2,                /**< 2 */
        SlowDownViewSpaces = 3,                     /**< 3 */
        DriveForwardSpaceSearch = 4,                /**< 4 */
        MoveSuspensionToNormalHeight = 5,           /**< 5 */
        SpaceTooSmall = 6,                          /**< 6 */
        NarrowSpaceAvailableThroughRemoteOnly = 7,  /**< 7 */
        SpaceOccupied = 8,                          /**< 8 */
        CantManeuverIntoSpace = 9,                  /**< 9 */
        BringVehicleToRestAndApplyBrake = 10,       /**< 10 */
        ConfirmAndReleaseBrakeToStart = 11,         /**< 11 */
        ReleaseBrakeToStart = 12,                   /**< 12 */
        MoveIndicatorToChangeSelectionSide = 13,    /**< 13 */
        NoAvailableParkOut = 14,                    /**< 14 */
        NotEnoughSpaceToParkOut = 15,               /**< 15 */
        SelectParkOutManeuver = 16,                 /**< 16 */
        RemoteDeviceConnectedAndReady = 17,         /**< 17 */
        RemoteDeviceBatteryTooLow = 18,             /**< 18 */
        RemoteManeuverReady = 19,                   /**< 19 */
        DriverMustBeOutsideOfVehicle = 20,          /**< 20 */
        PowerReservesLow = 21,                      /**< 21 */
        MindOtherRoadUsersDisclaimer = 22,          /**< 22 */
        EngageReverseToStartManeuver = 23,          /**< 23 */
        ApproachingLegalDistanceLimit = 24,         /**< 24 */
        ApproachingMaximumDistance = 25,            /**< 25 */
        ApproachingMaximumDistanceForOperation = 26 /**< 26 */
    };

    // Telematics_VM forwards the ASP currently active feature to the remote
    // device; pass through to remote device
    enum class ActiveAutonomousFeature : uint8_t
    {
        InvalidSig = 255,
        NoFeatureActive = 0,
        Parking = 1,
        RemoteControlDrive = 2,
        WirelessChargingAlignment = 3,
        LSAEB = 4,
        TowAssist = 5,
        Reserved10 = 6,
        Reserved9 = 7,
        Reserved8 = 8,
        Reserved7 = 9,
        Reserved6 = 10,
        Reserved5 = 11,
        Reserved4 = 12,
        Reserved3 = 13,
        Reserved2 = 14,
        Reserved1 = 15
    };

    // Maneouvre type Selection Status; pass through to remote device
    enum class ActiveParkingType : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        Assisted = 1,
        Automated = 2,
        Remote = 3,
        PushPull = 4,
        LongitudinalAssist = 5,
        Reserved2 = 6,
        Reserved1 = 7
    };

    // Park In or Park Out Maneouvre Selection Status; pass through to remote
    enum class ActiveParkingMode : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        ParkIn = 1,
        ParkOut = 2,
        Reserved1 = 3
    };

    // Active Feature Status - System provides the current state of the Active
    // Feature; pass through to remote device
    enum class ManeuverStatus : uint8_t
    {
        InvalidSig = 255,
        NotActive = 0,
        Scanning = 1,
        Selecting = 2,
        Confirming = 3,
        Maneuvering = 4,
        Interrupted = 5,
        Finishing = 6,
        Ended = 7,
        RCStartStop = 8,
        Holding = 9,
        Cancelled = 10,
        Reserved5 = 11,
        Reserved4 = 12,
        Reserved3 = 13,
        Reserved2 = 14,
        Reserved1 = 15
    };

    // Reason of Feature cancellation; pass through to remote device
    enum class CancelMsg : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        CancelledFromLossOfTraction = 1,
        CancelledMaxNumMovesReached = 2,
        CancelledPausedForTooLong = 3,
        CancelledInternalSystemFailure = 4,
        CancelledTrailerConnected = 5,
        ManoeuvreCancelledDriverRequest = 6,
        ManoeuvreCancelledVehicleDrivenOn = 7,
        CancelledSpeedTooHigh = 8,
        CancelledVehicleInMotorway = 9,
        Reserved6 = 10,
        Reserved5 = 11,
        Reserved4 = 12,
        Reserved3 = 13,
        Reserved2 = 14,
        Reserved1 = 15
    };

    // Persistent pop-up when feature is active and won’t disappear until
    // condition exist; pass through to remote device
    enum class PauseMsg1 : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        PausedDriverReq  = 1,
        PausedSteeringIntervention  = 2,
        PausedManualGearChange  = 3,
        PausedDriverBraked = 4,
        PausedParkBrake  = 5,
        PausedAcceleratorPressed = 6,
        PausedEngineStalled = 7,
        PausedDoorOpen = 8,
        PausedBootOpen  = 9,
        PausedBonnetOpen = 10,
        PausedObstacleDetected  = 11,
        PauseRideHeightChanged = 12,
        PausedSensorPerformance = 13,
        RemoteCommunicationLost = 14,
        PausedPowerLow = 15
    };

    // To inform the user about the reason for the feature interruption;
    // pass through to remote device
    enum class PauseMsg2 : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        ActivityKeyInVehicle  = 1,
        ActivityKeyOutsideLegalDistance  = 2,
        ActivityKeyDistanceIndeterminate  = 3,
        ActivityKeyMissing = 4,
        MaximumDistanceReached  = 5,
        MaximumDurationForOperationReached = 6,
        ApplicationCRCFailure = 7,
        ChallengeResponseMismatch = 8,
        DMHInvalid = 9,
        TemporarySystemFailure = 10
    };

    // Reason for feature unavailability
    enum class NoFeatureAvailableMsg : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        NotAvailableSystemFault  = 1,
        NotAvailableSensorBlocked  = 2,
        NotAvailableVehicleNotStarted  = 3,
        NotAvailableTiltTooStrong = 4,
        NotAvailableTrailerConnected  = 5,
        NotAvailableRideHeight = 6,
        NotAvailableSpeedToohigh = 7,
        NotAvailableATPC = 8,
        NotAvailableTowAssistOn  = 9,
        NotAvailableWadeAssistOn = 10,
        NotAvailableACCOn  = 11,
        NotAvailableTJPOn = 12,
        NotAvailableVehicleOnMotorway = 13,
        Reserved2 = 14,
        Reserved1 = 15
    };

    // Pop-up will appear when feature is active and respective condition is
    // true and would disappear on driver/user interference.
    enum class InstructMsg : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        BringVehicleToRest = 1,
        ReleaseBrakesToStart = 2,
        SelectR = 3,
        SelectD = 4,
        SelectFirstGear = 5,
        DeselctRtoDisplaySpaces = 6,
        DriveForward = 7,
        ContinueForward = 8,
        DriveBackward = 9,
        ContinueBackward = 10,
        Stop = 11,
        EngageParkBrake = 12,
        MonitorManouevreInProgress = 13,
        RemoteManouevreInProgress = 14,
        VehicleStopping = 15,
        VehicleStopped = 16,
        SystemOperationRestrictedCapabilityReached = 17,
        SystemOperationRestrictedOccupantMovement = 18,
        PressAcceleratorToResume = 19,
        PauseManuouevre = 20
    };

    // Push Pull Maneouver availability; pass through to remote device
    enum class ExploreModeAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Remote Control Drive Maneouvre Availabilty; pass through to remote device
    enum class RemoteDriveAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Remote Control Drive Maneouvre Availabilty; pass through to remote device
    enum class ManeuverSideAvailability : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        OfferLeftOnly = 1,
        OfferRightOnly = 2,
        OfferLeftAndRight = 3,
    };

    // Perpendicular Mode Change[Nose First/Rear First]; pass through to remote
    enum class DirectionChangeAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Parking Side Selection Status; pass through to remote device
    enum class ActiveManeuverSide : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        Left = 1,
        Right = 2,
        Center = 3
    };

    // Parking orientation Status; pass through to remote device
    enum class ActiveManeuverOrientation : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        Parallel = 1,
        PerpendicularRear = 2,
        PerpendicularFront = 3
    };

    // Parking Orientation change is available (when both paraller and
    // Perpendicular are possible); pass through to remote device
    enum class ParkTypeChangeAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Push/Pull and Nudge available direction; pass through to remote device
    enum class ManeuverDirectionAvailability : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        OfferForward = 1,
        OfferBackward = 2,
        OfferForwardAndBackward = 3
    };

    // Available Maneouver Side to be sent to Remote for Driver Selection; pass
    // through to remote device
    enum class ManeuverAlignmentAvailability : uint8_t
    {
        InvalidSig = 255,
        None = 0,
        OfferLeftOnly = 1,
        OfferRightOnly = 2,
        OfferCenterOnly = 3,
        OfferLeftAndRight = 4,
        OfferLeftAndCenter = 5,
        OfferCenterAndRight = 6,
        OfferLeftCenterAndRight = 7
    };

    // Return to Original Position Availability; pass through to remote
    enum class ReturnToStartAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Longitudinal Adjustement Maneouvre Availability; pass through to remote
    enum class LongitudinalAdjustAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    /*!
     * Holds the current ASP signal value for reference; read from ASP
     * Longitudinal adjustment reamaining distance from target
     * PhysicalRange 0 - 1023 (mm)
     */
    typedef uint16_t LongitudinalAdjustLength;

    /*!
     * Indicates the progress of each manouevre in the form of a progress bar
     * for the remote device Holds the current ASP signal value for reference;
     * read from ASP
     * PhysicalRange 0 - 100 1 0 %
     * LogicalValue 101 Hide Progress Bar
     * PhysicalRange 102 - 127 1 0 Reserved
     */
    typedef uint8_t ManeuverProgressBar;

    // Cancel the Maneouvre; pass through to remote device
    enum class CancelAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Confirm the Maneouvre; pass through to remote device
    enum class ConfirmAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Range of remote from the Vehicle; pass through to remote device
    enum class KeyFobRange : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        Unknown = 0,                                /**< 0 */
        Normal = 1,                                 /**< 1 */
        TooClose = 2,                               /**< 2 */
        ApproachingFar = 3,                         /**< 3 */
        TooFar = 4,                                 /**< 4 */
        Reserved3 = 5,                              /**< 5 */
        Reserved2 = 6,                              /**< 6 */
        Reserved1 = 7                               /**< 7 */
    };

    // Display Lateral Control info; pass through to remote device
    enum class LateralControlInfo : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        RetakeLateralControls = 1,                  /**< 1 */
        DriverInChargeOfLateralControls = 2,        /**< 2 */
        ReleaseLateralControls = 3,                 /**< 3 */
        SystemInChargeOfLateralControls = 4,        /**< 4 */
        Reserved3 = 5,                              /**< 5 */
        Reserved2 = 6,                              /**< 6 */
        Reserved1 = 7                               /**< 7 */
    };

    // Display Longitudinal Control info; pass through to remote device
    enum class LongitudinalControlInfo : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        RetakeLongitudinalControls = 1,             /**< 1 */
        DriverInChargeOfLongitudinalControls = 2,   /**< 2 */
        ReleaseLongitudinalControls = 3,            /**< 3 */
        SystemInChargeOfLongitudinalControls = 4,   /**< 4 */
        Reserved3 = 5,                              /**< 5 */
        Reserved2 = 6,                              /**< 6 */
        Reserved1 = 7                               /**< 7 */
    };

    // Remote Control Pop-up Enable/Disable; pass through to remote device
    enum class ManueverPopupDisplay : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        Hide = 0,                                   /**< 0 */
        Show = 1                                    /**< 1 */
    };

    // Resume the Maneouvre; pass through to remote device
    enum class ResumeAvailability : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        OfferEnabled = 1,                           /**< 1 */
        OfferDisabled = 2,                          /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    // Override states; pass through to remote device
    enum class RemoteDriveOverrideState : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        NoOvrrdAvail = 0,                           /**< 0 */
        OvrrdAvail = 1,                             /**< 1 */
        OvrrdInPrgs = 2,                            /**< 2 */
        Reserved1 = 3                               /**< 3 */
    };

    /*!
     * (Application level)  Challenge Information from ASP for Remote Features
     */
    typedef uint64_t MobileChallengeSend;

};

/*!
 * Signals that originate from or terminanting at DCM on VDC
 */
class DCM
{

public:

    /*!
     * Holds the fob ranging request rate values, in μs
     */
    enum class FobRangeRequestRate : uint32_t
    {
        None = 0,                                   /**<0 */
        DefaultRate = 5000000,                      /**<5000000 */
        DeadmanRate = 500000                        /**<500000 */
    };

    /*
    *** SIGNALS ORIGINATING FROM OR TERMINATING AT VDC ***
    */

    /*!
     * InControl Remote PIN accepted by Telematics_VM inform RD the DCM PIN
     * stored in memory is not set ([REQ NAME HERE]); DCM to Mobile
     */
    enum class AcknowledgeRemotePIN : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        IncorrectPIN = 1,                           /**< 1 */
        IncorrectPIN3xLock60s = 2,                  /**< 2 */
        IncorrectPIN3xLock300s = 3,                 /**< 3 */
        IncorrectPIN3xLock3600s = 4,                /**< 4 */
        IncorrectPIN3xLockIndefinite = 5,           /**< 5 */
        ExpiredPIN = 6,                             /**< 6 */
        NotSetInDCM = 7,                            /**< 7 */
        CorrectPIN = 8,                             /**< 8 */
    };

    // The DCM RPA application shall provide feedback messages to the RD.
    enum class ErrorMsg : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        None = 0,                                   /**< 0 */
        ElectricChargerConnected = 1,               /**< 1 */
        StolenVehicleTrackingAlert = 2,             /**< 2 */
        RemoteStartMaxAttemptsReached = 3,          /**< 3 */
        VehicleCrashDetected = 4,                   /**< 4 */
        LowFuelWarning = 5,                         /**< 5 */
        ErrorFromCCM = 6,                           /**< 6 */
        MultipleKeyFobsDetected = 7,                /**< 7 */
        LowBatteryWarning = 8,                      /**< 8 */
        RemoteSessionExpirationWarning = 9,         /**< 9 */
        Reserved9 = 10,                             /**<10 */
        Reserved8 = 11,                             /**<11 */
        Reserved7 = 12,                             /**<12 */
        Reserved6 = 13,                             /**<13 */
        Reserved5 = 14,                             /**<14 */
        Reserved4 = 15,                             /**<15 */
        Reserved3 = 16,                             /**<16 */
        Reserved2 = 17,                             /**<17 */
        Reserved1 = 18,                             /**<18 */
        Reserved0 = 19,                             /**<19 */
        WiFiAdvertisingTimeoutIn30sec = 20,         /**<20 */
        WiFiAdvertisingTimeoutIn1min = 21,          /**<21 */
        WiFiAdvertisingTimeoutIn2min = 22,          /**<22 */
        WiFiAdvertisingTimeoutIn3min = 23,          /**<23 */
        WiFiAdvertisingTimeoutIn4min = 24,          /**<24 */
        WiFiAdvertisingTimeoutIn5min = 25           /**<25 */
    };

    // hold PIN macros
    static constexpr auto POC_PIN = "0000";
    static constexpr auto PIN_NOT_SET = "-1";

};

/*!
 * Signals that originate from Body Control Module (CCM)
 */
class CCM
{

public:

    /*!
     * Holds the current engine power status
     */
    enum class PowerStatus : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        KeyOut = 0,                                 /**< 0 */
        KeyRecentlyOut = 1,                         /**< 1 */
        KeyApproved = 2,                            /**< 2 */
        PostAccessory = 3,                          /**< 3 */
        Accessory = 4,                              /**< 4 */
        PostIgnition = 5,                           /**< 5 */
        IgnitionOn = 6,                             /**< 6 */
        Running = 7,                                /**< 7 */
        RESERVED = 8,                               /**< 8 */
        Crank = 9                                   /**< 9 */
    };

    /*!
     * Holds the current binary value for if driver door is open
     */
    typedef bool DoorOpenDriver;

    /*!
     * Holds the current binary value for if passenger door is open
     */
    typedef bool DoorOpenPassenger;

    /*!
     * Holds the current binary value for if left rear door is open
     */
    typedef bool DoorOpenLeftRear;

    /*!
     * Holds the current binary value for if right rear door is open
     */
    typedef bool DoorOpenRightRear;

    /*!
     * Holds the current binary value for if tailgate is open
     */
    typedef bool DoorOpenTailgate;

    /*!
     * Holds the current binary value for if bonnet is open
     */
    typedef bool DoorOpenEngineHood;

    /*!
     * Current vehicle door lock status from CCM
     */
    enum class LockStatus : uint8_t
    {
        InvalidSig = 255,                           /**<255 */
        Unlocked = 0,                               /**< 0 */
        PartLocked = 1,                             /**< 1 */
        CentralLocked = 2,                          /**< 2 */
        DoubleLocked = 3,                           /**< 3 */
        TheftProtectNODoubleLock = 4,               /**< 4 */
        RESERVED = 5                                /**< 5 */
    };

};


/*
*** VENDOR-PROVIDED CODE LISTED BELOW FOR VDC-SPECIFIC PROCESSES ***
*/


// constants related to ASPM -> TCM UDP payload
// packet size from ASP : (8*5)+37+8+42+32+16 = 175
#define ASPM_TOTAL_PACKET_SIZE             175
#define NUMBER_OF_ASPM_PDU                   5
#define LENGTH_OF_ASPM_LM                   37
#define LENGTH_OF_ASPM_RemoteTarget          8
#define LENGTH_OF_ASPM_LM_Session           42
#define LENGTH_OF_ASPM_LM_ObjSegment        32
#define LENGTH_OF_ASPM_LM_Trunc             16
#define LENGTH_OF_ASPM_LARGEST_Size         42

#define HRD_ID_OF_ASPM_LM                   49
#define HRD_ID_OF_ASPM_RemoteTarget         50
#define HRD_ID_OF_ASPM_LM_Session           66
#define HRD_ID_OF_ASPM_LM_ObjSegment        73
#define HRD_ID_OF_ASPM_LM_Trunc             76

// constants related to TCM -> ASPM UDP payload
// packet size from TCM : (8*9)+30+42+8+20+32+1+18+18+18=259
#define TCM_TOTAL_PACKET_SIZE              259
#define NUMBER_OF_TCM_PDU                    9
#define LENGTH_OF_TCM_LM                    30
#define LENGTH_OF_TCM_LM_Session            42
#define LENGTH_OF_TCM_RemoteControl          8
#define LENGTH_OF_TCM_TransportKey          20
#define LENGTH_OF_TCM_LM_App                32
#define LENGTH_OF_TCM_LM_KeyID               1
#define LENGTH_OF_TCM_LM_KeyAlpha           18
#define LENGTH_OF_TCM_LM_KeyBeta            18
#define LENGTH_OF_TCM_LM_KeyGamma           18

#define HRD_ID_OF_TCM_LM                    57
#define HRD_ID_OF_TCM_LM_Session            68
#define HRD_ID_OF_TCM_RemoteControl         67
#define HRD_ID_OF_TCM_TransportKey          69
#define HRD_ID_OF_TCM_LM_App                73
#define HRD_ID_OF_TCM_LM_KeyID              75
#define HRD_ID_OF_TCM_LM_KeyAlpha           70
#define HRD_ID_OF_TCM_LM_KeyBeta            71
#define HRD_ID_OF_TCM_LM_KeyGamma           72

/*!
 * PDU group that a signal in a UDP packet beongs to.
 * PDU_ASPM_* indicates a PDU sent from the ASPM to the TCM.
 * PDU_TCM_* indicates a PDU sent from the TCM to the ASPM.
 */
namespace PDU_TYPE {
    enum ID {
        PDU_NONE = 0,
        PDU_ASPM_LM = 11,
        PDU_ASPM_RemoteTarget,
        PDU_ASPM_LM_Session,
        PDU_ASPM_LM_ObjSegment,
        PDU_ASPM_LM_Trunc,

        PDU_TCM_LM = 21,
        PDU_TCM_RemoteControl,
        PDU_TCM_LM_Session,
        PDU_TCM_TransportKey,
        PDU_TCM_LM_App,
        PDU_TCM_LM_KeyID,
        PDU_TCM_LM_KeyAlpha,
        PDU_TCM_LM_KeyBeta,
        PDU_TCM_LM_KeyGamma
    };
}

/*!
 * Signals that belong to the ASPM_LM PDU, sent from the ASPM to the TCM
 */
namespace ASPM_LM { //ASPM_LM : <HEADER-ID>49</HEADER-ID>, <LENGTH>37</LENGTH>
    enum ID {
        // 1
        LMAppConsChkASPM = 1,       //!<   16 bits, ID: 7, (Application level) Safety CRC for remote features - ASPM
        ActiveAutonomousFeature,         //!<   4 bits, ID: 23, Remote Active Feature - System informs which feature/group of feature is active.
        CancelAvailability,            //!<   2 bits, ID: 19, Cancel the Maneouvre
        ConfirmAvailability,           //!<   2 bits, ID: 17, Confirm the Maneouvre
        LongitudinalAdjustAvailability,      //!<   2 bits, ID: 31, Longitudinal Adjustement Maneouvre Availability
        ManeuverDirectionAvailability,        //!<   2 bits, ID: 29, Push/Pull and Nudge available direction.
        ManeuverSideAvailability,          //!<   2 bits, ID: 27, Available Maneouver Side to be sent to Remote for Driver Selection
        ActiveManeuverOrientation,           //!<   2 bits, ID: 25, Parking Orientation Selection Status
        ActiveParkingMode,           //!<   2 bits, ID: 39, Park In or Park Out Maneouvre Selection Status
        DirectionChangeAvailability,       //!<   2 bits, ID: 37, Perpendicular Mode Change[Nose First/Rear First]
        // 11
        ParkTypeChangeAvailability,      //!<   2 bits, ID: 35, Parking Orientation change is available (when both paraller and Perpendicular are possible)
        ExploreModeAvailability,      //!<   2 bits, ID: 33, Push Pull Maneouver availbality.
        ActiveManeuverSide,                  //!<   2 bits, ID: 47, Parking Side Selection Status
        ManeuverStatus,                //!<   4 bits, ID: 45, Active Feature Status - System provides the current state of the Active Feature.
        RemoteDriveOverrideState,           //!<   2 bits, ID: 41, Override States
        ActiveParkingType,           //!<   3 bits, ID: 55, Maneouvre type Selection Status
        ResumeAvailability,            //!<   2 bits, ID: 52, Resume the Maeouvre
        ReturnToStartAvailability,       //!<   2 bits, ID: 50, Return to Original Position
        NNNNNNNNNN = -1,            //!<   2 bits, ID: 48, Unused
        KeyFobRange = 19,      //!<   3 bits, ID: 62, Range of remote from the Vehicle
        LMDviceAliveCntAckRMT,      //!<   4 bits, ID: 59, Heartbeat acknowledgement to TCM
        // 21
        NoFeatureAvailableMsg,       //!<   4 bits, ID: 71, Reason for feature unavailability.
        LMFrwdCollSnsType1RMT,      //!<   1 bit, ID: 67, The classification of the type of object that is detected by the ASPM at front in Zone 1.
        LMFrwdCollSnsType2RMT,      //!<   1 bit, ID: 66, The classification of the type of object that is detected by the ASPM at front in Zone 2.
        LMFrwdCollSnsType3RMT,      //!<   1 bit, ID: 65, The classification of the type of object that is detected by the ASPM at front in Zone 3.
        LMFrwdCollSnsType4RMT,      //!<   1 bit, ID: 64, The classification of the type of object that is detected by the ASPM at front in Zone 4.
        LMFrwdCollSnsZone1RMT,      //!<   4 bits, ID: 79, Forward Vehicle Threat Sensing Zone - Segment 1 to 4
        LMFrwdCollSnsZone2RMT,      //!<   4 bits, ID: 75, Forward Vehicle Threat Sensing Zone - Segment 5 to 8
        LMFrwdCollSnsZone3RMT,      //!<   4 bits, ID: 87, Forward Vehicle Threat Sensing Zone - Segment 9 to 12
        LMFrwdCollSnsZone4RMT,      //!<   4 bits, ID: 83, Forward Vehicle Threat Sensing Zone - Segment 13 to 16
        InfoMsg,              //!<   6 bits, ID: 95, Remote Message - disappear only when condition no more exist and feature is still active
        // 31
        InstructMsg,          //!<   5 bits, ID: 89, Pop-up will appear when feature is active and respective condition is true and would disappear on driver/user interference.
        LateralControlInfo,          //!<   3 bits, ID: 100, Display Lateral Control info
        LongitudinalAdjustLength,      //!<   10 bits, ID: 97, Longitidinal adjustment reamaining distance from target
        LongitudinalControlInfo,         //!<   3 bits, ID: 119, Display Longitudinal Control info
        ManeuverAlignmentAvailability,         //!<   3 bits, ID: 116, Available Maneouver Side to be sent to Remote for Driver Selection
        RemoteDriveAvailability,           //!<   2 bits, ID: 113, Remote Control Drive Maneouvre Availabilty
        PauseMsg2,            //!<   4 bits, ID: 127, A persitent pop-up that appears when the feature is active to inform of a pause due to activity key parameters.
        PauseMsg1,             //!<   4 bits, ID: 123, Persistent pop-up when feature is active and won't disappear until condition exist
        LMRearCollSnsType1RMT,      //!<   1 bit, ID: 135, The classification of the type of object that is detected by the ASPM at Rear in Zone 1.
        LMRearCollSnsType2RMT,      //!<   1 bit, ID: 134
        // 41
        LMRearCollSnsType3RMT,      //!<   1 bit, ID: 133
        LMRearCollSnsType4RMT,      //!<   1 bit, ID: 132, The classification of the type of object that is detected by the ASPM at Rear in Zone 4.
        LMRearCollSnsZone1RMT,      //!<   4 bits, ID: 131, Rear Vehicle Threat Sensing Zone - Segment 1 to 4
        LMRearCollSnsZone2RMT,      //!<   4 bits, ID: 143, Rear Vehicle Threat Sensing Zone - Segment 5 to 8
        LMRearCollSnsZone3RMT,      //!<   4 bits, ID: 139, Rear Vehicle Threat Sensing Zone - Segment 9 to 12
        LMRearCollSnsZone4RMT,      //!<   4 bits, ID: 151, Rear Vehicle Threat Sensing Zone - Segment 13 to 16
        LMRemoteFeatrReadyRMT,      //!<   2 bits, ID: 147, Remote Feature ready to maneouvre
        CancelMsg,            //!<   4 bits, ID: 145, Reason of Feature cancellation
        LMVehMaxRmteVLimRMT,        //!<   6 bits, ID: 157, Maximum Permissible Vehicle Speed for Remote Maneouvre
        ManueverPopupDisplay,             //!<   1 bit, ID: 167, Remote Control Pop-up Enable/Disable
        // 51
        ManeuverProgressBar,     //!<   7 bits, ID: 166, Indicates the progress of each manouevre in the form of a progress bar for the remote device.
        MobileChallengeSend,      //!<   64 bits, ID: 175, (Application level) Challenge Information from ASP for Remote Features
        LMRemoteResponseASPM,       //!<   64 bits, ID: 239, (Application level) Response from ASPM for challenge ( Remote Features)

        MAXSignal
    };
}

/*!
 * Signals that belong to the ASPM_RemoteTarget PDU, sent from the ASPM to the TCM
 */
namespace ASPM_RemoteTarget { //ASPM_LM : <HEADER-ID>50</HEADER-ID>, <LENGTH>8</LENGTH>
    enum ID {
        TCMRemoteTarget = 1,
        MAXSignal
    };
}

/*!
 * Signals that belong to the ASPM_LM_Session PDU, sent from the ASPM to the TCM
 */
namespace ASPM_LM_Session { //ASPM_LM : <HEADER-ID>66</HEADER-ID>, <LENGTH>42</LENGTH>
    enum ID {
        LMEncrptSessionCntASPM_1 = 1,
        LMEncrptSessionCntASPM_2,
        LMEncryptSessionIDASPM_1,
        LMEncryptSessionIDASPM_2,
        LMTruncMACASPM,
        LMTruncSessionCntASPM,
        LMSessionControlASPM,
        LMSessionControlASPMExt,
        MAXSignal
    };
}

/*!
 * Signals that belong to the ASPM_LM_Trunc PDU, sent from the ASPM to the TCM
 */
namespace ASPM_LM_Trunc { //ASPM_LM : <HEADER-ID>76</HEADER-ID>, <LENGTH>16</LENGTH>
    enum ID {
        LMTruncEnPsPrasRotASPM_1 =1,
        LMTruncEnPsPrasRotASPM_2,
        MAXSignal

    };
}

/*!
 * Signals that belong to the ASPM_LM_ObjSegment PDU, sent from the ASPM to the TCM
 */
namespace ASPM_LM_ObjSegment { //ASPM_LM : <HEADER-ID>73</HEADER-ID>, <LENGTH>32</LENGTH>
    enum ID {
        ASPMXXXXX = -1,
        ASPMFrontSegType1RMT = 1,
        ASPMFrontSegDist1RMT,
        ASPMFrontSegType2RMT,
        ASPMFrontSegDist2RMT,
        ASPMFrontSegType3RMT,
        ASPMFrontSegDist3RMT,
        ASPMFrontSegType4RMT,
        ASPMFrontSegDist4RMT,
        ASPMFrontSegType5RMT,
        ASPMFrontSegDist5RMT,
        ASPMFrontSegType6RMT,
        ASPMFrontSegDist6RMT,
        ASPMFrontSegType7RMT,
        ASPMFrontSegDist7RMT,
        ASPMFrontSegType8RMT,
        ASPMFrontSegDist8RMT,
        ASPMFrontSegType9RMT,
        ASPMFrontSegDist9RMT,
        ASPMFrontSegType10RMT,
        ASPMFrontSegDist10RMT,
        ASPMFrontSegType11RMT,
        ASPMFrontSegDist11RMT,
        ASPMFrontSegType12RMT,
        ASPMFrontSegDist12RMT,
        ASPMFrontSegType13RMT,
        ASPMFrontSegDist13RMT,
        ASPMFrontSegType14RMT,
        ASPMFrontSegDist14RMT,
        ASPMFrontSegType15RMT,
        ASPMFrontSegDist15RMT,
        ASPMFrontSegType16RMT,
        ASPMFrontSegDist16RMT,
        ASPMRearSegType1RMT,
        ASPMRearSegDist1RMT,
        ASPMRearSegType2RMT,
        ASPMRearSegDist2RMT,
        ASPMRearSegType3RMT,
        ASPMRearSegDist3RMT,
        ASPMRearSegType4RMT,
        ASPMRearSegDist4RMT,
        ASPMRearSegType5RMT,
        ASPMRearSegDist5RMT,
        ASPMRearSegType6RMT,
        ASPMRearSegDist6RMT,
        ASPMRearSegType7RMT,
        ASPMRearSegDist7RMT,
        ASPMRearSegType8RMT,
        ASPMRearSegDist8RMT,
        ASPMRearSegType9RMT,
        ASPMRearSegDist9RMT,
        ASPMRearSegType10RMT,
        ASPMRearSegDist10RMT,
        ASPMRearSegType11RMT,
        ASPMRearSegDist11RMT,
        ASPMRearSegType12RMT,
        ASPMRearSegDist12RMT,
        ASPMRearSegType13RMT,
        ASPMRearSegDist13RMT,
        ASPMRearSegType14RMT,
        ASPMRearSegDist14RMT,
        ASPMRearSegType15RMT,
        ASPMRearSegDist15RMT,
        ASPMRearSegType16RMT,
        ASPMRearSegDist16RMT,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM {  // TCM_Telematics_LM : <HEADER-ID>57</HEADER-ID>, <LENGTH>30</LENGTH>
    enum ID {
        // 1
        AppCalcCheck = 1,        //!< 16 bits, ID: 7 , (Application level) Safety CRC for remote features
        LMDviceAliveCntRMT,         //!< 4 bits, ID: 23, Heartbeat ensures Connection between Remote Device and Vehicle
        ManeuverEnableInput,        //!< 2 bits, ID: 19, Dead Mans handle status
        ManeuverGearSelect,        //!< 2 bits, ID: 17, Remote Vehicle Gear selection request
        NudgeSelect,             //!< 2 bits, ID: 31, Remote Nudge Control Request
        RCDOvrrdReqRMT,             //!< 2 bits, ID: 29, Override Request to adjust the vehicle speed etc through Remote.
        AppSliderPosY,             //!< 12 bits, ID: 27, to detect the finger input coordinates on the device screen.
        AppSliderPosX,             //!< 11	bits, ID: 47, to detect the finger input coordinates on the device screen
        RCDSpeedChngReqRMT,         //!< 6 bits, ID: 52, Vehicle Speed change request through remote
        RCDSteWhlChngReqRMT,        //!< 10 bits, ID: 62, Steering Wheel Change request through Remote
        //11
        ConnectionApproval,          //!< 2 bits, ID: 68, Remote Device connection to the Vehicle
        ManeuverButtonPress,       //!< 3 bits, ID: 66, Feature Maneouvre state Selection throigh Remote like Confirm, Cancel, Pause, Resume or Return to Original Position.
        DeviceControlMode,         //!< 4 bits, ID: 79, Remote Device Control Mode
        ManeuverTypeSelect,      //!< 2 bits, ID: 75, Maneouver Orientation Change Request selected through Remote( Parallel/Perpendicular)
        ManeuverDirectionSelect,        //!< 2 bits, ID: 73, Nose In or Nose Out Selection through Remote, When perpendicular parking is selected.
        ExploreModeSelect,      //!< 1 bit, ID: 87, Push Pull Maneouver selection through remote interface.
        RemoteDeviceBatteryLevel,        //!< 7 bits, ID: 86, Remote Device Battery Level
        PairedWKeyId,               //!< 8 bits, ID: 95, The identification number of the Key ID for which Ranging needs to be performed.
                                    //!< This signal will be set to PkeyId received from RFA in the case when a Passive Key is used and there
                                    //!< is no WK present which is paired with Phone.
        ManeuverSideSelect,           //!< 2 bits, ID: 103, Parking Side selection through remote
        LMDviceRngeDistRMT,         //!< 10 bits, ID: 101, Distance of remote from the Vehicle
        //21
        TTTTTTTTTT,                 //!< 4 bits, ID: 107, Unused
        LMRemoteChallengeVDC,       //!< 64 bits, ID: 119, (Application level) Challenge Information from VDC for Remote Features
        MobileChallengeReply,        //!< 64 bits, ID: 183, (Application level) Response from VDC for challenge ( Remote Features)
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM_Session PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM_Session {
    enum ID {
        LMEncrptSessionCntVDC_1 = 1,
        LMEncrptSessionCntVDC_2,
        LMEncryptSessionIDVDC_1,
        LMEncryptSessionIDVDC_2,
        LMTruncMACVDC,
        LMTruncSessionCntVDC,
        LMSessionControlVDC,
        LMSessionControlVDCExt,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_RemoteControl PDU, sent from the TCM to the ASPM
 */
namespace TCM_RemoteControl {
    enum ID {
        TCMRemoteControl = 1,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_TransportKey PDU, sent from the TCM to the ASPM
 */
namespace TCM_TransportKey {
    enum ID {
        LMSessionKeyIDVDC = 1,
        LMHashEnTrnsportKeyVDC,
        LMEncTransportKeyVDC_1,
        LMEncTransportKeyVDC_2,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM_App PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM_App {
    enum ID {
        LMAppTimeStampRMT = 1,
        AppAccelerationX,
        AppAccelerationY,
        AppAccelerationZ,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM_KeyID PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM_KeyID {
    enum ID {
        NOT_USED_ONE_BIT = 1,
        LMRotKeyChkACKVDC,
        LMSessionKeyIDVDCExt,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM_KeyAlpha PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM_KeyAlpha {
    enum ID {
        LMHashEnRotKeyAlphaVDC = 1,
        LMEncRotKeyAlphaVDC_1,
        LMEncRotKeyAlphaVDC_2,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM_KeyBeta PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM_KeyBeta {
    enum ID {
        LMHashEncRotKeyBetaVDC = 1,
        LMEncRotKeyBetaVDC_1,
        LMEncRotKeyBetaVDC_2,
        MAXSignal
    };
}

/*!
 * Signals that belong to the TCM_LM_KeyGamma PDU, sent from the TCM to the ASPM
 */
namespace TCM_LM_KeyGamma {
    enum ID {
        LMHashEncRotKeyGamaVDC = 1,
        LMEncRotKeyGammaVDC_1,
        LMEncRotKeyGammaVDC_2,
        MAXSignal
    };
}

#endif //CONSTANTS_H
