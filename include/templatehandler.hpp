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
 * \file Header for \p TemplateHandler class.
 *
 * \author fdaniel, trice2
 */

#pragma once

#include "json.hpp"


/*!
 * Constructs and stores all JSON templates for reference from \p RemoteDeviceHandler.
 */
class TemplateHandler
{

public:

    TemplateHandler( );

    nlohmann::json getRawHeaderTemplate() const { return rawHeaderJSON_; }
    nlohmann::json getRawAvailableManeuversTemplate() const { return rawAvailableManeuversJSON_; }
    nlohmann::json getRawManeuverStatusTemplate() const { return rawManeuverStatusJSON_; }
    nlohmann::json getRawThreatDataTemplate() const { return rawThreatDataJSON_; }
    nlohmann::json getRawVehicleAPIVersionTemplate() const { return rawVehicleAPIVersionJSON_; }
    nlohmann::json getRawVehicleInitTemplate() const { return rawVehicleInitJSON_; }
    nlohmann::json getRawVehicleStatusTemplate() const { return rawVehicleStatusJSON_; }
    nlohmann::json getRawMobileChallengeTemplate() const { return rawMobileChallengeJSON_; }
    nlohmann::json getRawMobileResponseTemplate() const { return rawMobileResponseJSON_; }
    nlohmann::json getRawDeadmansHandleTemplate() const { return rawDeadmansHandleJSON_; }
    nlohmann::json getRawManeuverInitTemplate() const { return rawManeuverInitJSON_; }
    nlohmann::json getRawMobileInitTemplate() const { return rawMobileInitJSON_; }
    nlohmann::json getRawSendPINTemplate() const { return rawSendPINJSON_; }
    nlohmann::json getRawCabinCommandsTemplate() const { return rawCabinCommandsJSON_; }
    nlohmann::json getRawCabinStatusTemplate() const { return rawCabinStatusJSON_; }

private:

    nlohmann::json rawHeaderJSON_;
    // read commands
    nlohmann::json rawAvailableManeuversJSON_;
    nlohmann::json rawManeuverStatusJSON_;
    nlohmann::json rawThreatDataJSON_;
    nlohmann::json rawVehicleAPIVersionJSON_;
    nlohmann::json rawVehicleInitJSON_;
    nlohmann::json rawVehicleStatusJSON_;
    nlohmann::json rawCabinStatusJSON_;
    nlohmann::json rawMobileChallengeJSON_;
    // write commands
    nlohmann::json rawMobileResponseJSON_;
    nlohmann::json rawDeadmansHandleJSON_;
    nlohmann::json rawManeuverInitJSON_;
    nlohmann::json rawMobileInitJSON_;
    nlohmann::json rawSendPINJSON_;
    nlohmann::json rawCabinCommandsJSON_;
};


/*!
 * Stores all message header macros for use in \p RemoteDeviceHandler.
 */
class RD
{

public:
    static constexpr auto GET_API_VERSION = "get_api_version";
    static constexpr auto SEND_PIN = "send_pin";
    static constexpr auto MOBILE_INIT = "mobile_init";
    static constexpr auto GET_THREAT_DATA = "get_threat_data";
    static constexpr auto LIST_MANEUVERS = "list_maneuvers";
    static constexpr auto MANEUVER_INIT = "maneuver_init";
    static constexpr auto DEADMANS_HANDLE = "deadmans_handle";
    static constexpr auto CANCEL_DRIVE_ON = "cancel_drive_on";
    static constexpr auto CANCEL_MANEUVER = "cancel_maneuver";
    static constexpr auto GET_CABIN_STATUS = "get_cabin_status";
    static constexpr auto CABIN_COMMANDS = "cabin_commands";
    static constexpr auto MOBILE_RESPONSE = "mobile_response";
    static constexpr auto VEHICLE_API_VERSION = "vehicle_api_version";
    static constexpr auto VEHICLE_STATUS = "vehicle_status";
    static constexpr auto VEHICLE_INIT = "vehicle_init";
    static constexpr auto THREAT_DATA = "threat_data";
    static constexpr auto AVAILABLE_MANEUVERS = "available_maneuvers";
    static constexpr auto MANEUVER_STATUS = "maneuver_status";
    static constexpr auto MOBILE_CHALLENGE = "mobile_challenge";
    static constexpr auto CABIN_STATUS = "cabin_status";
    static constexpr auto DEBUG = "debug";

};
