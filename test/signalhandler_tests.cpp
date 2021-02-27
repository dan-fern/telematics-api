#include <gtest/gtest.h>
#include <cstring>

#include "signalhandler.hpp"

class SignalHandlerTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        sh_ = std::make_shared<SignalHandler>();
    }
    virtual void TearDown() {}
    std::shared_ptr<SignalHandler> sh_;
};

TEST_F(SignalHandlerTest, DecodeASPMSignalData) {
    uint8_t buffer[] = {
        // ASPM_LM PDU
        0x00, 0x00, 0x00, HRD_ID_OF_ASPM_LM, 0x00, 0x00, 0x00, LENGTH_OF_ASPM_LM, // header
        0x12, 0x34,  // LMAppConsChkASPM = 0x1234
        0xE7,        // ActiveAutonomousFeature = 0xE, CancelAvailability = 0x1, ConfirmAvailability = 0x3
        0x9B,        // LongitudinalAdjustAvailability = 0x2, ManeuverDirectionAvailability = 0x1, ManeuverSideAvailability = 0x2, ActiveManeuverOrientation = 0x3
        0x1B,        // ActiveParkingMode = 0x0, DirectionChangeAvailability = 0x1, ParkTypeChangeAvailability = 0x2, ExploreModeAvailability = 0x3
        0xE4,        // ActiveManeuverSide = 0x3, ManeuverStatus = 0x9, RemoteDriveOverrideState = 0x0
        0xAA,        // ActiveParkingType = 0x5, ResumeAvailability = 0x1, ReturnToStartAvailability=0x1
        0x55,        // KeyFobRange = 0x5, LMDviceAliveCntAckRMT = 0x5
        0xFE,        // NoFeatureAvailableMsg = 0xF, LMFrwdCollSnsType[1-3]RMT = 0x1, LMFrwdCollSnsType4RMT = 0x0
        0x43,        // LMFrwdCollSnsZone1RMT = 0x4, LMFrwdCollSnsZone2RMT = 0x3
        0x21,        // LMFrwdCollSnsZone3RMT = 0x2, LMFrwdCollSnsZone4RMT = 0x1
        0x87,        // InfoMsg = 0x21, partial InstructMsg
        0xDF,        // InstructMsg = 0x1E, LateralControlInfo = 0x7, partial LongitudinalAdjustLength
        0x80,        // LongitudinalAdjustLength = 0x380
        0xAA,        // LongitudinalControlInfo = 0x5, ManeuverAlignmentAvailability = 0x2, RemoteDriveAvailability=0x2
        0x96,        // PauseMsg2 = 0x9, PauseMsg1 = 0x6
        0xFF,        // LMRearCollSnsType[1-4]RMT = 0x1, LMRearCollSnsZone1RMT = 0xF
        0x00,        // LMRearCollSnsZone[2-3]RMT = 0x0
        0x4C,        // LMRearCollSnsZone4RMT = 0x4, LMRemoteFeatrReadyRMT = 0x3, partial CancelMsg
        0xC1,        // CancelMsg = 0x3, LMVehMaxRmteVLimRMT = 0x1
        0x66,        // ManueverPopupDisplay = 0x0, ManeuverProgressBar = 0x66
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,  // MobileChallengeSend = 0xFEDCBA9876543210, 0x76543210
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,  // LMRemoteResponseASPM = 0x0123456789ABCDEF

        // ASPM_LM_ObjSegment PDU
        0x00, 0x00, 0x00, HRD_ID_OF_ASPM_LM_ObjSegment, 0x00, 0x00, 0x00, LENGTH_OF_ASPM_LM_ObjSegment, // header
        0x70,  // ASPMFrontSegType1RMT = 0x3, ASPMFrontSegDist1RMT = 0x10
        0x04,  // ASPMFrontSegType2RMT = 0x0, ASPMFrontSegDist2RMT = 0x04
        0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04,
        0x70,  // ASPMRearSegType1RMT = 0x3, ASPMRearSegDist1RMT = 0x10
        0x04,  // ASPMRearSegType2RMT = 0x0, ASPMRearSegDist2RMT = 0x04
        0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04, 0x70, 0x04,

        // ASPM_RemoteTarget PDU
        0x00, 0x00, 0x00, HRD_ID_OF_ASPM_RemoteTarget, 0x00, 0x00, 0x00, LENGTH_OF_ASPM_RemoteTarget, // header
        0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67,  // TCMRemoteTarget = 0x89ABCDEF01234567

        // ASPM_LM_Session PDU
        0x00, 0x00, 0x00, HRD_ID_OF_ASPM_LM_Session, 0x00, 0x00, 0x00, LENGTH_OF_ASPM_LM_Session, // header
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,  // LMEncrptSessionCntASPM_1 = 0xFEDCBA9876543210
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,  // LMEncrptSessionCntASPM_2 = 0x0123456789ABCDEF
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,  // LMEncryptSessionIDASPM_1 = 0xFEDCBA9876543210
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,  // LMEncryptSessionIDASPM_1 = 0x0123456789ABCDEF
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,  // LMTruncMACASPM = 0x0123456789ABCDEF
        0xBB,  // LMTruncSessionCntASPM = 0xBB
        0xB1,  // LMSessionControlASPM = 0x5, LMSessionControlASPMExt = 0x11

        // ASPM_LM_Trunc PDU
        0x00, 0x00, 0x00, HRD_ID_OF_ASPM_LM_Trunc, 0x00, 0x00, 0x00, LENGTH_OF_ASPM_LM_Trunc, // header
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,  // LMTruncEnPsPrasRotASPM_1 = 0xFEDCBA9876543210
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,  // LMTruncEnPsPrasRotASPM_2 = 0x0123456789ABCDEF
    };
    ASSERT_EQ(ASPM_TOTAL_PACKET_SIZE, sizeof(buffer));
    sh_->decodeASPMSignalData(buffer);
    EXPECT_EQ((int)sh_->ActiveAutonomousFeature, 0xE);
    EXPECT_EQ((int)sh_->ConfirmAvailability, 0x3);
    EXPECT_EQ((int)sh_->ResumeAvailability, 0x1);
    EXPECT_EQ((int)sh_->LongitudinalAdjustAvailability, 0x2);
    EXPECT_EQ((int)sh_->ManeuverDirectionAvailability, 0x1);
    EXPECT_EQ((int)sh_->ManeuverSideAvailability, 0x2);
    EXPECT_EQ((int)sh_->ActiveManeuverOrientation, 0x3);
    EXPECT_EQ((int)sh_->ActiveParkingMode, 0x0);
    EXPECT_EQ((int)sh_->DirectionChangeAvailability, 0x1);
    EXPECT_EQ((int)sh_->ParkTypeChangeAvailability, 0x2);
    EXPECT_EQ((int)sh_->ExploreModeAvailability, 0x3);
    EXPECT_EQ((int)sh_->ActiveManeuverSide, 0x3);
    EXPECT_EQ((int)sh_->ManeuverStatus, 0x9);
    EXPECT_EQ((int)sh_->ActiveParkingType, 0x5);
    EXPECT_EQ((int)sh_->LongitudinalAdjustAvailability, 0x2);
    EXPECT_EQ((int)sh_->ReturnToStartAvailability, 0x1);
    EXPECT_EQ((int)sh_->NoFeatureAvailableMsg, 0xF);
    EXPECT_EQ((int)sh_->InfoMsg, 0x21);
    EXPECT_EQ((int)sh_->InstructMsg, 0x1E);
    EXPECT_EQ((int)sh_->LongitudinalAdjustAvailability, 0x2);
    EXPECT_EQ((int)sh_->LongitudinalAdjustLength, 0x380);
    EXPECT_EQ((int)sh_->ManeuverAlignmentAvailability, 0x2);
    EXPECT_EQ((int)sh_->RemoteDriveAvailability, 0x2);
    EXPECT_EQ((int)sh_->PauseMsg2, 0x9);
    EXPECT_EQ((int)sh_->PauseMsg1, 0x6);
    EXPECT_EQ((int)sh_->CancelMsg, 0x3);
    EXPECT_EQ((int)sh_->MobileChallengeSend, 0x76543210);
    EXPECT_EQ((int)sh_->ManeuverProgressBar, 0x66);
    for (int i = 0; i < 16; ++i) {
        if (i % 2 == 0) {
            EXPECT_EQ((int)sh_->getASPMFrontSegDistxxRMT(i), 0x10);
            EXPECT_EQ((int)sh_->getASPMFrontSegTypexxRMT(i), 0x3);
        }
        else {
            EXPECT_EQ((int)sh_->getASPMRearSegDistxxRMT(i), 0x04);
            EXPECT_EQ((int)sh_->getASPMRearSegTypexxRMT(i), 0x0);
        }
    }
}

TEST_F(SignalHandlerTest, EncodeTCMSignalData) {
    // 91a2b3c4855e72c
    sh_->AppAccelerationX = 0x0000000000000000; // 64 bits
    sh_->AppAccelerationY = 0x0000000000000000; // 64 bits
    sh_->AppAccelerationZ = 0x1234567890abcdef; // 64 bits
    sh_->AppCalcCheck = 0x1234; // 16 bits
    sh_->ManeuverEnableInput = TCM::ManeuverEnableInput::ValidScrnInput; // 1, 2 bits
    sh_->ManeuverGearSelect = TCM::ManeuverGearSelect::Reverse; // 2, 2 bits
    sh_->NudgeSelect = TCM::NudgeSelect::NudgeNotAvailable; // 0, 2 bits
    sh_->AppSliderPosY = 0x987; // 12 bits
    sh_->AppSliderPosX = 0x120; // 11 bits
    sh_->ConnectionApproval = TCM::ConnectionApproval::AllowedDevice; // 2, 2 bits
    sh_->ManeuverButtonPress = TCM::ManeuverButtonPress::ContinueManouevre; // 7, 3 bits
    sh_->DeviceControlMode = TCM::DeviceControlMode::RCParkIn; // 5, 4 bits
    sh_->ManeuverTypeSelect = TCM::ManeuverTypeSelect::Parallel; // 1, 2 bits
    sh_->ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::None; // 0, 2 bits
    sh_->ExploreModeSelect = TCM::ExploreModeSelect::Pressed; // 1, 1 bit
    sh_->RemoteDeviceBatteryLevel = 0x7F; // 7 bits
    sh_->ManeuverSideSelect = TCM::ManeuverSideSelect::Right; // 2, 2 bits
    sh_->MobileChallengeReply = 0x1234567890abcdef; // 64 bits

    uint8_t buffer[UDP_BUF_MAX];
    bzero(buffer, UDP_BUF_MAX);
    sh_->encodeTCMSignalData(buffer);

    uint8_t expected[] = {
        // TCM_LM PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM, // header
        0x12, 0x34, // AppCalcCheck
        0x06,       // LMDviceAliveCntRMT, ManeuverEnableInput, ManeuverGearSelect
        0x09, 0x87, // NudgeSelect, RCDOvrrdReqRMT, AppSliderPosY
        0x24, 0x00, // AppSliderPosX, RCDSpeedChngReqRMT, partial RCDSteWhlChngReqRMT
        0x00, 0x17, // partial RCDSteWhlChngReqRMT, ConnectionApproval, ManeuverButtonPress
        0x54,       // DeviceControlMode, ManeuverTypeSelect, ManeuverDirectionSelect
        0xFF,       // ExploreModeSelect, RemoteDeviceBatteryLevel
        0x00,       // PairedWKeyId
        0x80, 0x00, // ManeuverSideSelect, LMDviceRngeDistRMT, TTTTTTTTTT
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LMRemoteChallengeVDC
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, // MobileChallengeReply

        // TCM_LM_Session PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM_Session, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM_Session, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        // TCM_RemoteControl PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_RemoteControl, 0x00, 0x00, 0x00, LENGTH_OF_TCM_RemoteControl, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        // TCM_TransportKey PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_TransportKey, 0x00, 0x00, 0x00, LENGTH_OF_TCM_TransportKey, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

        // TCM_LM_App PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM_App, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM_App, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LMAppTimeStampRMT
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // AppAccelerationX
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // AppAccelerationY
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, // AppAccelerationZ

        // TCM_LM_KeyID PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM_KeyID, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM_KeyID, // header
        0x00,

        // TCM_LM_KeyAlpha PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM_KeyAlpha, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM_KeyAlpha, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,

        // TCM_LM_KeyBeta PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM_KeyBeta, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM_KeyBeta, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,

        // TCM_LM_KeyGamma PDU
        0x00, 0x00, 0x00, HRD_ID_OF_TCM_LM_KeyGamma, 0x00, 0x00, 0x00, LENGTH_OF_TCM_LM_KeyGamma, // header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };
    ASSERT_EQ(TCM_TOTAL_PACKET_SIZE, sizeof(expected));
    EXPECT_TRUE(std::memcmp(buffer, expected, TCM_TOTAL_PACKET_SIZE) == 0);
}

TEST_F(SignalHandlerTest, GetManeuverFromASP) {
    std::string maneuver_str;
    for (int ActiveParkingType = 3; ActiveParkingType <= 5; ++ActiveParkingType) {
    for (int ActiveParkingMode = 0; ActiveParkingMode <= 2; ++ActiveParkingMode) {
    for (int ActiveManeuverOrientation = 0; ActiveManeuverOrientation <= 3; ++ActiveManeuverOrientation) {
    for (int ActiveManeuverSide = 0; ActiveManeuverSide <= 3; ++ActiveManeuverSide) {
        sh_->ActiveParkingType = (ASP::ActiveParkingType)ActiveParkingType;
        sh_->ActiveParkingMode = (ASP::ActiveParkingMode)ActiveParkingMode;
        sh_->ActiveManeuverOrientation = (ASP::ActiveManeuverOrientation)ActiveManeuverOrientation;
        sh_->ActiveManeuverSide = (ASP::ActiveManeuverSide)ActiveManeuverSide;
        maneuver_str = sh_->getManeuverFromASP();
        if (sh_->ActiveParkingType == ASP::ActiveParkingType::PushPull &&
            sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularFront)
        {
            EXPECT_EQ(maneuver_str, "StrFwd");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::PushPull &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularRear)
        {
            EXPECT_EQ(maneuver_str, "StrRvs");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkIn &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularFront &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Left)
        {
            EXPECT_EQ(maneuver_str, "InLftFwd");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkIn &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularFront &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Right)
        {
            EXPECT_EQ(maneuver_str, "InRgtFwd");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkIn &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularRear &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Left)
        {
            EXPECT_EQ(maneuver_str, "InLftRvs");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkIn &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularRear &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Right)
        {
            EXPECT_EQ(maneuver_str, "InRgtRvs");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkOut &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularFront &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Left)
        {
            EXPECT_EQ(maneuver_str, "OutLftFwd");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkOut &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularFront &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Right)
        {
            EXPECT_EQ(maneuver_str, "OutRgtFwd");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkOut &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularRear &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Left)
        {
            EXPECT_EQ(maneuver_str, "OutLftRvs");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkOut &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularRear &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Right)
        {
            EXPECT_EQ(maneuver_str, "OutRgtRvs");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkOut &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::Parallel &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Left)
        {
            EXPECT_EQ(maneuver_str, "OutLftPrl");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::Remote &&
                 sh_->ActiveParkingMode == ASP::ActiveParkingMode::ParkOut &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::Parallel &&
                 sh_->ActiveManeuverSide == ASP::ActiveManeuverSide::Right)
        {
            EXPECT_EQ(maneuver_str, "OutRgtPrl");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::LongitudinalAssist &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularFront)
        {
            EXPECT_EQ(maneuver_str, "NdgFwd");
        }
        else if (sh_->ActiveParkingType == ASP::ActiveParkingType::LongitudinalAssist &&
                 sh_->ActiveManeuverOrientation == ASP::ActiveManeuverOrientation::PerpendicularRear)
        {
            EXPECT_EQ(maneuver_str, "NdgRvs");
        }
        else {
            EXPECT_EQ(maneuver_str, "");
        }
    }
    }
    }
    }
}
