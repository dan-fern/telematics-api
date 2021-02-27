#include <gtest/gtest.h>

#include "testutils.hpp"
#include "remotedevicehandler.hpp"

class ASPSignalTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        asp_ = std::make_shared<StubASP>();
        sh_ = std::make_shared<SignalHandler>();
        rdh_ = std::make_shared<RemoteDeviceHandler>(sh_);
        thr = std::thread([this] { rdh_->spin(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        client_ = std::make_shared<MobileClient>();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    void sendMobileInit() {
        TCPMessage msg;
        msg.header = constructHeader(RD::MOBILE_INIT).dump();
        json body = templates_.getRawMobileInitTemplate();
        body["terms_accepted"] = true;
        msg.body = body.dump();
        client_->send(msg);
    }
    void sendListManeuvers() {
        TCPMessage msg;
        msg.header = constructHeader(RD::LIST_MANEUVERS).dump();
        client_->send(msg);
    }
    void sendGetThreatData() {
        TCPMessage msg;
        msg.header = constructHeader(RD::GET_THREAT_DATA).dump();
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
    std::shared_ptr<SignalHandler> sh_;
    std::shared_ptr<RemoteDeviceHandler> rdh_;
    std::shared_ptr<MobileClient> client_;
    std::shared_ptr<StubASP> asp_;
    std::thread thr;
    TemplateHandler templates_;
};

// if ActiveAutonomousFeature is set to Parking mode (and other conditions are met)
// then "ready" field in vehicle_init should be true
TEST_F(ASPSignalTest, ActiveAutonomousFeatureMobileInitReady) {
    sh_->ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking;
    sendMobileInit();
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_INIT);
    json reply_body = json::parse(reply.body);
    EXPECT_TRUE(reply_body["ready"]);
}

// if ActiveAutonomousFeature is set to no feature active (and other conditions are met)
// then "ready" field in vehicle_init should be false
TEST_F(ASPSignalTest, ActiveAutonomousFeatureMobileInitNotReady) {
    sh_->ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::NoFeatureActive;
    sendMobileInit();
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_INIT);
    json reply_body = json::parse(reply.body);
    EXPECT_FALSE(reply_body["ready"]);
}

// if CancelMsg changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, CancelMsg) {
    sh_->CancelMsg = ASP::CancelMsg::CancelledFromLossOfTraction;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body[ "status_2xx" ]["status_code"], 201);
    EXPECT_EQ(reply_body[ "status_2xx" ]["status_text"], "CancelledFromLossOfTraction");
    sh_->CancelMsg = ASP::CancelMsg::CancelledSpeedTooHigh;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body[ "status_2xx" ]["status_code"], 208);
    EXPECT_EQ(reply2_body[ "status_2xx" ]["status_text"], "CancelledSpeedTooHigh");
}

// if ConfirmAvailability is set to OfferEnabled
// then "status" in maneuver_status should be "confirmed"
TEST_F(ASPSignalTest, ConfirmAvailabilityManeuverInitConfirmed) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Confirming;
    sendManeuverInit("StrFwd");
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["status"], 903);
}

// if ConfirmAvailability is not set to OfferEnabled
// then "status" in maneuver_status should be "initiated"
TEST_F(ASPSignalTest, ConfirmAvailabilityManeuverInitUnconfirmed) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferDisabled;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Confirming;
    sendManeuverInit("StrRvs");
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["status"], 903);
}

// if NoFeatureAvailableMsg changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, NoFeatureAvailableMsg) {
    sh_->NoFeatureAvailableMsg = ASP::NoFeatureAvailableMsg::NotAvailableACCOn;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body[ "status_1xx" ]["status_code"], 111);
    EXPECT_EQ(reply_body[ "status_1xx" ]["status_text"], "NotAvailableACCOn");
    sh_->NoFeatureAvailableMsg = ASP::NoFeatureAvailableMsg::NotAvailableSpeedToohigh;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body[ "status_1xx" ]["status_code"], 107);
    EXPECT_EQ(reply2_body[ "status_1xx" ]["status_text"], "NotAvailableSpeedToohigh");
}

// if NoFeatureAvailableMsg changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, InfoMsg) {
    sh_->InfoMsg = ASP::InfoMsg::ApproachingMaximumDistance;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body[ "status_5xx" ]["status_code"], 525);
    EXPECT_EQ(reply_body[ "status_5xx" ]["status_text"], "ApproachingMaximumDistance");
    sh_->InfoMsg = ASP::InfoMsg::ConfirmAndReleaseBrakeToStart;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body[ "status_5xx" ]["status_code"], 511);
    EXPECT_EQ(reply2_body[ "status_5xx" ]["status_text"], "ConfirmAndReleaseBrakeToStart");
}

// if NoFeatureAvailableMsg changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, InstructMsg) {
    sh_->InstructMsg = ASP::InstructMsg::ContinueForward;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body[ "status_6xx" ]["status_code"], 608);
    EXPECT_EQ(reply_body[ "status_6xx" ]["status_text"], "ContinueForward");
    sh_->InstructMsg = ASP::InstructMsg::EngageParkBrake;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body[ "status_6xx" ]["status_code"], 612);
    EXPECT_EQ(reply2_body[ "status_6xx" ]["status_text"], "EngageParkBrake");
}

// if ActiveParkingType is set to PushPull (and other conditions are met)
// then "maneuver" field in maneuver_status should be set to appropriate maneuver string
TEST_F(ASPSignalTest, ActiveParkingTypePushPull) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrFwd");
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["maneuver"], "StrFwd");
}

// if ActiveParkingType is not set to PushPull
// then "maneuver" field in maneuver_status should be set to ""
// TODO: once additional maneuvers are supported, these tests should be updated
TEST_F(ASPSignalTest, ActiveParkingTypeInvalid) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::None;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrRvs");
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["maneuver"], "");
}

// if PauseMsg2 changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, PauseMsg2) {
    sh_->PauseMsg2 = ASP::PauseMsg2::ActivityKeyMissing;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body[ "status_4xx" ]["status_code"], 404);
    EXPECT_EQ(reply_body[ "status_4xx" ]["status_text"], "ActivityKeyMissing");
    sh_->PauseMsg2 = ASP::PauseMsg2::MaximumDistanceReached;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body[ "status_4xx" ]["status_code"], 405);
    EXPECT_EQ(reply2_body[ "status_4xx" ]["status_text"], "MaximumDistanceReached");
}

// if PauseMsg1 changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, PauseMsg1) {
    sh_->PauseMsg1 = ASP::PauseMsg1::PausedBootOpen;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body[ "status_3xx" ]["status_code"], 309);
    EXPECT_EQ(reply_body[ "status_3xx" ]["status_text"], "PausedBootOpen");
    sh_->PauseMsg1 = ASP::PauseMsg1::PausedPowerLow;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body[ "status_3xx" ]["status_code"], 315);
    EXPECT_EQ(reply2_body[ "status_3xx" ]["status_text"], "PausedPowerLow");
}

// if MobileChallengeSend changes
// then mobile_challenge should be sent
TEST_F(ASPSignalTest, MobileChallengeSend) {
    sh_->MobileChallengeSend = 0x1234567890abcdef;
    std::ostringstream challengeVal;
    challengeVal << sh_->MobileChallengeSend;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MOBILE_CHALLENGE);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["packed_bytes"], challengeVal.str( ));
}

// if ManeuverStatus changes
// then vehicle_status should be sent with corresponding "status_code" and "status_text"
TEST_F(ASPSignalTest, ManeuverStatus) {
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["progress"], 0);
    EXPECT_EQ(reply_body["status"], 904);
    sh_->ManeuverStatus = ASP::ManeuverStatus::Scanning;
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_EQ(reply2_body["progress"], 0);
    EXPECT_EQ(reply2_body["status"], 901);
}

// if ManeuverStatus changes from Scanning to Selecting
// then maneuver_status, threat_data, and available_maneuvers should be sent
TEST_F(ASPSignalTest, ManeuverStatusScanningToSelecting) {
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Scanning;
    sendGetThreatData();
    sendListManeuvers();
    sh_->ManeuverStatus = ASP::ManeuverStatus::Selecting;

    // expecting three replies - available_maneuvers, threat_data, and
    // maneuver_status; these can arrive in any order
    std::vector<TCPMessage> received_messages;
    for( int i = 0; i < 3; ++i )
    {
        received_messages.push_back( client_->receive( ) );
    }

    for( const auto& msg : received_messages )
    {
        json reply_header = json::parse( msg.header );
        json reply_body = json::parse( msg.body );
        if( reply_header[ "group" ] == RD::THREAT_DATA )
        {
            EXPECT_EQ( reply_header[ "group" ], (std::string)RD::THREAT_DATA );
        }
        else if( reply_header[ "group" ] == RD::MANEUVER_STATUS )
        {
            EXPECT_EQ( reply_header[ "group" ], (std::string)RD::MANEUVER_STATUS );
        }
        else if( reply_header[ "group" ] == RD::AVAILABLE_MANEUVERS )
        {
            EXPECT_EQ( reply_header[ "group" ], (std::string)RD::AVAILABLE_MANEUVERS );
        }
        else {
            FAIL( ); // received unexpected message type
        }
    }
}

// if ManeuverStatus changes from Selecting to Confirming
// then maneuver_status should be sent
TEST_F(ASPSignalTest, ManeuverStatusSelectingToConfirming) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Selecting;
    sendManeuverInit("StrFwd");
    sh_->ManeuverStatus = ASP::ManeuverStatus::Confirming;
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
}

// if ManeuverStatus changes from Maneuvering to Cancelled
// then maneuver_status should be sent
TEST_F(ASPSignalTest, ManeuverStatusManeuveringToCancelled) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    sendCancelManeuver();
    sh_->ManeuverStatus = ASP::ManeuverStatus::Cancelled;
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
}

// if ManeuverStatus changes to Finishing
// then maneuver_status should be sent
TEST_F(ASPSignalTest, ManeuverStatusFinishing) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::Finishing;
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
}

// if ManeuverStatus changes to Ended
// then vehicle_status should be sent
TEST_F(ASPSignalTest, ManeuverStatusEnded) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::Ended;
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
}

// if ManeuverStatus is not set to a status that indicates maneuver is in progress
// then "active_maneuver" field in vehicle_init should be false
TEST_F(ASPSignalTest, ManeuverStatusVehicleInit) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::NotActive;
    sendMobileInit();
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_INIT);
    json reply_body = json::parse(reply.body);
    EXPECT_FALSE(reply_body["active_maneuver"]);
}

// if ManeuverStatus is set to a status that indicates maneuver is in progress
// then "active_maneuver" field in vehicle_init should be true
TEST_F(ASPSignalTest, ManeuverStatusVehicleInitManeuverInProgress) {
    sh_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    sendMobileInit();

    // expecting two statuses - vehicle_init and maneuver_status
    // these can arrive in any order
    json reply_header, reply_body;
    std::vector<TCPMessage> received_messages;
    for( int i = 0; i < 2; ++i )
    {
        received_messages.push_back( client_->receive( ) );
    }

    for( const auto& status : received_messages )
    {
        reply_header = json::parse( status.header );
        reply_body = json::parse( status.body );
        if( reply_header[ "group" ] == RD::VEHICLE_INIT )
        {
            EXPECT_EQ( reply_header["group"], (std::string)RD::VEHICLE_INIT );
            EXPECT_TRUE( reply_body["active_maneuver"] );
        }
        else if( reply_header[ "group" ] == RD::MANEUVER_STATUS )
        {
            EXPECT_EQ( reply_header["group"], (std::string)RD::MANEUVER_STATUS );
            EXPECT_EQ( reply_body["maneuver"], "" );
            EXPECT_EQ( reply_body["status"], 904 );
        }
        else {
            FAIL( ); // received unexpected message type
        }
    }


}

// if ManeuverStatus is set to Maneuvering
// then "status" field in maneuver_status should be set to "initiated"
TEST_F(ASPSignalTest, ManeuverStatusManeuverStatusManeuvering) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrRvs");
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["status"], 904);
}

// if ManeuverStatus is set to Cancelled
// then "status" field in maneuver_status should be set to "cancelled"
TEST_F(ASPSignalTest, ManeuverStatusManeuverStatusCancelled) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    sh_->ManeuverStatus = ASP::ManeuverStatus::Cancelled;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrFwd");
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["status"], 910);
}

// ManeuverProgressBar should pass through to RD in a maneuver_status
TEST_F(ASPSignalTest, ManeuverProgressBar) {
    sh_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    sh_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    sh_->ManeuverProgressBar = 77;
    sh_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    sendManeuverInit("StrRvs");
    struct TCPMessage reply = client_->receive(true);
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["progress"], 77);
}

// tests all combinations of signals used in available_maneuvers reply:
//     ExploreModeAvailability, ManeuverDirectionAvailability, LongitudinalAdjustAvailability,
//     ReturnToStartAvailability, ActiveParkingMode, ActiveManeuverOrientation, ManeuverSideAvailability
// see doxygen for sendAvailableManeuvers_() function for details
TEST_F(ASPSignalTest, AvailableManeuversAllBranches) {
    for (int lmpushpullmnvrrmt = 1; lmpushpullmnvrrmt <= 2; ++lmpushpullmnvrrmt) {
    for (int ManeuverDirectionAvailability = 1; ManeuverDirectionAvailability <= 3; ++ManeuverDirectionAvailability) {
    for (int lmlngadjstmnvrrmt = 1; lmlngadjstmnvrrmt <= 2; ++lmlngadjstmnvrrmt) {
    for (int ReturnToStartAvailability = 1; ReturnToStartAvailability <= 2; ++ReturnToStartAvailability) {
    for (int ActiveParkingMode = 1; ActiveParkingMode <= 2; ++ActiveParkingMode) {
    for (int ActiveManeuverOrientation = 1; ActiveManeuverOrientation <= 3; ++ActiveManeuverOrientation) {
    for (int ManeuverSideAvailability = 1; ManeuverSideAvailability <= 3; ++ManeuverSideAvailability) {
        sh_->ExploreModeAvailability = (ASP::ExploreModeAvailability)lmpushpullmnvrrmt;
        sh_->ManeuverDirectionAvailability = (ASP::ManeuverDirectionAvailability)ManeuverDirectionAvailability;
        sh_->LongitudinalAdjustAvailability = (ASP::LongitudinalAdjustAvailability)lmlngadjstmnvrrmt;
        sh_->ReturnToStartAvailability = (ASP::ReturnToStartAvailability)ReturnToStartAvailability;
        sh_->ActiveParkingMode = (ASP::ActiveParkingMode)ActiveParkingMode;
        sh_->ActiveManeuverOrientation = (ASP::ActiveManeuverOrientation)ActiveManeuverOrientation;
        sh_->ManeuverSideAvailability = (ASP::ManeuverSideAvailability)ManeuverSideAvailability;
        sh_->hasVehicleMoved = true;
        sendListManeuvers();
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::AVAILABLE_MANEUVERS);

        json reply_body = json::parse(reply.body);
        EXPECT_EQ( reply_body["default"], "" );
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkOut &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularFront &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["OutLftFwd"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["OutLftFwd"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkOut &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularRear &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["OutLftRvs"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["OutLftRvs"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkOut &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularFront &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferRightOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["OutRgtFwd"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["OutRgtFwd"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkOut &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularRear &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferRightOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["OutRgtRvs"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["OutRgtRvs"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkIn &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularFront &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["InLftFwd"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["InLftFwd"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkIn &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularRear &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["InLftRvs"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["InLftRvs"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkIn &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularFront &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferRightOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["InRgtFwd"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["InRgtFwd"]);
        }
        if (ActiveParkingMode == (int)ASP::ActiveParkingMode::ParkIn &&
            ActiveManeuverOrientation == (int)ASP::ActiveManeuverOrientation::PerpendicularRear &&
            (ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferRightOnly ||
             ManeuverSideAvailability == (int)ASP::ManeuverSideAvailability::OfferLeftAndRight))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["InRgtRvs"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["InRgtRvs"]);
        }
        if (lmlngadjstmnvrrmt == (int)ASP::LongitudinalAdjustAvailability::OfferEnabled &&
            (ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferForward ||
             ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferForwardAndBackward))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["NdgFwd"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["NdgFwd"]);
        }
        if (lmlngadjstmnvrrmt == (int)ASP::LongitudinalAdjustAvailability::OfferEnabled &&
            (ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferBackward ||
             ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferForwardAndBackward))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["NdgRvs"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["NdgRvs"]);
        }
        if (lmpushpullmnvrrmt == (int)ASP::ExploreModeAvailability::OfferEnabled &&
            (ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferForward ||
             ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferForwardAndBackward))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["StrFwd"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["StrFwd"]);
        }
        if (lmpushpullmnvrrmt == (int)ASP::ExploreModeAvailability::OfferEnabled &&
            (ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferBackward ||
             ManeuverDirectionAvailability == (int)ASP::ManeuverDirectionAvailability::OfferForwardAndBackward))
        {
            EXPECT_TRUE(reply_body["maneuvers"]["StrRvs"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["StrRvs"]);
        }
        if (ReturnToStartAvailability == (int)ASP::ReturnToStartAvailability::OfferEnabled)
        {
            EXPECT_TRUE(reply_body["maneuvers"]["RtnToOgn"]);
        }
        else {
            EXPECT_FALSE(reply_body["maneuvers"]["RtnToOgn"]);
        }
        if( lmpushpullmnvrrmt == (int)ASP::ExploreModeAvailability::OfferDisabled )
        {
            EXPECT_FALSE( reply_body["continue_exploring"] );
        }
        else
        {
            EXPECT_TRUE( reply_body["continue_exploring"] );
        }

    }
    }
    }
    }
    }
    }
    }
}

TEST_F(ASPSignalTest, ThreatDataAllSignals) {
    int i = 0;
    sh_->threatDistanceData.ASPMFrontSegDist1RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist2RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist3RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist4RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist5RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist6RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist7RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist8RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist9RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist10RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist11RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist12RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist13RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist14RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist15RMT = ++i;
    sh_->threatDistanceData.ASPMFrontSegDist16RMT = ++i;
    i = 20;
    sh_->threatDistanceData.ASPMRearSegDist1RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist2RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist3RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist4RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist5RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist6RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist7RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist8RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist9RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist10RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist11RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist12RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist13RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist14RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist15RMT = --i;
    sh_->threatDistanceData.ASPMRearSegDist16RMT = --i;

    sh_->threatTypeData.ASPMFrontSegType1RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType2RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType3RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType4RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType5RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType6RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType7RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType8RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType9RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType10RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType11RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType12RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType13RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType14RMT = 1;
    sh_->threatTypeData.ASPMFrontSegType15RMT = 0;
    sh_->threatTypeData.ASPMFrontSegType16RMT = 1;

    sh_->threatTypeData.ASPMRearSegType1RMT = 1;
    sh_->threatTypeData.ASPMRearSegType2RMT = 0;
    sh_->threatTypeData.ASPMRearSegType3RMT = 1;
    sh_->threatTypeData.ASPMRearSegType4RMT = 0;
    sh_->threatTypeData.ASPMRearSegType5RMT = 1;
    sh_->threatTypeData.ASPMRearSegType6RMT = 0;
    sh_->threatTypeData.ASPMRearSegType7RMT = 1;
    sh_->threatTypeData.ASPMRearSegType8RMT = 0;
    sh_->threatTypeData.ASPMRearSegType9RMT = 1;
    sh_->threatTypeData.ASPMRearSegType10RMT = 0;
    sh_->threatTypeData.ASPMRearSegType11RMT = 1;
    sh_->threatTypeData.ASPMRearSegType12RMT = 0;
    sh_->threatTypeData.ASPMRearSegType13RMT = 1;
    sh_->threatTypeData.ASPMRearSegType14RMT = 0;
    sh_->threatTypeData.ASPMRearSegType15RMT = 1;
    sh_->threatTypeData.ASPMRearSegType16RMT = 0;

    sendGetThreatData();

    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::THREAT_DATA);
    json reply_body = json::parse(reply.body);
    ASSERT_TRUE(reply_body["threats"].is_array());
    ASSERT_EQ(reply_body["threats"].size(), 32);
    for(int i = 0; i < reply_body["threats"].size(); ++i) {
        auto threat_int = reply_body["threats"][i];
        // every other threat should be 19 (SegTypeXX is 0)
        if (i % 2 == 0) {
            EXPECT_EQ(threat_int, 19);
        }
        // first half of threats are the front signals (2-16)
        else if (i < (reply_body["threats"].size() / 2)) {
            EXPECT_EQ(threat_int, i + 1);
        }
        // second half of threats are the rear signals (5-19)
        else {
            EXPECT_EQ(threat_int, i - 16 + 4);
        }
    }
}
