#include <gtest/gtest.h>

#include "templatehandler.hpp"
#include "remotedevicehandler.hpp"
#include "testutils.hpp"
#include "aspm.hpp"
#include "picosha2.h"

using json = nlohmann::json;
using namespace std::placeholders;

class MobileCommsTest: public ::testing::Test {
protected:
    virtual void SetUp() {
        asp_ = std::make_shared<StubASP>();
        sh_ = std::make_shared<SignalHandler>();
        sh_->ActiveAutonomousFeature = ASP::ActiveAutonomousFeature::Parking; // test RPA state only
        server_ = std::make_shared<RemoteDeviceHandler>(sh_);
        thr = std::thread([this] { server_->spin(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        client_ = std::make_shared<MobileClient>();
        srand(time(NULL));
    }
    virtual void TearDown() {
        server_->stop();
        client_->disconnect();
        asp_->disconnect();
        thr.join();
    }
    void sendCorrectPIN() {
        TCPMessage msg;
        msg.header = constructHeader(RD::SEND_PIN).dump();
        json body = templates_.getRawSendPINTemplate();
        body["pin"] = picosha2::hash256_hex_string( std::string( DCM::POC_PIN ) );
        msg.body = body.dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_STATUS);
        json reply_body = json::parse(reply.body);
        EXPECT_EQ( reply_body[ "status_7xx" ]["status_code"], 708 );
    }
    void sendCorrectMobileInit() {
        sendCorrectPIN();
        TCPMessage msg;
        msg.header = constructHeader(RD::MOBILE_INIT).dump();
        json body2 = templates_.getRawMobileInitTemplate();
        body2["terms_accepted"] = true;
        msg.body = body2.dump();
        client_->send(msg);
        struct TCPMessage reply2 = client_->receive();
        json reply_header2 = json::parse(reply2.header);
        EXPECT_EQ(reply_header2["group"], (std::string)RD::VEHICLE_INIT);
        json reply_body2 = json::parse(reply2.body);
        EXPECT_FALSE(reply_body2["active_maneuver"]);
        EXPECT_TRUE(reply_body2["ready"]);
    }
    json sendListManeuversPushPull( )
    {
        asp_->ExploreModeAvailability = ASP::ExploreModeAvailability::OfferEnabled;
        asp_->ManeuverDirectionAvailability = ASP::ManeuverDirectionAvailability::OfferForwardAndBackward;
        asp_->sync();
        TCPMessage msg;
        msg.header = constructHeader(RD::LIST_MANEUVERS).dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::AVAILABLE_MANEUVERS);
        json reply_body = json::parse(reply.body);
        EXPECT_TRUE(reply_body["maneuvers"]["StrFwd"]);
        EXPECT_TRUE(reply_body["maneuvers"]["StrRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["RtnToOgn"]);
        return reply_body;
    }
    json sendListManeuversParkOutRvs( )
    {
        asp_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
        asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
        asp_->ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
        sh_->hasVehicleMoved = true;
        asp_->sync();
        TCPMessage msg;
        msg.header = constructHeader(RD::LIST_MANEUVERS).dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::AVAILABLE_MANEUVERS);
        json reply_body = json::parse(reply.body);\
        EXPECT_FALSE(reply_body["maneuvers"]["StrFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["StrRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftFwd"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtFwd"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["RtnToOgn"]);
        return reply_body;
    }
    json sendListManeuversParkOutFwd( )
    {
        asp_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
        asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
        asp_->ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
        sh_->hasVehicleMoved = true;
        asp_->sync();
        TCPMessage msg;
        msg.header = constructHeader(RD::LIST_MANEUVERS).dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::AVAILABLE_MANEUVERS);
        json reply_body = json::parse(reply.body);
        EXPECT_FALSE(reply_body["maneuvers"]["StrFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["StrRvs"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftRvs"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["RtnToOgn"]);
        return reply_body;
    }
    json sendListManeuversParkOutPrl( )
    {
        asp_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
        asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
        asp_->ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
        sh_->hasVehicleMoved = true;
        asp_->sync();
        TCPMessage msg;
        msg.header = constructHeader(RD::LIST_MANEUVERS).dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::AVAILABLE_MANEUVERS);
        json reply_body = json::parse(reply.body);
        EXPECT_FALSE(reply_body["maneuvers"]["StrFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["StrRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["OutRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgRvs"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutLftPrl"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutRgtPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["RtnToOgn"]);
        return reply_body;
    }
    json sendListManeuversParkOutBypass( )
    {
        asp_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
        asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
        asp_->ManeuverSideAvailability = ASP::ManeuverSideAvailability::OfferLeftAndRight;
        sh_->hasVehicleMoved = false;
        asp_->sync();
        TCPMessage msg;
        msg.header = constructHeader(RD::LIST_MANEUVERS).dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::AVAILABLE_MANEUVERS);
        json reply_body = json::parse(reply.body);
        EXPECT_FALSE(reply_body["maneuvers"]["StrFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["StrRvs"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutLftFwd"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutLftRvs"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutRgtFwd"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InLftRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["InRgtRvs"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgFwd"]);
        EXPECT_FALSE(reply_body["maneuvers"]["NdgRvs"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutLftPrl"]);
        EXPECT_TRUE(reply_body["maneuvers"]["OutRgtPrl"]);
        EXPECT_FALSE(reply_body["maneuvers"]["RtnToOgn"]);
        return reply_body;
    }
    void sendGetThreatData( )
    {
        asp_->threatDistanceData.ASPMFrontSegDist2RMT = 3;
        asp_->threatTypeData.ASPMFrontSegType2RMT = 1;
        asp_->threatDistanceData.ASPMRearSegDist2RMT = 3;
        asp_->threatTypeData.ASPMRearSegType2RMT = 1;
        asp_->sync();
        TCPMessage msg;
        msg.header = constructHeader(RD::GET_THREAT_DATA).dump();
        client_->send(msg);
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::THREAT_DATA);
        json reply_body = json::parse(reply.body);
        ASSERT_TRUE(reply_body["threats"].is_array());
        ASSERT_EQ(reply_body["threats"].size(), 32);
        for(int i = 0; i < reply_body["threats"].size(); ++i) {
            auto threat_int = reply_body["threats"][i];
            EXPECT_TRUE( 0 <= threat_int <= 19 );
            if (i == 1 || i == 30) {
                EXPECT_EQ(threat_int, 3);
            }
        }
    }
    void sendManeuverInitPushPull( std::string& maneuverName )
    {
        asp_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
        if( maneuverName == "StrFwd" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
        }
        else if( maneuverName == "StrRvs" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
        }
        else
        {
            FAIL( ); // received unexpected maneuver name
        }
        asp_->ManeuverStatus = ASP::ManeuverStatus::Confirming;
        asp_->ManeuverProgressBar = 0;
        asp_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
        asp_->sync();

        // state change - ManeuverStatus -> Confirming
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
        json reply_body = json::parse(reply.body);
        EXPECT_EQ(reply_body["status"], 903);

        // maneuver_init -> maneuver_status
        TCPMessage msg;
        msg.header = constructHeader(RD::MANEUVER_INIT).dump();
        json body = templates_.getRawManeuverInitTemplate();
        body["maneuver"] = maneuverName;
        msg.body = body.dump();
        client_->send(msg);
        reply = client_->receive();
        reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
        reply_body = json::parse(reply.body);
        EXPECT_EQ(reply_body["maneuver"], maneuverName);
        EXPECT_EQ(reply_body["progress"], 0);
        EXPECT_EQ(reply_body["status"], 903);
    }
    void sendManeuverInitParkOut( std::string& maneuverName )
    {
        asp_->ActiveParkingType = ASP::ActiveParkingType::Remote;
        asp_->ActiveParkingMode = ASP::ActiveParkingMode::ParkOut;
        if( maneuverName == "OutLftFwd" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
            asp_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
        }
        else if( maneuverName == "OutLftRvs" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
            asp_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
        }
        else if( maneuverName == "OutRgtFwd" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
            asp_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
        }
        else if( maneuverName == "OutRgtRvs" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
            asp_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
        }
        else if( maneuverName == "OutLftPrl" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
            asp_->ActiveManeuverSide = ASP::ActiveManeuverSide::Left;
        }
        else if( maneuverName == "OutRgtPrl" )
        {
            asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::Parallel;
            asp_->ActiveManeuverSide = ASP::ActiveManeuverSide::Right;
        }
        else
        {
            FAIL( ); // received unexpected maneuver name
        }
        asp_->ManeuverStatus = ASP::ManeuverStatus::Confirming;
        asp_->ManeuverProgressBar = 0;
        asp_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
        asp_->sync();

        // state change - ManeuverStatus -> Confirming
        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
        json reply_body = json::parse(reply.body);
        EXPECT_EQ(reply_body["status"], 903);

        // maneuver_init -> maneuver_status
        TCPMessage msg;
        msg.header = constructHeader(RD::MANEUVER_INIT).dump();
        json body = templates_.getRawManeuverInitTemplate();
        body["maneuver"] = maneuverName;
        msg.body = body.dump();
        client_->send(msg);
        reply = client_->receive();
        reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
        reply_body = json::parse(reply.body);
        EXPECT_EQ(reply_body["maneuver"], maneuverName);
        EXPECT_EQ(reply_body["progress"], 0);
        EXPECT_EQ(reply_body["status"], 903);
    }
    void completeManeuver( std::string& testManeuver )
    {
        asp_->ManeuverStatus = ASP::ManeuverStatus::Finishing;
        asp_->ManeuverProgressBar = 100;
        asp_->sync( );

        struct TCPMessage reply = client_->receive( );
        json reply_header = json::parse( reply.header );
        json reply_body = json::parse( reply.body );
        EXPECT_EQ( reply_header[ "group" ], (std::string)RD::MANEUVER_STATUS );
        EXPECT_EQ( reply_body[ "maneuver" ], testManeuver );
        EXPECT_EQ( reply_body[ "progress" ], 100 );
        EXPECT_EQ( reply_body[ "status" ], 906 );
    }
    void sendMobileChallengeAndResponse(  )
    {

        asp_->MobileChallengeSend = 87654321;
        asp_->sync();

        struct TCPMessage reply = client_->receive();
        json reply_header = json::parse(reply.header);
        EXPECT_EQ(reply_header["group"], (std::string)RD::MOBILE_CHALLENGE);
        json reply_body = json::parse(reply.body);
        EXPECT_EQ(reply_body["packed_bytes"], "87654321");

        TCPMessage msg;
        msg.header = constructHeader(RD::MOBILE_RESPONSE).dump();
        json body = templates_.getRawMobileResponseTemplate();
        body["response_to_challenge"] = reply_body["packed_bytes"];
        msg.body = body.dump();
        client_->send(msg);

        asp_->sync();
        EXPECT_EQ(asp_->MobileChallengeReply, asp_->MobileChallengeSend);

    }
    std::shared_ptr<MobileClient> client_;
    std::shared_ptr<SignalHandler> sh_;
    std::shared_ptr<RemoteDeviceHandler> server_;
    std::shared_ptr<StubASP> asp_;
    std::thread thr;
    TemplateHandler templates_;
};

TEST_F(MobileCommsTest, GetAPIVersion) {
    TCPMessage msg;
    msg.header = constructHeader(RD::GET_API_VERSION).dump();
    client_->send(msg);
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_API_VERSION);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["api_version"], API_DOC_VERSION);
}

TEST_F( MobileCommsTest, NoPinInVDC )
{
    sh_->AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::NotSetInDCM;
    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );

    // send get_api_version to trigger a 707 response
    TCPMessage msg;
    msg.header = constructHeader( RD::GET_API_VERSION ).dump( );
    client_->send( msg );

    // expecting two statuses - vehicle_status and api_version in any order
    std::vector<TCPMessage> received_statuses;
    for( int i = 0; i < 2; ++i )
    {
        received_statuses.push_back( client_->receive( ) );
    }

    json status_header;
    json status_body;
    for( const auto& status : received_statuses )
    {
        status_header = json::parse( status.header );
        status_body = json::parse( status.body );
        if( status_header[ "group" ] == RD::VEHICLE_STATUS )
        {
            EXPECT_EQ( status_body[ "status_1xx" ][ "status_code" ], 100 );
            EXPECT_EQ( status_body[ "status_2xx" ][ "status_code" ], 200 );
            EXPECT_EQ( status_body[ "status_3xx" ][ "status_code" ], 300 );
            EXPECT_EQ( status_body[ "status_4xx" ][ "status_code" ], 400 );
            EXPECT_EQ( status_body[ "status_5xx" ][ "status_code" ], 500 );
            EXPECT_EQ( status_body[ "status_6xx" ][ "status_code" ], 600 );
            EXPECT_EQ( status_body[ "status_7xx" ][ "status_code" ], 707 );
            EXPECT_EQ( status_body[ "status_8xx" ][ "status_code" ], 800 );
            EXPECT_EQ( status_body[ "status_9xx" ][ "status_code" ], 900 );
        }
        else if( status_header[ "group" ] == RD::VEHICLE_API_VERSION )
        {
            EXPECT_EQ( status_body[ "api_version" ], API_DOC_VERSION );
        }
        else {
            FAIL( ); // received unexpected message type
        }
    }
}

TEST_F( MobileCommsTest, SendCorrectPin )
{
    sendCorrectPIN();
    client_->disconnect( );
    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
    EXPECT_EQ( sh_->AcknowledgeRemotePIN, DCM::AcknowledgeRemotePIN::ExpiredPIN );
}

TEST_F( MobileCommsTest, SendIncorrectPin )
{
    TCPMessage msg;
    msg.header = constructHeader( RD::SEND_PIN ).dump( );
    json body = templates_.getRawSendPINTemplate( );
    body[ "pin" ] = "INCORRECT_PIN";
    msg.body = body.dump( );

    // 1st attempt
    client_->send( msg );
    struct TCPMessage reply = client_->receive( );
    json reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    json reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ]["status_code"], 701 );

    // 2nd attempt
    client_->send( msg );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ][ "status_code" ], 701 );

    // 3rd attempt locks out
    client_->send( msg );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ][ "status_code" ], 702 );

    // once locked out, even a valid PIN should still return an error.
    body["pin"] = picosha2::hash256_hex_string( std::string( DCM::POC_PIN ) );
    msg.body = body.dump();
    client_->send(msg);
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ][ "status_code" ], 702 );

    // check other timeouts as well.
    sh_->AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock300s;
    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ][ "status_code" ], 703 );

    sh_->AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock3600s;
    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ][ "status_code" ], 704 );

    sh_->AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLockIndefinite;
    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    reply_body = json::parse( reply.body );
    EXPECT_EQ( reply_body[ "status_7xx" ][ "status_code" ], 705 );
}


// If mobile_init is received before send_pin, ready should be false
TEST_F(MobileCommsTest, MobileInitBeforePIN) {
    TCPMessage msg;
    msg.header = constructHeader(RD::MOBILE_INIT).dump();
    json body = templates_.getRawMobileInitTemplate();
    body["terms_accepted"] = true;
    msg.body = body.dump();
    client_->send(msg);
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::VEHICLE_INIT);
    json reply_body = json::parse(reply.body);
    EXPECT_FALSE(reply_body["active_maneuver"]);
    EXPECT_FALSE(reply_body["ready"]);
}

// If mobile_init is received after valid send_pin, ready should be true
TEST_F(MobileCommsTest, MobileInitAfterPIN) {
    sendCorrectMobileInit();
}

// If mobile_init is received during a manuever, ready and active_maneuver should be true
TEST_F(MobileCommsTest, MobileInitDuringManeuver) {
    sendCorrectMobileInit();
    asp_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    asp_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    asp_->ManeuverProgressBar = 60;
    asp_->sync();

    // state change - ManeuverStatus -> Maneuvering
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::MANEUVER_STATUS );
    EXPECT_EQ( reply_body[ "maneuver" ], "StrFwd" );
    EXPECT_EQ( reply_body[ "progress" ], 60 );
    EXPECT_EQ( reply_body[ "status" ], 904 );

    // mobile_init -> vehicle_init
    TCPMessage msg;
    msg.header = constructHeader(RD::MOBILE_INIT).dump();
    json body = templates_.getRawMobileInitTemplate();
    body["terms_accepted"] = true;
    msg.body = body.dump();
    client_->send(msg);

    // expecting two statuses - vehicle_init and maneuver_status
    // these can arrive in any order
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
            EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_INIT );
            EXPECT_TRUE( reply_body[ "active_maneuver" ] );
            EXPECT_TRUE( reply_body[ "ready" ] );
        }
        else if( reply_header[ "group" ] == RD::MANEUVER_STATUS )
        {
            EXPECT_EQ( reply_header[ "group" ], (std::string)RD::MANEUVER_STATUS );
            EXPECT_EQ( reply_body[ "maneuver" ], "StrFwd" );
            EXPECT_EQ( reply_body[ "progress" ], 60 );
            EXPECT_EQ( reply_body[ "status" ], 904 );
        }
        else {
            FAIL( ); // received unexpected message type
        }
    }

}

TEST_F( MobileCommsTest, MobileInitParkIn )
{

    std::vector<std::string> test_moves;
    test_moves.push_back( "InLftFwd" );
    test_moves.push_back( "InLftRvs" );
    test_moves.push_back( "InRgtFwd" );
    test_moves.push_back( "InRgtRvs" );

    sendCorrectMobileInit( );

    for( int i = 0; i < 4; ++i )
    {
        asp_->ManeuverStatus = ASP::ManeuverStatus::Selecting;
        asp_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
        asp_->changePreferredSimMove( test_moves[ i ] );
        asp_->sync( );

        TCPMessage msg;
        msg.header = constructHeader( RD::MOBILE_INIT ).dump( );
        json body2 = templates_.getRawMobileInitTemplate( );
        body2[ "terms_accepted" ] = true;
        msg.body = body2.dump( );
        client_->send( msg );

        // expecting two statuses -- vehicle_init and maneuver_status
        // these can arrive in any order
        std::vector<TCPMessage> received_messages;
        for( int j = 0; j < 2; ++j )
        {
            received_messages.push_back( client_->receive( ) );
        }

        for( const auto& status : received_messages )
        {
            json reply_header = json::parse( status.header );
            json reply_body = json::parse( status.body );
            if( reply_header[ "group" ] == RD::VEHICLE_INIT )
            {
                EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_INIT );
                EXPECT_TRUE( reply_body[ "active_maneuver" ] );
                EXPECT_TRUE( reply_body[ "ready" ] );
            }
            else if( reply_header[ "group" ] == RD::MANEUVER_STATUS )
            {
                EXPECT_EQ( reply_header[ "group" ], (std::string)RD::MANEUVER_STATUS );
                EXPECT_EQ( reply_body[ "maneuver" ], test_moves[ i ] );
                EXPECT_EQ( reply_body[ "progress" ], 0 );
                EXPECT_EQ( reply_body[ "status" ], 902 );
            }
            else
            {
                FAIL( ); // received unexpected message type
            }
        }
    }
}


TEST_F( MobileCommsTest, StatusEnded )
{
    asp_->ManeuverStatus = ASP::ManeuverStatus::Ended;
    asp_->sync();
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::VEHICLE_STATUS );
    json reply_body = json::parse(reply.body);
    EXPECT_EQ( reply_body[ "status_9xx" ][ "status_code" ], 907 );
}

TEST_F(MobileCommsTest, ListManeuversPushPull) {
    sendCorrectMobileInit();
    json available_maneuvers = sendListManeuversPushPull( );
    EXPECT_EQ(available_maneuvers["default"], "");
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );
}

TEST_F(MobileCommsTest, ListManeuversParkOutFwd) {
    sendCorrectMobileInit();
    json available_maneuvers = sendListManeuversParkOutFwd( );
    EXPECT_EQ(available_maneuvers["default"], "");
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );
}

TEST_F(MobileCommsTest, ListManeuversParkOutRvs) {
    sendCorrectMobileInit();
    json available_maneuvers = sendListManeuversParkOutRvs( );
    EXPECT_EQ(available_maneuvers["default"], "");
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );
}

TEST_F(MobileCommsTest, ListManeuversParkOutPrl) {
    sendCorrectMobileInit();
    json available_maneuvers = sendListManeuversParkOutPrl( );
    EXPECT_EQ(available_maneuvers["default"], "");
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );
}

TEST_F(MobileCommsTest, ListManeuversParkOutBypass) {
    sendCorrectMobileInit();
    json available_maneuvers = sendListManeuversParkOutBypass( );
    EXPECT_EQ(available_maneuvers["default"], "");
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );
}

TEST_F(MobileCommsTest, GetThreatData) {
    sendCorrectMobileInit();
    sendGetThreatData( );
}

TEST_F(MobileCommsTest, ManeuverInitSF) {
    std::string testManeuver = "StrFwd";
    sendCorrectMobileInit();
    sendManeuverInitPushPull( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitSR) {
    std::string testManeuver = "StrRvs";
    sendCorrectMobileInit();
    sendManeuverInitPushPull( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitPOLF) {
    std::string testManeuver = "OutLftFwd";
    sendCorrectMobileInit();
    sendManeuverInitParkOut( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitPOLR) {
    std::string testManeuver = "OutLftRvs";
    sendCorrectMobileInit();
    sendManeuverInitParkOut( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitPORF) {
    std::string testManeuver = "OutRgtFwd";
    sendCorrectMobileInit();
    sendManeuverInitParkOut( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitPORR) {
    std::string testManeuver = "OutRgtRvs";
    sendCorrectMobileInit();
    sendManeuverInitParkOut( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitPOLP) {
    std::string testManeuver = "OutLftPrl";
    sendCorrectMobileInit();
    sendManeuverInitParkOut( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitPORP) {
    std::string testManeuver = "OutLftPrl";
    sendCorrectMobileInit();
    sendManeuverInitParkOut( testManeuver );
}

TEST_F(MobileCommsTest, ManeuverInitDuringManeuver) {
    sendCorrectMobileInit();
    asp_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularRear;
    asp_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    asp_->ManeuverProgressBar = 50;
    asp_->sync();

    // state change - ManeuverStatus -> Maneuvering
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ( reply_body[ "maneuver" ], "StrRvs" );
    EXPECT_EQ( reply_body[ "progress" ], 50 );
    EXPECT_EQ(reply_body["status"], 904);

    // maneuver_init -> maneuver_status
    TCPMessage msg;
    msg.header = constructHeader(RD::MANEUVER_INIT).dump();
    json body = templates_.getRawManeuverInitTemplate();
    body["maneuver"] = "StrRvs";
    msg.body = body.dump();
    client_->send(msg);
    reply = client_->receive();
    reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["maneuver"], "StrRvs");
    EXPECT_EQ(reply_body["progress"], 50);
    EXPECT_EQ(reply_body["status"], 904);
}

TEST_F(MobileCommsTest, DeadmansHandle)
{
    sh_->ManeuverStatus = ASP::ManeuverStatus::Selecting;
    TCPMessage msg;
    msg.header = constructHeader( RD::DEADMANS_HANDLE ).dump( );
    json body = templates_.getRawDeadmansHandleTemplate( );
    body[ "enable_vehicle_motion" ] = true;
    body[ "dmh_gesture_progress" ] = 20.02;
    body[ "dmh_horizontal_touch" ] = 30.03;
    body[ "dmh_vertical_touch" ] = 40.04;
    body[ "crc_value" ] = 50.05;
    msg.body = body.dump( );
    client_->send( msg );

    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
    EXPECT_EQ( sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::ValidScrnInput );
    EXPECT_EQ( sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::ConfirmationSelected );

    asp_->sync( );
    EXPECT_EQ( asp_->AppAccelerationZ, 20 );
    EXPECT_EQ( asp_->AppSliderPosX, 30 );
    EXPECT_EQ( asp_->AppSliderPosY, 40 );
    EXPECT_EQ( asp_->AppCalcCheck, 50 );
    EXPECT_EQ( asp_->ManeuverEnableInput, TCM::ManeuverEnableInput::ValidScrnInput );
    EXPECT_EQ( asp_->ManeuverButtonPress, TCM::ManeuverButtonPress::ConfirmationSelected );
}

TEST_F( MobileCommsTest, DeadmansHandleAlt )
{
    sh_->ManeuverStatus = ASP::ManeuverStatus::Interrupted;
    sh_->ResumeAvailability = ASP::ResumeAvailability::OfferEnabled;

    TCPMessage msg;
    msg.header = constructHeader( RD::DEADMANS_HANDLE ).dump( );
    json body = templates_.getRawDeadmansHandleTemplate( );
    body[ "enable_vehicle_motion" ] = false;
    body[ "dmh_gesture_progress" ] = 20.02;
    body[ "dmh_horizontal_touch" ] = 30.03;
    body[ "dmh_vertical_touch" ] = 40.04;
    body[ "crc_value" ] = 50.05;
    msg.body = body.dump( );
    client_->send( msg );

    std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
    EXPECT_EQ( sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::NoScrnInput );
    EXPECT_EQ( sh_->ManeuverButtonPress, TCM::ManeuverButtonPress::ResumeSelected );

    asp_->sync( );
    EXPECT_EQ( asp_->AppAccelerationZ, 20 );
    EXPECT_EQ( asp_->AppSliderPosX, 30 );
    EXPECT_EQ( asp_->AppSliderPosY, 40 );
    EXPECT_EQ( asp_->AppCalcCheck, 50 );
    EXPECT_EQ( asp_->ManeuverEnableInput, TCM::ManeuverEnableInput::NoScrnInput );
    EXPECT_EQ( asp_->ManeuverButtonPress, TCM::ManeuverButtonPress::ResumeSelected );
}


TEST_F( MobileCommsTest, ManeuverComplete )
{
    std::string testManeuver = "StrFwd";
    sendCorrectMobileInit( );
    sendManeuverInitPushPull( testManeuver );
    completeManeuver( testManeuver );
}


TEST_F( MobileCommsTest, ExploratoryLimitReached )
{
    std::string testManeuver = "StrFwd";
    sendCorrectMobileInit( );

    // send first consecutive explore request, expect continue_exploring == true
    sendManeuverInitPushPull( testManeuver );
    completeManeuver( testManeuver );

    TCPMessage msg;
    msg.header = constructHeader( RD::LIST_MANEUVERS ).dump( );
    client_->send( msg );
    struct TCPMessage reply = client_->receive( );
    json reply_header = json::parse( reply.header );
    json reply_body = json::parse( reply.body );

    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::AVAILABLE_MANEUVERS );
    EXPECT_EQ( reply_body["default"], testManeuver );
    EXPECT_TRUE( reply_body["continue_exploring"] );

    // send second consectutive explore request, expect continue_exploring == true
    sendManeuverInitPushPull( testManeuver );
    completeManeuver( testManeuver );

    client_->send( msg );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    reply_body = json::parse( reply.body );

    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::AVAILABLE_MANEUVERS );
    EXPECT_EQ( reply_body["default"], testManeuver );
    EXPECT_TRUE( reply_body["continue_exploring"] );

    // send third consectutive explore request, expect continue_exploring == false
    sendManeuverInitPushPull( testManeuver );
    asp_->ExploreModeAvailability = ASP::ExploreModeAvailability::OfferDisabled;
    completeManeuver( testManeuver );

    client_->send( msg );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    reply_body = json::parse( reply.body );

    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::AVAILABLE_MANEUVERS );
    EXPECT_EQ( reply_body["default"], testManeuver );
    EXPECT_FALSE( reply_body["continue_exploring"] );

    // send new maneuver, expect continue_exploring == true
    testManeuver = "StrRvs";
    sendManeuverInitPushPull( testManeuver );
    asp_->ExploreModeAvailability = ASP::ExploreModeAvailability::OfferEnabled;
    completeManeuver( testManeuver );

    client_->send( msg );
    reply = client_->receive( );
    reply_header = json::parse( reply.header );
    reply_body = json::parse( reply.body );

    EXPECT_EQ( reply_header[ "group" ], (std::string)RD::AVAILABLE_MANEUVERS );
    EXPECT_EQ( reply_body["default"], testManeuver );
    EXPECT_TRUE( reply_body["continue_exploring"] );
}

TEST_F( MobileCommsTest, CancelDriveOn )
{
    TCPMessage msg;
    msg.header = constructHeader( RD::CANCEL_DRIVE_ON ).dump( );
    client_->send( msg );

    asp_->sync( );
    EXPECT_EQ( asp_->ManeuverButtonPress, TCM::ManeuverButtonPress::ResumeSelected );
}

TEST_F(MobileCommsTest, CancelManeuver) {
    sendCorrectMobileInit();
    asp_->ActiveParkingType = ASP::ActiveParkingType::PushPull;
    asp_->ActiveManeuverOrientation = ASP::ActiveManeuverOrientation::PerpendicularFront;
    asp_->ManeuverStatus = ASP::ManeuverStatus::Confirming;
    asp_->ManeuverProgressBar = 0;
    asp_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    asp_->sync();

    // state change - ManeuverStatus -> Confirming
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["status"], 903);

    // maneuver_init -> maneuver_status
    TCPMessage msg;
    msg.header = constructHeader( RD::MANEUVER_INIT ).dump();
    json body = templates_.getRawManeuverInitTemplate();
    body["maneuver"] = "StrFwd";
    msg.body = body.dump();
    client_->send(msg);
    reply = client_->receive();
    reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MANEUVER_STATUS);
    reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["maneuver"], "StrFwd");
    EXPECT_EQ(reply_body["progress"], 0);
    EXPECT_EQ(reply_body["status"], 903);

    // cancel_maneuver -> maneuver_status + vehicle_status
    TCPMessage msg2;
    msg2.header = constructHeader(RD::CANCEL_MANEUVER).dump();
    client_->send(msg2);
    asp_->ManeuverStatus = ASP::ManeuverStatus::Cancelled;
    asp_->CancelMsg = ASP::CancelMsg::ManoeuvreCancelledDriverRequest;
    asp_->ManeuverProgressBar = 40;
    asp_->sync();

    // expecting two statuses - one vehicle_status and one maneuver_status
    // these can arrive in any order
    std::vector<TCPMessage> received_statuses;
    for (int i = 0; i < 2; ++i) {
        received_statuses.push_back(client_->receive());
    }

    json status_header;
    json status_body;
    for (const auto& status : received_statuses) {
        status_header = json::parse(status.header);
        status_body = json::parse(status.body);
        if (status_header["group"] == RD::VEHICLE_STATUS) {
            EXPECT_EQ(status_body[ "status_2xx" ]["status_code"], 206);  // maneuver cancelled at driver's request
        }
        else if (status_header["group"] == RD::MANEUVER_STATUS) {
            EXPECT_EQ(status_body["maneuver"], "StrFwd");
            EXPECT_EQ(status_body["progress"], 40);
            EXPECT_EQ(status_body["status"], 910);
        }
        else {
            FAIL(); // received unexpected message type
        }
    }
}

TEST_F(MobileCommsTest, CabinCommands) {
    // use cabin_commands to set cabin state
    TCPMessage msg;
    msg.header = constructHeader(RD::CABIN_COMMANDS).dump();
    json body = templates_.getRawCabinCommandsTemplate();
    body["engine_off"] = true;
    body["doors_locked"] = true;
    msg.body = body.dump();
    client_->send(msg);
    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::CABIN_STATUS);
    json reply_body = json::parse(reply.body);
    EXPECT_TRUE(reply_body["power_status"]);
    EXPECT_TRUE(reply_body["lock_status"]);

    // use get_cabin_status to check current state matches
    TCPMessage msg2;
    msg2.header = constructHeader(RD::GET_CABIN_STATUS).dump();
    client_->send(msg2);
    struct TCPMessage reply2 = client_->receive();
    json reply2_header = json::parse(reply2.header);
    EXPECT_EQ(reply2_header["group"], (std::string)RD::CABIN_STATUS);
    json reply2_body = json::parse(reply2.body);
    EXPECT_TRUE(reply2_body["power_status"]);
    EXPECT_TRUE(reply2_body["lock_status"]);
}

TEST_F(MobileCommsTest, MobileChallenge) {
    asp_->MobileChallengeSend = 12345678;
    asp_->sync();

    struct TCPMessage reply = client_->receive();
    json reply_header = json::parse(reply.header);
    EXPECT_EQ(reply_header["group"], (std::string)RD::MOBILE_CHALLENGE);
    json reply_body = json::parse(reply.body);
    EXPECT_EQ(reply_body["packed_bytes"], "12345678");
}

TEST_F(MobileCommsTest, MobileResponse) {
    TCPMessage msg;
    msg.header = constructHeader(RD::MOBILE_RESPONSE).dump();
    json body = templates_.getRawMobileResponseTemplate();
    body["response_to_challenge"] = "12345678";
    msg.body = body.dump();
    client_->send(msg);

    asp_->sync();
    EXPECT_EQ(asp_->MobileChallengeReply, 12345678);
}

TEST_F( MobileCommsTest, MobileChallengeAndResponse )
{
    sendMobileChallengeAndResponse( );
}

TEST_F( MobileCommsTest, HappyPath )
{
    std::string testManeuver = "";
    sendCorrectMobileInit( );
    sendGetThreatData( );
    json available_maneuvers = sendListManeuversPushPull( );
    EXPECT_EQ( available_maneuvers["default"], testManeuver );
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );

    testManeuver = "StrRvs";
    sendManeuverInitPushPull( testManeuver );
    sendMobileChallengeAndResponse( );

    TCPMessage msgDMH;
    msgDMH.header = constructHeader( RD::DEADMANS_HANDLE ).dump( );
    json bodyDMH = templates_.getRawDeadmansHandleTemplate( );
    bodyDMH[ "enable_vehicle_motion" ] = true;
    msgDMH.body = bodyDMH.dump( );
    client_->send( msgDMH );
    asp_->ManeuverStatus = ASP::ManeuverStatus::Maneuvering;
    asp_->InfoMsg = ASP::InfoMsg::RemoteManeuverReady;
    asp_->InstructMsg = ASP::InstructMsg::RemoteManouevreInProgress;
    asp_->sync( );

    // state change - ManeuverStatus -> Manoeuvring
    EXPECT_EQ( asp_->AppAccelerationZ, bodyDMH["dmh_gesture_progress"] );
    EXPECT_EQ( asp_->AppSliderPosX, (int)bodyDMH["dmh_horizontal_touch"] );
    EXPECT_EQ( asp_->AppSliderPosY, (int)bodyDMH["dmh_vertical_touch"] );
    EXPECT_EQ( asp_->AppCalcCheck, bodyDMH["crc_value"] );
    EXPECT_EQ( sh_->ManeuverStatus, ASP::ManeuverStatus::Maneuvering );
    EXPECT_EQ( sh_->InstructMsg, ASP::InstructMsg::RemoteManouevreInProgress );
    EXPECT_EQ( sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::ValidScrnInput );

    // expecting one vehicle statuses - these can arrive in any order
    std::vector<TCPMessage> receivedStatuses;
    for( int i = 0; i < 2; ++i )
    {
        receivedStatuses.push_back( client_->receive( ) );
    }
    json replyHeader;
    json replyBody;
    for( const auto& status : receivedStatuses )
    {
        replyHeader = json::parse( status.header );
        replyBody = json::parse( status.body );
        if( replyHeader["group"] == RD::VEHICLE_STATUS )
        {
            EXPECT_EQ( replyBody[ "status_6xx" ]["status_code"], 614 ); // Remote manouevre in progress
            EXPECT_EQ( replyBody[ "status_5xx" ]["status_code"], 519 ); // Remote maneuver ready
        }
        else if( replyHeader["group"] == RD::MANEUVER_STATUS )
        {
            EXPECT_TRUE( replyBody["status"] == 904 ); // state change - ManeuverStatus -> Maneuvering
            EXPECT_EQ( replyBody["maneuver"], testManeuver );
            EXPECT_EQ( replyBody["progress"], 0 );
        }
        else {
            FAIL( ); // received unexpected message type
        }
    }

    // state change - ManeuverStatus -> Interrupted
    bodyDMH[ "enable_vehicle_motion" ] = false;
    msgDMH.body = bodyDMH.dump( );
    client_->send( msgDMH );
    asp_->ManeuverProgressBar = 56;
    asp_->ManeuverStatus = ASP::ManeuverStatus::Interrupted;
    asp_->ResumeAvailability = ASP::ResumeAvailability::OfferEnabled;
    asp_->sync( );
    EXPECT_EQ( sh_->ManeuverProgressBar, 56 );
    EXPECT_EQ( sh_->ManeuverStatus, ASP::ManeuverStatus::Interrupted );
    EXPECT_EQ( sh_->ResumeAvailability, ASP::ResumeAvailability::OfferEnabled );
    EXPECT_EQ( sh_->InstructMsg, ASP::InstructMsg::RemoteManouevreInProgress );
    EXPECT_EQ( sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::NoScrnInput );
    struct TCPMessage reply = client_->receive( );
    replyHeader = json::parse( reply.header );
    EXPECT_EQ( replyHeader[ "group" ], (std::string)RD::MANEUVER_STATUS );
    replyBody = json::parse( reply.body );
    EXPECT_EQ( replyBody["maneuver"], testManeuver );
    EXPECT_EQ( replyBody[ "status" ], 905 );
    EXPECT_EQ( replyBody["progress"], 56 );

    // Upon interrupt, the API should receive get_threat_data and list_maneuvers
    sendGetThreatData( );
    available_maneuvers = sendListManeuversPushPull( );
    EXPECT_EQ( available_maneuvers["default"], testManeuver );
    EXPECT_TRUE( available_maneuvers["continue_exploring"] );

    // state change - ManeuverStatus -> Finishing
    bodyDMH[ "enable_vehicle_motion" ] = true;
    msgDMH.body = bodyDMH.dump( );
    client_->send( msgDMH );
    asp_->ManeuverProgressBar = 100;
    asp_->ManeuverStatus = ASP::ManeuverStatus::Finishing;
    asp_->ConfirmAvailability = ASP::ConfirmAvailability::OfferEnabled;
    asp_->InfoMsg = ASP::InfoMsg::None;
    asp_->CancelMsg = ASP::CancelMsg::None;
    asp_->InstructMsg = ASP::InstructMsg::None;
    asp_->sync( );

    EXPECT_EQ( sh_->ManeuverProgressBar, 100 );
    EXPECT_EQ( sh_->ManeuverStatus, ASP::ManeuverStatus::Finishing );
    EXPECT_EQ( sh_->ConfirmAvailability, ASP::ConfirmAvailability::OfferEnabled );
    EXPECT_EQ( sh_->InfoMsg, ASP::InfoMsg::None );
    EXPECT_EQ( sh_->CancelMsg, ASP::CancelMsg::None );
    EXPECT_EQ( sh_->InstructMsg, ASP::InstructMsg::None );
    EXPECT_EQ( sh_->ManeuverEnableInput, TCM::ManeuverEnableInput::ValidScrnInput );

    // expecting one vehicle status, one maneuver status
    // - these can arrive in any order
    receivedStatuses.clear( );
    for( int i = 0; i < 2; ++i )
    {
        receivedStatuses.push_back( client_->receive( ) );
    }
    for( const auto& status : receivedStatuses )
    {
        replyHeader = json::parse( status.header );
        replyBody = json::parse( status.body );
        if( replyHeader["group"] == RD::VEHICLE_STATUS )
        {
            EXPECT_EQ( replyBody[ "status_6xx" ]["status_code"], 600 ); // None
            EXPECT_EQ( replyBody[ "status_5xx" ]["status_code"], 500 ); // None
        }
        else if( replyHeader["group"] == RD::MANEUVER_STATUS )
        {
            EXPECT_EQ(replyBody["maneuver"], testManeuver );
            EXPECT_EQ(replyBody["progress"], 100 );
            EXPECT_EQ(replyBody["status"], 906); // state change - ManeuverStatus -> Finishing
        }
        else {
            FAIL( ); // received unexpected message type
        }
    }
}
