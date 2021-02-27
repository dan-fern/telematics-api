#include <gtest/gtest.h>

#include "testutils.hpp"
#include "remotedevicehandler.hpp"

class TCMSignalTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        asp_ = std::make_shared<StubASP>();
        sh_ = std::make_shared<SignalHandler>();
        rdh_ = std::make_shared<RemoteDeviceHandler>(sh_);
        thr = std::thread([this] { rdh_->spin(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        client_ = std::make_shared<MobileClient>();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // device should not be authenticated until PIN entered and terms accepted
        EXPECT_EQ(sh_->ConnectionApproval, TCM::ConnectionApproval::NotAllowedDevice);

        // simulate RD authentication
        sh_->AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::CorrectPIN;
        sh_->ConnectionApproval = TCM::ConnectionApproval::AllowedDevice;
        client_->receive(); // clear vehicle_status for CorrectPIN
    }
    virtual void TearDown() {
        rdh_->stop();
        client_->disconnect();
        asp_->disconnect();
        thr.join();
    }
    void sendMobileInit(bool terms_accepted) {
        TCPMessage msg;
        msg.header = constructHeader(RD::MOBILE_INIT).dump();
        json body = templates_.getRawMobileInitTemplate();
        body["terms_accepted"] = terms_accepted;
        msg.body = body.dump();
        client_->send(msg);
    }
    void sendDeadmansHandle(bool enable_vehicle_motion) {
        TCPMessage msg;
        msg.header = constructHeader(RD::DEADMANS_HANDLE).dump();
        json body = templates_.getRawDeadmansHandleTemplate();
        body["response_to_challenge"] = 10;
        body["enable_vehicle_motion"] = enable_vehicle_motion;
        body["dmh_gesture_progress"] = 11;
        body["dmh_horizontal_touch"] = 20;
        body["dmh_vertical_touch"] = 30;
        body["crc_value"] = 40;
        msg.body = body.dump();
        client_->send(msg);
    }
    void sendManeuverInit(const std::string& maneuver) {
        TCPMessage msg;
        msg.header = constructHeader(RD::MANEUVER_INIT).dump();
        json body = templates_.getRawManeuverInitTemplate();
        body["maneuver"] = maneuver;
        msg.body = body.dump();
        client_->send(msg);
    }
    void sendCancelManeuver() {
        TCPMessage msg;
        msg.header = constructHeader(RD::CANCEL_MANEUVER).dump();
        client_->send(msg);
    }
    void sendCabinCommands(bool doors_locked, bool engine_off) {
        TCPMessage msg;
        msg.header = constructHeader(RD::CABIN_COMMANDS).dump();
        json body = templates_.getRawCabinCommandsTemplate();
        body["doors_locked"] = doors_locked;
        body["engine_off"] = engine_off;
        msg.body = body.dump();
        client_->send(msg);
    }
    std::shared_ptr<SignalHandler> sh_;
    std::shared_ptr<RemoteDeviceHandler> rdh_;
    std::shared_ptr<MobileClient> client_;
    std::shared_ptr<StubASP> asp_;
    std::thread thr;
    TemplateHandler templates_;
};

// if terms are accepted on mobile_init
// then ConnectionApproval should be set to AllowedDevice
TEST_F(TCMSignalTest, ConnectionApprovalTermsAccepted) {
    sh_->ConnectionApproval = TCM::ConnectionApproval::NotAllowedDevice;
    sendMobileInit(true);
    client_->receive(true);
    EXPECT_EQ(sh_->ConnectionApproval, TCM::ConnectionApproval::AllowedDevice);
}

// if terms are rejected on mobile_init
// then ConnectionApproval should remain set to NotAllowedDevice
TEST_F(TCMSignalTest, ConnectionApprovalTermsRejected) {
    sh_->ConnectionApproval = TCM::ConnectionApproval::NotAllowedDevice;
    sendMobileInit(false);
    client_->receive(true);
    EXPECT_EQ(sh_->ConnectionApproval, TCM::ConnectionApproval::NotAllowedDevice);
}

// if deadmans_handle is received in a selecting state,
// then ManeuverButtonPress should be set to ConfirmationSelected
TEST_F(TCMSignalTest, ManeuverButtonPressConfirmation) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::Selecting;
    sh_->ManeuverButtonPress = TCM::ManeuverButtonPress::None;
    sendDeadmansHandle(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::ConfirmationSelected);
}

// if deadmans_handle is received in an interrupted state,
// then ManeuverButtonPress should be set to ResumeSelected
TEST_F(TCMSignalTest, ManeuverButtonPressResume) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::Interrupted;
    sh_->ManeuverButtonPress = TCM::ManeuverButtonPress::None;
    sendDeadmansHandle(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::ResumeSelected);
}

// if cancel_maneuver is received
// then ManeuverButtonPress should be set to CancellationSelected
TEST_F(TCMSignalTest, ManeuverButtonPressCancellation) {
    sh_->ManeuverButtonPress = TCM::ManeuverButtonPress::None;
    sendCancelManeuver();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sh_->ManeuverStatus = ASP::ManeuverStatus::Cancelled; // simulate ASP state change
    client_->receive();
    EXPECT_EQ(sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::CancellationSelected);
}

// if cabin_commands sets engine_off and doors_locked to true
// then ManeuverButtonPress should be set to EndManouevre
TEST_F(TCMSignalTest, ManeuverButtonPressEndManouevre) {
    sh_->ManeuverButtonPress = TCM::ManeuverButtonPress::EndManouevre;
    sendCabinCommands(true, true);
    client_->receive();
    EXPECT_EQ(sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::EndManouevre);
}

// if deadmans_handle is sent with "enable_vehicle_motion" set to true
// then ManeuverEnableInput should be set to ValidScrnInput
TEST_F(TCMSignalTest, ManeuverEnableInputValid) {
    sh_->ManeuverEnableInput = TCM::ManeuverEnableInput::NoScrnInput;
    sendDeadmansHandle(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::ValidScrnInput);
}

// if deadmans_handle is sent with "enable_vehicle_motion" set to false
// then ManeuverEnableInput should not be set to NoScrnInput
TEST_F(TCMSignalTest, ManeuverEnableInputInvalid) {
    sh_->ManeuverEnableInput = TCM::ManeuverEnableInput::InvalidScrnInput;
    sendDeadmansHandle(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::NoScrnInput);
}

// if deadmans_handle is received
// then coordinate signals should be set to "dmh_horizontal_touch" and "dmh_vertical_touch"
TEST_F(TCMSignalTest, DMHCoordinates) {
    EXPECT_EQ(sh_->AppSliderPosX, 0);
    EXPECT_EQ(sh_->AppSliderPosY, 0);
    sendDeadmansHandle(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(sh_->AppAccelerationZ, 11);
    EXPECT_EQ(sh_->AppSliderPosX, 20);
    EXPECT_EQ(sh_->AppSliderPosY, 30);
}

// TCM should start up with no maneuver selected
TEST_F(TCMSignalTest, ManeuverSelectStartup) {
    EXPECT_EQ(sh_->ExploreModeSelect, TCM::ExploreModeSelect::NotPressed);
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::None);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Park);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::None);
}

// if maneuver_init is sent with "maneuver" set to "StrFwd",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectStrFwd) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrFwd");
    client_->receive();
    EXPECT_EQ(sh_->ExploreModeSelect, TCM::ExploreModeSelect::Pressed);
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::None);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Center);
}

// if maneuver_init is sent with "maneuver" set to "StrRvs",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectStrRvs) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrRvs");
    client_->receive();
    EXPECT_EQ(sh_->ExploreModeSelect, TCM::ExploreModeSelect::Pressed);
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::None);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Reverse);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Center);
}

// if maneuver_init is sent with "maneuver" set to "NdgFwd",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectNdgFwd) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::LongitudinalAssist;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("NdgFwd");
    client_->receive();
    EXPECT_EQ(sh_->NudgeSelect, TCM::NudgeSelect::NudgePressed);
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::None);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Center);
}

// if maneuver_init is sent with "maneuver" set to "NdgRvs",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectNdgRvs) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::LongitudinalAssist;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("NdgRvs");
    client_->receive();
    EXPECT_EQ(sh_->NudgeSelect, TCM::NudgeSelect::NudgePressed);
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::None);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Reverse);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Center);
}

// if maneuver_init is sent with "maneuver" set to "InLftFwd",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectInLftFwd) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("InLftFwd");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::NoseFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Left);
}

// if maneuver_init is sent with "maneuver" set to "InLftRvs",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectInLftRvs) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("InLftRvs");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::RearFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Reverse);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Left);
}

// if maneuver_init is sent with "maneuver" set to "InRgtFwd",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectInRgtFwd) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("InRgtFwd");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::NoseFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Right);
}

// if maneuver_init is sent with "maneuver" set to "InRgtRvs",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectInRgtRvs) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkIn;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("InRgtRvs");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::RearFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Reverse);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Right);
}

// if maneuver_init is sent with "maneuver" set to "OutLftFwd",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectOutLftFwd) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("OutLftFwd");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::NoseFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Left);
}

// if maneuver_init is sent with "maneuver" set to "OutLftRvs",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectOutLftRvs) {
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
    sendManeuverInit("OutLftRvs");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::RearFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Reverse);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Left);
}

// if maneuver_init is sent with "maneuver" set to "OutRgtFwd",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectOutRgtFwd) {
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
    sendManeuverInit("OutRgtFwd");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::NoseFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Right);
}

// if maneuver_init is sent with "maneuver" set to "OutRgtRvs",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectOutRgtRvs) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("OutRgtRvs");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Perpendicular);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::RearFirst);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Reverse);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Right);
}

// if maneuver_init is sent with "maneuver" set to "OutLftPrl",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectOutLftPrl) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("OutLftPrl");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Parallel);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Left);
}

// if maneuver_init is sent with "maneuver" set to "OutRgtPrl",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectOutRgtPrl) {
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ActiveParkingType = ASP::ActiveParkingType::Remote;
    sh_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
    sh_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
    sendManeuverInit("OutRgtPrl");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverTypeSelect, TCM::ManeuverTypeSelect::Parallel);
    EXPECT_EQ(sh_->ManeuverDirectionSelect, TCM::ManeuverDirectionSelect::None);
    EXPECT_EQ(sh_->ManeuverGearSelect, TCM::ManeuverGearSelect::Forward);
    EXPECT_EQ(sh_->ManeuverSideSelect, TCM::ManeuverSideSelect::Right);
}

// if maneuver_init is sent with "maneuver" set to "RtnToOgn",
// then corresponding TCM signals should be set accordingly
TEST_F(TCMSignalTest, ManeuverSelectRtnToOgn) {
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("RtnToOgn");
    client_->receive();
    EXPECT_EQ(sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::ReturnToStart);
}
