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
 * \file Class definition for \p SignalHandler.
 *
 * \author fdaniel, trice2
 */

#include "signalhandler.hpp"
#include "picosha2.h"

using namespace std::placeholders;

/*!
 * List of all signals sent in the payload of the TCM_LM PDU
 */
lm_signal_t TCM_lm_t[] = {
    // 1
    {TCM_LM::AppCalcCheck, "AppCalcCheck", 0, 0, 16},
    {TCM_LM::LMDviceAliveCntRMT, "LMDviceAliveCntRMT", 0, 0, 4},
    {TCM_LM::ManeuverEnableInput, "ManeuverEnableInput", 0, 0, 2},
    {TCM_LM::ManeuverGearSelect, "ManeuverGearSelect", 0, 0, 2},
    {TCM_LM::NudgeSelect, "NudgeSelect", 0, 0, 2},
    {TCM_LM::RCDOvrrdReqRMT, "RCDOvrrdReqRMT", 0, 0, 2},
    {TCM_LM::AppSliderPosY, "AppSliderPosY", 0, 0, 12},
    {TCM_LM::AppSliderPosX, "AppSliderPosX", 0, 0, 11},
    {TCM_LM::RCDSpeedChngReqRMT, "RCDSpeedChngReqRMT", 0, 0, 6},
    {TCM_LM::RCDSteWhlChngReqRMT, "RCDSteWhlChngReqRMT", 0, 0, 10},
    // 11
    {TCM_LM::ConnectionApproval, "ConnectionApproval", 0, 0, 2},
    {TCM_LM::ManeuverButtonPress, "ManeuverButtonPress", 0, 0, 3},
    {TCM_LM::DeviceControlMode, "DeviceControlMode", 0, 0, 4},
    {TCM_LM::ManeuverTypeSelect, "ManeuverTypeSelect", 0, 0, 2},
    {TCM_LM::ManeuverDirectionSelect, "ManeuverDirectionSelect", 0, 0, 2},
    {TCM_LM::ExploreModeSelect, "ExploreModeSelect", 0, 0, 1},
    {TCM_LM::RemoteDeviceBatteryLevel, "RemoteDeviceBatteryLevel", 0, 0, 7},
    {TCM_LM::PairedWKeyId, "PairedWKeyId", 0, 0, 8},
    {TCM_LM::ManeuverSideSelect, "ManeuverSideSelect", 0, 0, 2},
    {TCM_LM::LMDviceRngeDistRMT, "LMDviceRngeDistRMT", 0, 0, 10},
    // 21
    {TCM_LM::TTTTTTTTTT, "TTTTTTTTTT", 0, 0, 4},
    {TCM_LM::LMRemoteChallengeVDC, "LMRemoteChallengeVDC", 0, 0, 64},
    {TCM_LM::MobileChallengeReply, "MobileChallengeReply", 0, 0, 64},
    {TCM_LM::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_LM_Session PDU
 */
lm_signal_t TCM_lm_session_t[] = {
    // 1
    {TCM_LM_Session::LMEncrptSessionCntVDC_1, "LMEncrptSessionCntVDC_1", 0, 0, 64},
    {TCM_LM_Session::LMEncrptSessionCntVDC_2, "LMEncrptSessionCntVDC_2", 0, 0, 64},
    {TCM_LM_Session::LMEncryptSessionIDVDC_1, "LMEncryptSessionIDVDC_1", 0, 0, 64},
    {TCM_LM_Session::LMEncryptSessionIDVDC_2, "LMEncryptSessionIDVDC_2", 0, 0, 64},
    {TCM_LM_Session::LMTruncMACVDC, "LMTruncMACVDC", 0, 0, 64},
    {TCM_LM_Session::LMTruncSessionCntVDC, "LMTruncSessionCntVDC", 0, 0, 8},
    {TCM_LM_Session::LMSessionControlVDC, "LMSessionControlVDC", 0, 0, 3},
    {TCM_LM_Session::LMSessionControlVDCExt, "LMSessionControlVDCExt", 0, 0, 5},
    {TCM_LM_Session::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_RemoteControl PDU
 */
lm_signal_t TCM_remotecontrol_t[] = {
    {TCM_RemoteControl::TCMRemoteControl, "TCMRemoteControl", 0, 0, 64},
    {TCM_RemoteControl::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_TransportKey PDU
 */
lm_signal_t TCM_transportkey_t[] = {
    {TCM_TransportKey::LMSessionKeyIDVDC, "LMSessionKeyIDVDC", 0, 0, 16},
    {TCM_TransportKey::LMHashEnTrnsportKeyVDC, "LMHashEnTrnsportKeyVDC", 0, 0, 16},
    {TCM_TransportKey::LMEncTransportKeyVDC_1, "LMEncTransportKeyVDC_1", 0, 0, 64},
    {TCM_TransportKey::LMEncTransportKeyVDC_2, "LMEncTransportKeyVDC_2", 0, 0, 64},
    {TCM_TransportKey::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_LM_App PDU
 */
lm_signal_t TCM_lm_app_t[] = {
    {TCM_LM_App::LMAppTimeStampRMT, "LMAppTimeStampRMT", 0, 0, 64},
    {TCM_LM_App::AppAccelerationX, "AppAccelerationX", 0, 0, 64},
    {TCM_LM_App::AppAccelerationY, "AppAccelerationY", 0, 0, 64},
    {TCM_LM_App::AppAccelerationZ, "AppAccelerationZ", 0, 0, 64},
    {TCM_LM_App::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_LM_KeyID PDU
 */
lm_signal_t TCM_lm_keyid_t[] = {
    {TCM_LM_KeyID::NOT_USED_ONE_BIT, "NOT_USED_ONE_BIT", 0, 0, 1},
    {TCM_LM_KeyID::LMRotKeyChkACKVDC, "LMRotKeyChkACKVDC", 0, 0, 3},
    {TCM_LM_KeyID::LMSessionKeyIDVDCExt, "LMSessionKeyIDVDCExt", 0, 0, 4},
    {TCM_LM_KeyID::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_LM_KeyAlpha PDU
 */
lm_signal_t TCM_lm_keyalpha_t[] = {
    {TCM_LM_KeyAlpha::LMHashEnRotKeyAlphaVDC, "LMHashEnRotKeyAlphaVDC", 0, 0, 16},
    {TCM_LM_KeyAlpha::LMEncRotKeyAlphaVDC_1, "LMEncRotKeyAlphaVDC_1", 0, 0, 64},
    {TCM_LM_KeyAlpha::LMEncRotKeyAlphaVDC_2, "LMEncRotKeyAlphaVDC_2", 0, 0, 64},
    {TCM_LM_KeyAlpha::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_LM_KeyBeta PDU
 */
lm_signal_t TCM_lm_keybeta_t[] = {
    {TCM_LM_KeyBeta::LMHashEncRotKeyBetaVDC, "LMHashEncRotKeyBetaVDC", 0, 0, 16},
    {TCM_LM_KeyBeta::LMEncRotKeyBetaVDC_1, "LMEncRotKeyBetaVDC_1", 0, 0, 64},
    {TCM_LM_KeyBeta::LMEncRotKeyBetaVDC_2, "LMEncRotKeyBetaVDC_2", 0, 0, 64},
    {TCM_LM_KeyBeta::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the TCM_LM_KeyGamma PDU
 */
lm_signal_t TCM_lm_keygamma_t[] = {
    {TCM_LM_KeyGamma::LMHashEncRotKeyGamaVDC, "LMHashEncRotKeyGamaVDC", 0, 0, 16},
    {TCM_LM_KeyGamma::LMEncRotKeyGammaVDC_1, "LMEncRotKeyGammaVDC_1", 0, 0, 64},
    {TCM_LM_KeyGamma::LMEncRotKeyGammaVDC_2, "LMEncRotKeyGammaVDC_2", 0, 0, 64},
    {TCM_LM_KeyGamma::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the ASPM_LM PDU
 */
lm_signal_t ASPM_lm_t[] = {
    // 1
    {ASPM_LM::LMAppConsChkASPM, "LMAppConsChkASPM", 0, 0, 16},
    {ASPM_LM::ActiveAutonomousFeature, "ActiveAutonomousFeature", 0, 0, 4},
    {ASPM_LM::CancelAvailability, "CancelAvailability", 0, 0, 2},
    {ASPM_LM::ConfirmAvailability, "ConfirmAvailability", 0, 0, 2},
    {ASPM_LM::LongitudinalAdjustAvailability, "LongitudinalAdjustAvailability", 0, 0, 2},
    {ASPM_LM::ManeuverDirectionAvailability, "ManeuverDirectionAvailability", 0, 0, 2},
    {ASPM_LM::ManeuverSideAvailability, "ManeuverSideAvailability", 0, 0, 2},
    {ASPM_LM::ActiveManeuverOrientation, "ActiveManeuverOrientation", 0, 0, 2},
    {ASPM_LM::ActiveParkingMode, "ActiveParkingMode", 0, 0, 2},
    {ASPM_LM::DirectionChangeAvailability, "DirectionChangeAvailability", 0, 0, 2},
    // 11
    {ASPM_LM::ParkTypeChangeAvailability, "ParkTypeChangeAvailability", 0, 0, 2},
    {ASPM_LM::ExploreModeAvailability, "ExploreModeAvailability", 0, 0, 2},
    {ASPM_LM::ActiveManeuverSide, "ActiveManeuverSide", 0, 0, 2},
    {ASPM_LM::ManeuverStatus, "ManeuverStatus", 0, 0, 4},
    {ASPM_LM::RemoteDriveOverrideState, "RemoteDriveOverrideState", 0, 0, 2},
    {ASPM_LM::ActiveParkingType, "ActiveParkingType", 0, 0, 3},
    {ASPM_LM::ResumeAvailability, "ResumeAvailability", 0, 0, 2},
    {ASPM_LM::ReturnToStartAvailability, "ReturnToStartAvailability", 0, 0, 2},
    {ASPM_LM::NNNNNNNNNN, "NNNNNNNNNN", 0, 0, 2},
    {ASPM_LM::KeyFobRange, "KeyFobRange", 0, 0, 3},
    // 21
    {ASPM_LM::LMDviceAliveCntAckRMT, "LMDviceAliveCntAckRMT", 0, 0, 4},
    {ASPM_LM::NoFeatureAvailableMsg, "NoFeatureAvailableMsg", 0, 0, 4},
    {ASPM_LM::LMFrwdCollSnsType1RMT, "LMFrwdCollSnsType1RMT", 0, 0, 1},
    {ASPM_LM::LMFrwdCollSnsType2RMT, "LMFrwdCollSnsType2RMT", 0, 0, 1},
    {ASPM_LM::LMFrwdCollSnsType3RMT, "LMFrwdCollSnsType3RMT", 0, 0, 1},
    {ASPM_LM::LMFrwdCollSnsType4RMT, "LMFrwdCollSnsType4RMT", 0, 0, 1},
    {ASPM_LM::LMFrwdCollSnsZone1RMT, "LMFrwdCollSnsZone1RMT", 0, 0, 4},
    {ASPM_LM::LMFrwdCollSnsZone2RMT, "LMFrwdCollSnsZone2RMT", 0, 0, 4},
    {ASPM_LM::LMFrwdCollSnsZone3RMT, "LMFrwdCollSnsZone3RMT", 0, 0, 4},
    {ASPM_LM::LMFrwdCollSnsZone4RMT, "LMFrwdCollSnsZone4RMT", 0, 0, 4},
    // 31
    {ASPM_LM::InfoMsg, "InfoMsg", 0, 0, 6},
    {ASPM_LM::InstructMsg, "InstructMsg", 0, 0, 5},
    {ASPM_LM::LateralControlInfo, "LateralControlInfo", 0, 0, 3},
    {ASPM_LM::LongitudinalAdjustLength, "LongitudinalAdjustLength", 0, 0, 10},
    {ASPM_LM::LongitudinalControlInfo, "LongitudinalControlInfo", 0, 0, 3},
    {ASPM_LM::ManeuverAlignmentAvailability, "ManeuverAlignmentAvailability", 0, 0, 3},
    {ASPM_LM::RemoteDriveAvailability, "RemoteDriveAvailability", 0, 0, 2},
    {ASPM_LM::PauseMsg2, "PauseMsg2", 0, 0, 4},
    {ASPM_LM::PauseMsg1, "PauseMsg1", 0, 0, 4},
    {ASPM_LM::LMRearCollSnsType1RMT, "LMRearCollSnsType1RMT", 0, 0, 1},
    // 41
    {ASPM_LM::LMRearCollSnsType2RMT, "LMRearCollSnsType2RMT", 0, 0, 1},
    {ASPM_LM::LMRearCollSnsType3RMT, "LMRearCollSnsType3RMT", 0, 0, 1},
    {ASPM_LM::LMRearCollSnsType4RMT, "LMRearCollSnsType4RMT", 0, 0, 1},
    {ASPM_LM::LMRearCollSnsZone1RMT, "LMRearCollSnsZone1RMT", 0, 0, 4},
    {ASPM_LM::LMRearCollSnsZone2RMT, "LMRearCollSnsZone2RMT", 0, 0, 4},
    {ASPM_LM::LMRearCollSnsZone3RMT, "LMRearCollSnsZone3RMT", 0, 0, 4},
    {ASPM_LM::LMRearCollSnsZone4RMT, "LMRearCollSnsZone4RMT", 0, 0, 4},
    {ASPM_LM::LMRemoteFeatrReadyRMT, "LMRemoteFeatrReadyRMT", 0, 0, 2},
    {ASPM_LM::CancelMsg, "CancelMsg", 0, 0, 4},
    {ASPM_LM::LMVehMaxRmteVLimRMT, "LMVehMaxRmteVLimRMT", 0, 0, 6},
    // 51
    {ASPM_LM::ManueverPopupDisplay, "ManueverPopupDisplay", 0, 0, 1},
    {ASPM_LM::ManeuverProgressBar, "ManeuverProgressBar", 0, 0, 7},
    {ASPM_LM::MobileChallengeSend, "MobileChallengeSend", 0, 0, 64},
    {ASPM_LM::LMRemoteResponseASPM, "LMRemoteResponseASPM", 0, 0, 64},
    {ASPM_LM::MAXSignal, "MAXSignal", 0, 0, 0},
};

/*!
 * List of all signals sent in the payload of the ASPM_LM_ObjSegment PDU
 */
lm_signal_t ASPM_lm_objsegment_t[] = {
    // 1
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType1RMT, "ASPMFrontSegType1RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist1RMT, "ASPMFrontSegDist1RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType2RMT, "ASPMFrontSegType2RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist2RMT, "ASPMFrontSegDist2RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType3RMT, "ASPMFrontSegType3RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist3RMT, "ASPMFrontSegDist3RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType4RMT, "ASPMFrontSegType4RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist4RMT, "ASPMFrontSegDist4RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType5RMT, "ASPMFrontSegType5RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist5RMT, "ASPMFrontSegDist5RMT", 0, 0, 5},
    // 11
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType6RMT, "ASPMFrontSegType6RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist6RMT, "ASPMFrontSegDist6RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType7RMT, "ASPMFrontSegType7RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist7RMT, "ASPMFrontSegDist7RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType8RMT, "ASPMFrontSegType8RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist8RMT, "ASPMFrontSegDist8RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType9RMT, "ASPMFrontSegType9RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist9RMT, "ASPMFrontSegDist9RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType10RMT, "ASPMFrontSegType10RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist10RMT, "ASPMFrontSegDist10RMT", 0, 0, 5},
    // 21
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType11RMT, "ASPMFrontSegType11RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist11RMT, "ASPMFrontSegDist11RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType12RMT, "ASPMFrontSegType12RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist12RMT, "ASPMFrontSegDist12RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType13RMT, "ASPMFrontSegType13RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist13RMT, "ASPMFrontSegDist13RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType14RMT, "ASPMFrontSegType14RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist14RMT, "ASPMFrontSegDist14RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType15RMT, "ASPMFrontSegType15RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist15RMT, "ASPMFrontSegDist15RMT", 0, 0, 5},
    // 31
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMFrontSegType16RMT, "ASPMFrontSegType16RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMFrontSegDist16RMT, "ASPMFrontSegDist16RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType1RMT, "ASPMRearSegType1RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist1RMT, "ASPMRearSegDist1RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType2RMT, "ASPMRearSegType2RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist2RMT, "ASPMRearSegDist2RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType3RMT, "ASPMRearSegType3RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist3RMT, "ASPMRearSegDist3RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType4RMT, "ASPMRearSegType4RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist4RMT, "ASPMRearSegDist4RMT", 0, 0, 5},
    // 41
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType5RMT, "ASPMRearSegType5RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist5RMT, "ASPMRearSegDist5RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType6RMT, "ASPMRearSegType6RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist6RMT, "ASPMRearSegDist6RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType7RMT, "ASPMRearSegType7RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist7RMT, "ASPMRearSegDist7RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType8RMT, "ASPMRearSegType8RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist8RMT, "ASPMRearSegDist8RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType9RMT, "ASPMRearSegType9RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist9RMT, "ASPMRearSegDist9RMT", 0, 0, 5},
    // 51
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType10RMT, "ASPMRearSegType10RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist10RMT, "ASPMRearSegDist10RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType11RMT, "ASPMRearSegType11RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist11RMT, "ASPMRearSegDist11RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType12RMT, "ASPMRearSegType12RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist12RMT, "ASPMRearSegDist12RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType13RMT, "ASPMRearSegType13RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist13RMT, "ASPMRearSegDist13RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType14RMT, "ASPMRearSegType14RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist14RMT, "ASPMRearSegDist14RMT", 0, 0, 5},
    // 61
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType15RMT, "ASPMRearSegType15RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist15RMT, "ASPMRearSegDist15RMT", 0, 0, 5},
    {ASPM_LM_ObjSegment::ASPMXXXXX, "ASPMXXXXX", 0, 0, 1},
    {ASPM_LM_ObjSegment::ASPMRearSegType16RMT, "ASPMRearSegType16RMT", 0, 0, 2},
    {ASPM_LM_ObjSegment::ASPMRearSegDist16RMT, "ASPMRearSegDist16RMT", 0, 0, 5},

    {ASPM_LM_ObjSegment::MAXSignal, "MAXSignal", 0, 0, 0}
};

/*!
 * List of all signals sent in the payload of the ASPM_RemoteTarget PDU
 */
lm_signal_t ASPM_remotetarget_t[] = {
    {ASPM_RemoteTarget::TCMRemoteTarget, "TCMRemoteTarget", 0, 0, 64},
    {ASPM_RemoteTarget::MAXSignal, "MAXSignal", 0, 0, 0}
};

/*!
 * List of all signals sent in the payload of the ASPM_LM_Session PDU
 */
lm_signal_t ASPM_lm_session_t[] = {
    {ASPM_LM_Session::LMEncrptSessionCntASPM_1, "LMEncrptSessionCntASPM_1", 0, 0, 64},
    {ASPM_LM_Session::LMEncrptSessionCntASPM_2, "LMEncrptSessionCntASPM_2", 0, 0, 64},
    {ASPM_LM_Session::LMEncryptSessionIDASPM_1, "LMEncryptSessionIDASPM_1", 0, 0, 64},
    {ASPM_LM_Session::LMEncryptSessionIDASPM_2, "LMEncryptSessionIDASPM_2", 0, 0, 64},
    {ASPM_LM_Session::LMTruncMACASPM, "LMTruncMACASPM", 0, 0, 64},
    {ASPM_LM_Session::LMTruncSessionCntASPM, "LMTruncSessionCntASPM", 0, 0, 8},
    {ASPM_LM_Session::LMSessionControlASPM, "LMSessionControlASPM", 0, 0, 3},
    {ASPM_LM_Session::LMSessionControlASPMExt, "LMSessionControlASPMExt", 0, 0, 5},
    {ASPM_LM_Session::MAXSignal, "MAXSignal", 0, 0, 0}
};

/*!
 * List of all signals sent in the payload of the ASPM_LM_Trunc PDU
 */
lm_signal_t ASPM_lm_trunc_t[] = {
    {ASPM_LM_Trunc::LMTruncEnPsPrasRotASPM_1, "LMTruncEnPsPrasRotASPM_1", 0, 0, 64},
    {ASPM_LM_Trunc::LMTruncEnPsPrasRotASPM_2, "LMTruncEnPsPrasRotASPM_2", 0, 0, 64},
    {ASPM_LM_Trunc::MAXSignal, "MAXSignal", 0, 0, 0}
};

// Constructor initializes all member variables.
SignalHandler::SignalHandler( )
        :
        mtx( ),
        hasVehicleMoved( false ),
        ConnectionApproval( TCM::ConnectionApproval::NoDevice ),
        DeviceControlMode( TCM::DeviceControlMode::NoMode ),
        ManeuverButtonPress( TCM::ManeuverButtonPress::None ),
        ManeuverTypeSelect( TCM::ManeuverTypeSelect::None ),
        ManeuverDirectionSelect( TCM::ManeuverDirectionSelect::None ),
        ManeuverGearSelect( TCM::ManeuverGearSelect::Park ),
        ManeuverSideSelect( TCM::ManeuverSideSelect::None ),
        ManeuverEnableInput( TCM::ManeuverEnableInput::NoScrnInput ),
        ExploreModeSelect( TCM::ExploreModeSelect::NotPressed ),
        NudgeSelect( TCM::NudgeSelect::NudgeNotAvailable),
        AppSliderPosX( 0000 ),
        AppSliderPosY( 0000 ),
        RemoteDeviceBatteryLevel( 77 ),
        AppCalcCheck( 0000 ),
        MobileChallengeReply( 0000 ),
        AppAccelerationX( 0000 ),
        AppAccelerationY( 0000 ),
        AppAccelerationZ( 0000 ),
        InfoMsg( ASP::InfoMsg::None ),
        ActiveAutonomousFeature( ASP::ActiveAutonomousFeature::NoFeatureActive ),
        ActiveParkingType( ASP::ActiveParkingType::None ),
        ActiveParkingMode( ASP::ActiveParkingMode::None ),
        ManeuverStatus( ASP::ManeuverStatus::NotActive ),
        NoFeatureAvailableMsg( ASP::NoFeatureAvailableMsg::None ),
        CancelMsg( ASP::CancelMsg::None ),
        PauseMsg1( ASP::PauseMsg1::None ),
        PauseMsg2( ASP::PauseMsg2::None ),
        InstructMsg( ASP::InstructMsg::None ),
        ExploreModeAvailability( ASP::ExploreModeAvailability::None ),
        DirectionChangeAvailability( ASP::DirectionChangeAvailability::None ),
        RemoteDriveAvailability( ASP::RemoteDriveAvailability::None ),
        ManeuverSideAvailability( ASP::ManeuverSideAvailability::None ),
        ActiveManeuverSide( ASP::ActiveManeuverSide::None ),
        ActiveManeuverOrientation( ASP::ActiveManeuverOrientation::None ),
        ParkTypeChangeAvailability( ASP::ParkTypeChangeAvailability::None ),
        ManeuverDirectionAvailability( ASP::ManeuverDirectionAvailability::None ),
        ManeuverAlignmentAvailability( ASP::ManeuverAlignmentAvailability::None ),
        ConfirmAvailability( ASP::ConfirmAvailability::None ),
        ResumeAvailability( ASP::ResumeAvailability::None ),
        ReturnToStartAvailability( ASP::ReturnToStartAvailability::None ),
        LongitudinalAdjustAvailability( ASP::LongitudinalAdjustAvailability::None ),
        LongitudinalAdjustLength( 0000 ),
        ManeuverProgressBar( 000 ),
        ASPMFrontSegDistxxRMT( 16, 19 ),
        ASPMRearSegDistxxRMT( 16, 19 ),
        MobileChallengeSend( 0000 ),
        ASPMFrontSegTypexxRMT( 16, 0 ),
        ASPMRearSegTypexxRMT( 16, 0 ),
        AcknowledgeRemotePIN( DCM::AcknowledgeRemotePIN::None ),
        ErrorMsg( DCM::ErrorMsg::None ),
        rangingRequestRate_( DCM::FobRangeRequestRate::None ),
        InControlRemotePIN_( DCM::PIN_NOT_SET ),
        pinStoredInVDC_( DCM::PIN_NOT_SET ),
        maneuverButtonPressTime_( (struct timeval){0} ),
        pinEntryLockoutTime_( (struct timeval){0} ),
        pinIncorrectCount_( 0 ),
        pinLockoutLimit_( 0 ),
        socketHandler_( ),
        running_( true ),
        engine_off_( false ),
        doors_locked_( false )
{

    // Prepopulate structs
    threatDistanceData.ASPMFrontSegDist1RMT = 19;
    threatDistanceData.ASPMFrontSegDist2RMT = 19;
    threatDistanceData.ASPMFrontSegDist3RMT = 19;
    threatDistanceData.ASPMFrontSegDist4RMT = 19;
    threatDistanceData.ASPMFrontSegDist5RMT = 19;
    threatDistanceData.ASPMFrontSegDist6RMT = 19;
    threatDistanceData.ASPMFrontSegDist7RMT = 19;
    threatDistanceData.ASPMFrontSegDist8RMT = 19;
    threatDistanceData.ASPMFrontSegDist9RMT = 19;
    threatDistanceData.ASPMFrontSegDist10RMT = 19;
    threatDistanceData.ASPMFrontSegDist11RMT = 19;
    threatDistanceData.ASPMFrontSegDist12RMT = 19;
    threatDistanceData.ASPMFrontSegDist13RMT = 19;
    threatDistanceData.ASPMFrontSegDist14RMT = 19;
    threatDistanceData.ASPMFrontSegDist15RMT = 19;
    threatDistanceData.ASPMFrontSegDist16RMT = 19;
    threatDistanceData.ASPMRearSegDist1RMT = 19;
    threatDistanceData.ASPMRearSegDist2RMT = 19;
    threatDistanceData.ASPMRearSegDist3RMT = 19;
    threatDistanceData.ASPMRearSegDist4RMT = 19;
    threatDistanceData.ASPMRearSegDist5RMT = 19;
    threatDistanceData.ASPMRearSegDist6RMT = 19;
    threatDistanceData.ASPMRearSegDist7RMT = 19;
    threatDistanceData.ASPMRearSegDist8RMT = 19;
    threatDistanceData.ASPMRearSegDist9RMT = 19;
    threatDistanceData.ASPMRearSegDist10RMT = 19;
    threatDistanceData.ASPMRearSegDist11RMT = 19;
    threatDistanceData.ASPMRearSegDist12RMT = 19;
    threatDistanceData.ASPMRearSegDist13RMT = 19;
    threatDistanceData.ASPMRearSegDist14RMT = 19;
    threatDistanceData.ASPMRearSegDist15RMT = 19;
    threatDistanceData.ASPMRearSegDist16RMT = 19;
    threatTypeData.ASPMFrontSegType1RMT = 0;
    threatTypeData.ASPMFrontSegType2RMT = 0;
    threatTypeData.ASPMFrontSegType3RMT = 0;
    threatTypeData.ASPMFrontSegType4RMT = 0;
    threatTypeData.ASPMFrontSegType5RMT = 0;
    threatTypeData.ASPMFrontSegType6RMT = 0;
    threatTypeData.ASPMFrontSegType7RMT = 0;
    threatTypeData.ASPMFrontSegType8RMT = 0;
    threatTypeData.ASPMFrontSegType9RMT = 0;
    threatTypeData.ASPMFrontSegType10RMT = 0;
    threatTypeData.ASPMFrontSegType11RMT = 0;
    threatTypeData.ASPMFrontSegType12RMT = 0;
    threatTypeData.ASPMFrontSegType13RMT = 0;
    threatTypeData.ASPMFrontSegType14RMT = 0;
    threatTypeData.ASPMFrontSegType15RMT = 0;
    threatTypeData.ASPMFrontSegType16RMT = 0;
    threatTypeData.ASPMRearSegType1RMT = 0;
    threatTypeData.ASPMRearSegType2RMT = 0;
    threatTypeData.ASPMRearSegType3RMT = 0;
    threatTypeData.ASPMRearSegType4RMT = 0;
    threatTypeData.ASPMRearSegType5RMT = 0;
    threatTypeData.ASPMRearSegType6RMT = 0;
    threatTypeData.ASPMRearSegType7RMT = 0;
    threatTypeData.ASPMRearSegType8RMT = 0;
    threatTypeData.ASPMRearSegType9RMT = 0;
    threatTypeData.ASPMRearSegType10RMT = 0;
    threatTypeData.ASPMRearSegType11RMT = 0;
    threatTypeData.ASPMRearSegType12RMT = 0;
    threatTypeData.ASPMRearSegType13RMT = 0;
    threatTypeData.ASPMRearSegType14RMT = 0;
    threatTypeData.ASPMRearSegType15RMT = 0;
    threatTypeData.ASPMRearSegType16RMT = 0;

    srand( time( NULL ) );

    int i;
    // from TCM->ASP
    for (i = 0; (TCM_LM::ID) TCM_lm_t[i].index < TCM_LM::MAXSignal ; ++i)
        vt_TCM_lm.push_back(std::make_shared<LMSignalInfo>(TCM_lm_t[i]));
    for (i = 0; (TCM_LM_Session::ID) TCM_lm_session_t[i].index < TCM_LM_Session::MAXSignal ; ++i)
        vt_TCM_lm_session.push_back(std::make_shared<LMSignalInfo>(TCM_lm_session_t[i]));
    for (i = 0; (TCM_RemoteControl::ID) TCM_remotecontrol_t[i].index < TCM_RemoteControl::MAXSignal ; ++i)
        vt_TCM_remotecontrol.push_back(std::make_shared <LMSignalInfo>(TCM_remotecontrol_t[i]));
    for (i = 0; (TCM_TransportKey::ID) TCM_transportkey_t[i].index < TCM_TransportKey::MAXSignal ; ++i)
        vt_TCM_transportkey.push_back(std::make_shared <LMSignalInfo>(TCM_transportkey_t[i]));
    for (i = 0; (TCM_LM_App::ID) TCM_lm_app_t[i].index < TCM_LM_App::MAXSignal ; ++i)
        vt_TCM_lm_app.push_back(std::make_shared <LMSignalInfo>(TCM_lm_app_t[i]));
    for (i = 0; (TCM_LM_KeyID::ID) TCM_lm_keyid_t[i].index < TCM_LM_KeyID::MAXSignal ; ++i)
        vt_TCM_lm_keyid.push_back(std::make_shared< LMSignalInfo>(TCM_lm_keyid_t[i]));
    for (i = 0; (TCM_LM_KeyAlpha::ID) TCM_lm_keyalpha_t[i].index < TCM_LM_KeyAlpha::MAXSignal ; ++i)
        vt_TCM_lm_keyalpha.push_back(std::make_shared< LMSignalInfo>(TCM_lm_keyalpha_t[i]));
    for (i = 0; (TCM_LM_KeyBeta::ID) TCM_lm_keybeta_t[i].index < TCM_LM_KeyBeta::MAXSignal ; ++i)
        vt_TCM_lm_keybeta.push_back(std::make_shared <LMSignalInfo>(TCM_lm_keybeta_t[i]));
    for (i = 0; (TCM_LM_KeyGamma::ID) TCM_lm_keygamma_t[i].index < TCM_LM_KeyGamma::MAXSignal ; ++i)
        vt_TCM_lm_keygamma.push_back(std::make_shared< LMSignalInfo>(TCM_lm_keygamma_t[i]));

    // from ASP->TCM
    for (i = 0; (ASPM_LM::ID) ASPM_lm_t[i].index < ASPM_LM::MAXSignal ; ++i)
        vt_ASPM_lm.push_back(std::make_shared<LMSignalInfo>(ASPM_lm_t[i]));
    for (i = 0; (ASPM_LM_ObjSegment::ID) ASPM_lm_objsegment_t[i].index < ASPM_LM_ObjSegment::MAXSignal ; ++i)
        vt_ASPM_lm_objsegment.push_back(std::make_shared <LMSignalInfo>(ASPM_lm_objsegment_t[i]));
    for (i = 0; (ASPM_RemoteTarget::ID) ASPM_remotetarget_t[i].index < ASPM_RemoteTarget::MAXSignal ; ++i)
        vt_ASPM_remotetarget.push_back(std::make_shared <LMSignalInfo>(ASPM_remotetarget_t[i]));
    for (i = 0; (ASPM_LM_Session::ID) ASPM_lm_session_t[i].index < ASPM_LM_Session::MAXSignal ; ++i)
        vt_ASPM_lm_session.push_back(std::make_shared <LMSignalInfo>(ASPM_lm_session_t[i]));
    for (int i = 0; (ASPM_LM_Trunc::ID) ASPM_lm_trunc_t[i].index < ASPM_LM_Trunc::MAXSignal ; ++i)
        vt_ASPM_lm_trunc.push_back(std::make_shared <LMSignalInfo>(ASPM_lm_trunc_t[i]));
}

void SignalHandler::stop( ) {
    running_ = false;
    socketHandler_.disconnectClient( false );
    socketHandler_.disconnectServer( );
    signalLoopHandler_.join( );
    rangingLoopHandler_.join( );
    buttonPressLoopHandler_.join( );
}

SignalHandler::~SignalHandler( ) {}

std::mutex& SignalHandler::getMutex( )
{
    return mtx;
}


void SignalHandler::initiateEventLoops( )
{

    // Generate a new UDP socket
    socketHandler_.connectServer(
            (int32_t)SOCK_DGRAM,
            (uint64_t)UDP_ADDR,
            (uint16_t)UDP_PORT,
            true );

    // Kick off threads
    signalLoopHandler_ = std::thread( &SignalHandler::updateSignalEventLoop_, this );       // 30ms loop
    rangingLoopHandler_ = std::thread( &SignalHandler::rangingRequestEventLoop_, this );    // dmh-related
    buttonPressLoopHandler_ = std::thread( &SignalHandler::buttonPressEventLoop_, this );   // maneuver button press rmt - 240ms countdown

    // ** FOR LG ** change below to whatever proprietary process needed
    // Set the PIN from value stored in VDC memory after socketHandler connects.
    getPinFromVDC_( );

}


MANOUEVRE SignalHandler::parseManeuverString( const std::string& maneuver )
{
    if( maneuver == "OutLftFwd" || maneuver == "OLF" || maneuver == "POLF" )
    {
        return MANOUEVRE::POLF;
    }
    else if( maneuver == "OutLftRvs" || maneuver == "OLR" || maneuver == "POLR" )
    {
        return MANOUEVRE::POLR;
    }
    else if( maneuver == "OutRgtFwd" || maneuver == "ORF" || maneuver == "PORF" )
    {
        return MANOUEVRE::PORF;
    }
    else if( maneuver == "OutRgtRvs" || maneuver == "ORR" || maneuver == "PORR" )
    {
        return MANOUEVRE::PORR;
    }
    else if( maneuver == "OutLftPrl" || maneuver == "OLP" || maneuver == "POLP" )
    {
        return MANOUEVRE::POLP;
    }
    else if( maneuver == "OutRgtPrl" || maneuver == "ORP" || maneuver == "PORP" )
    {
        return MANOUEVRE::PORP;
    }
    else if( maneuver == "InLftFwd" || maneuver == "ILF" || maneuver == "PILF" )
    {
        return MANOUEVRE::PILF;
    }
    else if( maneuver == "InLftRvs" || maneuver == "ILR" || maneuver == "PILR" )
    {
        return MANOUEVRE::PILR;
    }
    else if( maneuver == "InRgtFwd" || maneuver == "IRF" || maneuver == "PIRF" )
    {
        return MANOUEVRE::PIRF;
    }
    else if( maneuver == "InRgtRvs" || maneuver == "IRR" || maneuver == "PIRR" )
    {
        return MANOUEVRE::PIRR;
    }
    else if( maneuver == "NdgFwd" || maneuver == "NF" )
    {
        return MANOUEVRE::NF;
    }
    else if( maneuver == "NdgRvs" || maneuver == "NR" )
    {
        return MANOUEVRE::NR;
    }
    else if( maneuver == "StrFwd" || maneuver == "SF" )
    {
        return MANOUEVRE::SF;
    }
    else if( maneuver == "StrRvs" || maneuver == "SR" )
    {
        return MANOUEVRE::SR;
    }
    else if( maneuver == "RtnToOgn" || maneuver == "RTO" || maneuver == "RTS" )
    {
        return MANOUEVRE::RTS;
    }
    else
    {
        return MANOUEVRE::NADA;
    }

}


void SignalHandler::setInputManeuverSignals( const MANOUEVRE& maneuver )
{
    // set shared signals
    switch( maneuver )
    {
        case MANOUEVRE::POLF:
        case MANOUEVRE::PORF:
        case MANOUEVRE::POLR:
        case MANOUEVRE::PORR:
        case MANOUEVRE::POLP:
        case MANOUEVRE::PORP:
        case MANOUEVRE::PILF:
        case MANOUEVRE::PIRF:
        case MANOUEVRE::PILR:
        case MANOUEVRE::PIRR:
        {
            ExploreModeSelect = TCM::ExploreModeSelect::NotPressed;
            NudgeSelect = TCM::NudgeSelect::NudgeNotPressed;
            break;
        }
        case MANOUEVRE::NF:
        case MANOUEVRE::NR:
        {
            ExploreModeSelect = TCM::ExploreModeSelect::NotPressed;
            NudgeSelect = TCM::NudgeSelect::NudgePressed;
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::None;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::None;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Center;
            break;
        }
        case MANOUEVRE::SF:
        case MANOUEVRE::SR:
        {
            ExploreModeSelect = TCM::ExploreModeSelect::Pressed;
            NudgeSelect = TCM::NudgeSelect::NudgeNotPressed;
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::None;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::None;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Center;
            break;
        }
        case MANOUEVRE::RTS:
        case MANOUEVRE::NADA:
        default:
        {
            std::cout << "Error setting input maneuver signals." << std::endl;
            return;
        }
    }

    // set more specific signals
    switch( maneuver )
    {
        case MANOUEVRE::POLF:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::NoseFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Left;
            return;
        }
        case MANOUEVRE::PORF:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::NoseFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Right;
            return;
        }
        case MANOUEVRE::POLR:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::RearFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Reverse;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Left;
            return;
        }
        case MANOUEVRE::PORR:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::RearFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Reverse;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Right;
            return;
        }
        case MANOUEVRE::POLP:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Parallel;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::None;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Left;
            return;
        }
        case MANOUEVRE::PORP:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Parallel;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::None;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Right;
            return;
        }
        case MANOUEVRE::PILF:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::NoseFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Left;
            return;
        }
        case MANOUEVRE::PIRF:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::NoseFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Right;
            return;
        }
        case MANOUEVRE::PILR:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::RearFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Reverse;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Left;
            return;
        }
        case MANOUEVRE::PIRR:
        {
            ManeuverTypeSelect = TCM::ManeuverTypeSelect::Perpendicular;
            ManeuverDirectionSelect = TCM::ManeuverDirectionSelect::RearFirst;
            ManeuverGearSelect = TCM::ManeuverGearSelect::Reverse;
            ManeuverSideSelect = TCM::ManeuverSideSelect::Right;
            return;
        }
        case MANOUEVRE::NF:
        case MANOUEVRE::SF:
        {
            ManeuverGearSelect = TCM::ManeuverGearSelect::Forward;
            return;
        }
        case MANOUEVRE::NR:
        case MANOUEVRE::SR:
        {
            ManeuverGearSelect = TCM::ManeuverGearSelect::Reverse;
            return;
        }
        case MANOUEVRE::RTS:
        case MANOUEVRE::NADA:
        default:
        {
            std::cout << "Error setting input maneuver signals." << std::endl;
            return;
        }
    }

}


void SignalHandler::setDeviceControlMode( const TCM::DeviceControlMode& mode )
{

    // **For LG** any time a member variable is updated, the corresponding ASP
    // should likewise be updated.
    DeviceControlMode = mode;

}


void SignalHandler::setManeuverButtonPress( const TCM::ManeuverButtonPress& mode )
{

    // **For LG** any time a member variable is updated, the corresponding ASP
    // should likewise be updated.
    ManeuverButtonPress = mode;

    // Threading: after 240ms, this value should reset to 'None'
    gettimeofday( &maneuverButtonPressTime_, NULL );

}


void SignalHandler::setManeuverEnableInput(
        const uint16_t& xCoord,
        const uint16_t& yCoord,
        const int64_t& gesturePercent,
        const bool& validGesture )
{

    // **For LG** any time a member variable is updated, the corresponding ASP
    // should likewise be updated.
    AppSliderPosX = xCoord;
    AppSliderPosY = yCoord;
    AppAccelerationZ = gesturePercent;

    if( validGesture == true )
    {
        ManeuverEnableInput = TCM::ManeuverEnableInput::ValidScrnInput;
    }
    else
    {
        ManeuverEnableInput = TCM::ManeuverEnableInput::NoScrnInput;
    }

}


void SignalHandler::setInControlRemotePin( const std::string& pin )
{
    // **For LG** any time a member variable is updated, the corresponding DCM
    // process should likewise be executed and appropriate signals updated.
    if( pinStoredInVDC_ == DCM::PIN_NOT_SET )
    {
        AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::NotSetInDCM;
        std::cout << "No PIN stored in memory; authorisation failed." << std::endl;
        return;
    }
    else if( pin == DCM::PIN_NOT_SET &&
            AcknowledgeRemotePIN == DCM::AcknowledgeRemotePIN::CorrectPIN )
    {
        AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::ExpiredPIN;
        std::cout << "PIN reset upon disconnection of mobile device." << std::endl;
        return;
    }

    InControlRemotePIN_ = pin;

    // **For LG** the below code should be replaced with whatever methods are
    // used to authenticate PIN IAW [REQ NAME HERE].
    if( !isTimevalZero( pinEntryLockoutTime_ ) )
    {
        DCM::AcknowledgeRemotePIN currentPIN = AcknowledgeRemotePIN;
        if( currentPIN == DCM::AcknowledgeRemotePIN::IncorrectPIN3xLockIndefinite )
        {
            // game over, my man!!
            std::cout << "PIN entries locked out indefinitely." << std::endl;
        }
        else if( !checkTimeout( pinEntryLockoutTime_, pinLockoutLimit_ ) )
        {
            timeval currentTime;
            gettimeofday( &currentTime, NULL );
            unsigned int dTime( pinLockoutLimit_ );
            dTime -= diffTimeval( currentTime, pinEntryLockoutTime_ );
            dTime /= 1000000;
            std::cout << "PIN entries locked out for " << dTime << " sec. " << std::endl;
            return;
        }
        else
        {
            pinEntryLockoutTime_ = (struct timeval){0};
        }
    }

    // if no lockouts remain, then check the PIN against pinStoredInVDC_
    if( InControlRemotePIN_ != pinStoredInVDC_ )
    {
        switch( AcknowledgeRemotePIN )
        {
            case( DCM::AcknowledgeRemotePIN::IncorrectPIN3xLockIndefinite ):
            case( DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock3600s ):
            {
                gettimeofday( &pinEntryLockoutTime_, NULL );
                pinLockoutLimit_ = 4294967295;              // max uint value
                AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLockIndefinite;
                break;
            }
            case( DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock300s ):
            {
                gettimeofday( &pinEntryLockoutTime_, NULL );
                pinLockoutLimit_ = 3600000000;              // 3600sec
                AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock3600s;
                break;
            }
            case( DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock60s ):
            {
                gettimeofday( &pinEntryLockoutTime_, NULL );
                pinLockoutLimit_ = 300000000;               // 300sec
                AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock300s;
                break;
            }
            case( DCM::AcknowledgeRemotePIN::IncorrectPIN ):
            {
                if( ++pinIncorrectCount_ >= 3 )
                {
                    gettimeofday( &pinEntryLockoutTime_, NULL );
                    pinLockoutLimit_ = 60000000;            // 60sec
                    AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN3xLock60s;
                    pinIncorrectCount_ = 2;
                    break;
                }
            }
            case( DCM::AcknowledgeRemotePIN::CorrectPIN ):
            case( DCM::AcknowledgeRemotePIN::InvalidSig ):
            default:
            {
                AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::IncorrectPIN;
                ++pinIncorrectCount_;
            }
        }
    }
    else
    {
        AcknowledgeRemotePIN = DCM::AcknowledgeRemotePIN::CorrectPIN;
        pinIncorrectCount_ = 0;
    }

    // Leave below for LG debugging
    #if 0
    /**LGE Code**
    SRD___TELEM_RCD9722___v1: Once the Telematics_VM:
    - Received a confirmation that the disclaimer is agreed (DisclaimerRejected_RD=0), and
    - The customer input the correct InControl Remote PIN (AcknowledgeRemotePIN_DCM=1),
    - Received a ranging request from the mobile phone (RequestWKeyDist_RD),
    The Telematics_VM shall:
    - instruct the RFA to start ranging:
    - RequestWKeyDist=0x01
    - provide RFA with the information "which key to range against": PairedWKeyId
    ''NOTE: the mobile phone will request the Telematics to proceed to ranging until the driver is close enough, then the Telematics VM will start the engine." */
    LOGD("CAN TX to RFA (RequestWKeyDist=0x01, PairedWKeyID=%d)=====================================>", m_RCDLog->getPairedWKeyId());
    m_RCDUtils->setCanValue(VS_CAN_REQUESTWKEYDIST_IDX, 0x01); //if(m_RCDSignal->getSigValue(WkeyVehDist) == -1)
    m_RCDUtils->setCanValue(VS_CAN_PAIREDWKEYID_IDX , m_RCDLog->getPairedWKeyId());

    LOGD("PIN Authenticated. Passing RCMainMenu to ASP.");
    #endif

    return;
}


void SignalHandler::setConnectionApproval( const TCM::ConnectionApproval& mode )
{
    // **For LG** Advanced logic for checking device compatibility should be
    // added here.  For FDJ demo, API assumes that WiFi connection indicates a
    // compatible device, which does not hold true for production app.
    ConnectionApproval = mode;
    // std::cout << "ConnectionApproval: " << (int)ConnectionApproval << std::endl;
}


void SignalHandler::setFobRangeRequestRate( const DCM::FobRangeRequestRate& rate )
{
    // Set private member variable
    rangingRequestRate_ = rate;
}

void SignalHandler::setCabinCommands( bool engine_off, bool doors_locked )
{
    engine_off_ = engine_off;
    doors_locked_ = doors_locked;
}

bool SignalHandler::getEngineOff( ) const
{
    return engine_off_;
}

bool SignalHandler::getDoorsLocked( ) const
{
    return doors_locked_;
}

std::string SignalHandler::getManeuverFromASP( )
{
    if( ActiveParkingType == ASP::ActiveParkingType::PushPull )
    {

        // Check parallel, perpendicular rear or front
        switch( ActiveManeuverOrientation )
        {

            // int value of 0 --> ** UNKNOWN **
            case ASP::ActiveManeuverOrientation::None:           break;

            // int value of 1 --> PushPull Parallel does not make sense
            case ASP::ActiveManeuverOrientation::Parallel:       break;

            // int value of 2 --> PerpendicularRear
            case ASP::ActiveManeuverOrientation::PerpendicularRear:
            {
                return "StrRvs";
            }

            // int value of 3 --> PerpendicularFront
            case ASP::ActiveManeuverOrientation::PerpendicularFront:
            {
                return "StrFwd";
            }

            // scaffolding to parse improperly initialized sig if needed
            case ASP::ActiveManeuverOrientation::InvalidSig:     break;
            default:                                    break;
        }
    }
    else if( ActiveParkingType == ASP::ActiveParkingType::Remote )
    {
        switch( ActiveParkingMode )
        {
            case ASP::ActiveParkingMode::ParkIn:
            {
                switch ( ActiveManeuverOrientation )
                {
                    case ASP::ActiveManeuverOrientation::PerpendicularFront:
                    {
                        switch ( ActiveManeuverSide )
                        {
                            case ASP::ActiveManeuverSide::Left:
                            {
                                return "InLftFwd";
                            }
                            case ASP::ActiveManeuverSide::Right:
                            {
                                return "InRgtFwd";
                            }
                            default: break;
                        }
                        break;
                    }
                    case ASP::ActiveManeuverOrientation::PerpendicularRear:
                    {
                        switch ( ActiveManeuverSide )
                        {
                            case ASP::ActiveManeuverSide::Left:
                            {
                                return "InLftRvs";
                            }
                            case ASP::ActiveManeuverSide::Right:
                            {
                                return "InRgtRvs";
                            }
                            default: break;
                        }
                        break;
                    }
                    default: break;
                }
                break;
            }
            case ASP::ActiveParkingMode::ParkOut:
            {
                switch ( ActiveManeuverOrientation )
                {
                    case ASP::ActiveManeuverOrientation::Parallel:
                    {
                        switch ( ActiveManeuverSide )
                        {
                            case ASP::ActiveManeuverSide::Left:
                            {
                                return "OutLftPrl";
                            }
                            case ASP::ActiveManeuverSide::Right:
                            {
                                return "OutRgtPrl";
                            }
                            default: break;
                        }
                        break;
                    }
                    case ASP::ActiveManeuverOrientation::PerpendicularFront:
                    {
                        switch ( ActiveManeuverSide )
                        {
                            case ASP::ActiveManeuverSide::Left:
                            {
                                return "OutLftFwd";
                            }
                            case ASP::ActiveManeuverSide::Right:
                            {
                                return "OutRgtFwd";
                            }
                            default: break;
                        }
                        break;
                    }
                    case ASP::ActiveManeuverOrientation::PerpendicularRear:
                    {
                        switch ( ActiveManeuverSide )
                        {
                            case ASP::ActiveManeuverSide::Left:
                            {
                                return "OutLftRvs";
                            }
                            case ASP::ActiveManeuverSide::Right:
                            {
                                return "OutRgtRvs";
                            }
                            default: break;
                        }
                        break;
                    }
                    default: break;
                }
                break;
            }
            default: break;
        }
    }
    else if (ActiveParkingType == ASP::ActiveParkingType::LongitudinalAssist)
    {
        switch ( ActiveManeuverOrientation )
        {
            case ASP::ActiveManeuverOrientation::PerpendicularFront:
            {
                return "NdgFwd";
            }
            case ASP::ActiveManeuverOrientation::PerpendicularRear:
            {
                return "NdgRvs";
            }
            default: break;
        }
    }
    // TODO: how to determine if current maneuver is "RtnToOgn"?

    return "";
}


uint8_t SignalHandler::getASPMFrontSegDistxxRMT( int& sensorID )
{

    switch( sensorID )
    {

        case 0: return threatDistanceData.ASPMFrontSegDist1RMT;
        case 1: return threatDistanceData.ASPMFrontSegDist2RMT;
        case 2: return threatDistanceData.ASPMFrontSegDist3RMT;
        case 3: return threatDistanceData.ASPMFrontSegDist4RMT;
        case 4: return threatDistanceData.ASPMFrontSegDist5RMT;
        case 5: return threatDistanceData.ASPMFrontSegDist6RMT;
        case 6: return threatDistanceData.ASPMFrontSegDist7RMT;
        case 7: return threatDistanceData.ASPMFrontSegDist8RMT;
        case 8: return threatDistanceData.ASPMFrontSegDist9RMT;
        case 9: return threatDistanceData.ASPMFrontSegDist10RMT;
        case 10: return threatDistanceData.ASPMFrontSegDist11RMT;
        case 11: return threatDistanceData.ASPMFrontSegDist12RMT;
        case 12: return threatDistanceData.ASPMFrontSegDist13RMT;
        case 13: return threatDistanceData.ASPMFrontSegDist14RMT;
        case 14: return threatDistanceData.ASPMFrontSegDist15RMT;
        case 15: return threatDistanceData.ASPMFrontSegDist16RMT;

        // return 19 if bad value
        default: return 19;

    }
}


uint8_t SignalHandler::getASPMRearSegDistxxRMT( int& sensorID )
{

    switch( sensorID )
    {

        case 0: return threatDistanceData.ASPMRearSegDist1RMT;
        case 1: return threatDistanceData.ASPMRearSegDist2RMT;
        case 2: return threatDistanceData.ASPMRearSegDist3RMT;
        case 3: return threatDistanceData.ASPMRearSegDist4RMT;
        case 4: return threatDistanceData.ASPMRearSegDist5RMT;
        case 5: return threatDistanceData.ASPMRearSegDist6RMT;
        case 6: return threatDistanceData.ASPMRearSegDist7RMT;
        case 7: return threatDistanceData.ASPMRearSegDist8RMT;
        case 8: return threatDistanceData.ASPMRearSegDist9RMT;
        case 9: return threatDistanceData.ASPMRearSegDist10RMT;
        case 10: return threatDistanceData.ASPMRearSegDist11RMT;
        case 11: return threatDistanceData.ASPMRearSegDist12RMT;
        case 12: return threatDistanceData.ASPMRearSegDist13RMT;
        case 13: return threatDistanceData.ASPMRearSegDist14RMT;
        case 14: return threatDistanceData.ASPMRearSegDist15RMT;
        case 15: return threatDistanceData.ASPMRearSegDist16RMT;

        // return 19 if bad value
        default: return 19;

    }
}

uint8_t SignalHandler::getASPMFrontSegTypexxRMT( int& sensorID )
{
    switch( sensorID )
    {
        case 0: return threatTypeData.ASPMFrontSegType1RMT;
        case 1: return threatTypeData.ASPMFrontSegType2RMT;
        case 2: return threatTypeData.ASPMFrontSegType3RMT;
        case 3: return threatTypeData.ASPMFrontSegType4RMT;
        case 4: return threatTypeData.ASPMFrontSegType5RMT;
        case 5: return threatTypeData.ASPMFrontSegType6RMT;
        case 6: return threatTypeData.ASPMFrontSegType7RMT;
        case 7: return threatTypeData.ASPMFrontSegType8RMT;
        case 8: return threatTypeData.ASPMFrontSegType9RMT;
        case 9: return threatTypeData.ASPMFrontSegType10RMT;
        case 10: return threatTypeData.ASPMFrontSegType11RMT;
        case 11: return threatTypeData.ASPMFrontSegType12RMT;
        case 12: return threatTypeData.ASPMFrontSegType13RMT;
        case 13: return threatTypeData.ASPMFrontSegType14RMT;
        case 14: return threatTypeData.ASPMFrontSegType15RMT;
        case 15: return threatTypeData.ASPMFrontSegType16RMT;

        // return 19 if bad value
        default: return 19;

    }
}


uint8_t SignalHandler::getASPMRearSegTypexxRMT( int& sensorID )
{
    switch( sensorID )
    {
        case 0: return threatTypeData.ASPMRearSegType1RMT;
        case 1: return threatTypeData.ASPMRearSegType2RMT;
        case 2: return threatTypeData.ASPMRearSegType3RMT;
        case 3: return threatTypeData.ASPMRearSegType4RMT;
        case 4: return threatTypeData.ASPMRearSegType5RMT;
        case 5: return threatTypeData.ASPMRearSegType6RMT;
        case 6: return threatTypeData.ASPMRearSegType7RMT;
        case 7: return threatTypeData.ASPMRearSegType8RMT;
        case 8: return threatTypeData.ASPMRearSegType9RMT;
        case 9: return threatTypeData.ASPMRearSegType10RMT;
        case 10: return threatTypeData.ASPMRearSegType11RMT;
        case 11: return threatTypeData.ASPMRearSegType12RMT;
        case 12: return threatTypeData.ASPMRearSegType13RMT;
        case 13: return threatTypeData.ASPMRearSegType14RMT;
        case 14: return threatTypeData.ASPMRearSegType15RMT;
        case 15: return threatTypeData.ASPMRearSegType16RMT;

        // return 19 if bad value
        default: return 19;

    }
}

bool SignalHandler::checkTimeout( timeval& tv, const unsigned int& maxVal )
{
    timeval curTime;
    gettimeofday( &curTime, NULL );

    if( diffTimeval( curTime, tv ) > maxVal )
    {
        return true;
    }

    return false;
}


unsigned int SignalHandler::diffTimeval( timeval& t1, timeval& t2 )
{
    return ( ( t1.tv_sec - t2.tv_sec ) * 1000000 ) + ( t1.tv_usec - t2.tv_usec );
}


bool SignalHandler::isTimevalZero( timeval& tv )
{
   return ( tv.tv_sec == 0 ) && ( tv.tv_usec == 0 );
}


void SignalHandler::getPinFromVDC_( )
{

    // **For LG** the process for polling and assigning the PIN value stored in
    // vehicle memory should be defined here and replace DCM::POC_PIN
    pinStoredInVDC_ = picosha2::hash256_hex_string( std::string( DCM::POC_PIN ) );

    return;

}


void SignalHandler::rangingRequestEventLoop_( )
{
    while( running_.load( ) )
    {
        while(  ConnectionApproval == TCM::ConnectionApproval::AllowedDevice &&
                (uint32_t)rangingRequestRate_ != 0 && running_.load( ) )
        {
            // update rangingRequestRate_ to DeadmanRate if maneuver is underway
            if(     ManeuverStatus == ASP::ManeuverStatus::Confirming ||
                    ManeuverStatus == ASP::ManeuverStatus::Maneuvering )
            {
                setFobRangeRequestRate( DCM::FobRangeRequestRate::DeadmanRate );
                hasVehicleMoved = true;
            }
            else
            {
                setFobRangeRequestRate( DCM::FobRangeRequestRate::DefaultRate );
            }

            // // Leave below for ranging debugging.
            // float showRate( (unsigned int)rangingRequestRate_ );
            // showRate /= 1000000;
            //
            // time_t curTime( time( NULL ) );
            //
            // std::cout << "Loop ran at: " << ctime( &curTime );
            // std::cout << "Loop periodicity: ";
            // std::cout << std::fixed << std::setprecision( 2 ) << showRate;
            // std::cout << " seconds." << std::endl;

            /*
            *   ** FOR LG ** Insert request to check Key Fob Range here
            */

            usleep( (uint32_t)rangingRequestRate_ );

        }

        // To prevent memory leakage, if the FobRangeRequestRate is currently
        // None (0), put the loop to sleep at the highest frequency.
        usleep( (uint32_t)DCM::FobRangeRequestRate::DeadmanRate );

    }

    return;
}


void SignalHandler::buttonPressEventLoop_( )
{

    while( running_.load( ) )
    {
        usleep( ASP_REFRESH_RATE );

        if( ManeuverButtonPress == TCM::ManeuverButtonPress::None )
        {
            // do nothing, we only need to check when not "None"
            continue;
        }

        if( isTimevalZero( maneuverButtonPressTime_ ) )
        {
            // if not initialized, reset the button press time
            gettimeofday( &maneuverButtonPressTime_, NULL );
            continue;
        }

        if( checkTimeout( maneuverButtonPressTime_, BUTTON_TIMEOUT_RATE ) )
        {
            // if timeout, reset button to none and clear time holder.
            ManeuverButtonPress = TCM::ManeuverButtonPress::None;
            maneuverButtonPressTime_ = (struct timeval){0};
        }

    }

    return;
}


// ** FOR LG **  function renamed to fit threading
// void SignalHandler::udpSocketSendLoop_( ) { }
void SignalHandler::updateSignalEventLoop_( )
{
    uint8_t bufferToASP[ UDP_BUF_MAX ];
    uint8_t bufferToTCM[ UDP_BUF_MAX ];

    // spin in perpetuity
    while( running_.load( ) )
    {

        // While an appropriate mobile device is connected, send/receive UDP
        while(  ConnectionApproval != TCM::ConnectionApproval::NoDevice &&
                running_.load( ) )
        {
            bzero( bufferToASP, UDP_BUF_MAX );
            bzero( bufferToTCM, UDP_BUF_MAX );
        {
            // while updating signals, stop actions on concurrent threads.
            std::lock_guard<std::mutex> lock( getMutex( ) );

            // convert from integer to bit
            uint16_t outBufSize = encodeTCMSignalData(bufferToASP);

            int sentLen = (int)socketHandler_.sendUDP(
                    bufferToASP,
                    outBufSize );

            // // leave per commonly-used state debugging statements.
            // std::cout << "---" << std::endl << "Message sent.\t";
            // std::cout << "ConnectionApproval: " << (int)ConnectionApproval;
            // std::cout << "\tDeviceControlMode: " << (int)DeviceControlMode;
            // std::cout << std::endl;

            // Print sent message
            // printf( "** %i-Bytes of UDP data sent **\n", sentLen );
            // for( int k = 0; k < sentLen; ) {
            //     printf( "%02X ", bufferToASP[k] );
            //     if(++k%20==0) printf("\n");
            // }
            // printf( "\n\n" );

            ssize_t bytes_received = socketHandler_.receiveUDP(
                    bufferToTCM,
                    sizeof(bufferToTCM) );

            if (bytes_received == ASPM_TOTAL_PACKET_SIZE) {
                decodeASPMSignalData(bufferToTCM);
            }
            else if (bytes_received == 0) {
                std::cout << "Socket disconnected" << std::endl;
            }
            else {
                std::cout << "ERROR: received malformed UDP packet of length " << bytes_received << std::endl;
            }

            // leave per commonly-used state debugging statements.
            // std::cout << "---" << std::endl << "ManeuverStatus: " << (int)ManeuverStatus;
            // std::cout << "\tManeuverProgressBar: " << (int)ManeuverProgressBar;
            // std::cout << "MobileChallengeSend:\t" << (int)MobileChallengeSend << std::endl;
            // std::cout << std::endl;

        }
            // reset the DMH input so that it can timeout on the ASP side.
            // ManeuverEnableInput = TCM::ManeuverEnableInput::NoScrnInput;
            usleep( ASP_REFRESH_RATE );
        }

        // To prevent memory leakage, go to sleep
        usleep( ASP_REFRESH_RATE );

    }

    return;
}

uint64_t SignalHandler::getTCMSignal( uint8_t header_id, const uint16_t& sigid )
{
    // // leave for LG debugging.
    //uint32_t value = 0;
    //printf("Sig:%d", sigid);
    //RCD_LOGP("Sig:%d", sigid);
    if (header_id == HRD_ID_OF_TCM_LM) //ID:57
    {
        switch(sigid) {
            //1===============================
            case TCM_LM::AppCalcCheck:           return (uint64_t)AppCalcCheck;  //length:16bits
            case TCM_LM::LMDviceAliveCntRMT:        return 0x0;  //length:4bits
            case TCM_LM::ManeuverEnableInput:       return (uint64_t)ManeuverEnableInput; //length:2bits
            case TCM_LM::ManeuverGearSelect:       return (uint64_t)ManeuverGearSelect; //length:2bits
            case TCM_LM::NudgeSelect:            return (uint64_t)NudgeSelect;  //length:2bits
            case TCM_LM::RCDOvrrdReqRMT:            return 0x0;  //length:2bits
            case TCM_LM::AppSliderPosY:            return (uint64_t)AppSliderPosY; //length:12bits
            case TCM_LM::AppSliderPosX:            return (uint64_t)AppSliderPosX; //length:11bits
            case TCM_LM::RCDSpeedChngReqRMT:        return 0x00;   //length:6bits
            case TCM_LM::RCDSteWhlChngReqRMT:       return 0x000;  //length:10bits
            //11===============================
            case TCM_LM::ConnectionApproval:         return (uint64_t)ConnectionApproval;  //length:2bits
            case TCM_LM::ManeuverButtonPress:      return (uint64_t)ManeuverButtonPress;
            case TCM_LM::DeviceControlMode:        return (uint64_t)DeviceControlMode;
            case TCM_LM::ManeuverTypeSelect:     return (uint64_t)ManeuverTypeSelect;
            case TCM_LM::ManeuverDirectionSelect:       return (uint64_t)ManeuverDirectionSelect;
            case TCM_LM::ExploreModeSelect:     return (uint64_t)ExploreModeSelect;
            case TCM_LM::RemoteDeviceBatteryLevel:       return (uint64_t)RemoteDeviceBatteryLevel;   //length:7bits
            case TCM_LM::PairedWKeyId:              return 0x00;  //length:8bits
            case TCM_LM::ManeuverSideSelect:          return (uint64_t)ManeuverSideSelect;
            case TCM_LM::LMDviceRngeDistRMT:        return 0x00;  //length:10bits
            //21 ============================
            case TCM_LM::TTTTTTTTTT:                return 0x0;  //length:4bits, garbage value
            case TCM_LM::LMRemoteChallengeVDC:      return 0x0000000000000000;  //length:64bits
            case TCM_LM::MobileChallengeReply:       return (uint64_t)MobileChallengeReply;  //length:64bits
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_Session) //ID:68
    {
        switch(sigid) {
            case TCM_LM_Session::LMEncrptSessionCntVDC_1:       return 0x0000000000000000;
            case TCM_LM_Session::LMEncrptSessionCntVDC_2:       return 0x0000000000000000;
            case TCM_LM_Session::LMEncryptSessionIDVDC_1:       return 0x0000000000000000;
            case TCM_LM_Session::LMEncryptSessionIDVDC_2:       return 0x0000000000000000;
            case TCM_LM_Session::LMTruncMACVDC:                 return 0x0000000000000000;
            case TCM_LM_Session::LMTruncSessionCntVDC:          return 0x00;
            case TCM_LM_Session::LMSessionControlVDC:           return 0x00;
            case TCM_LM_Session::LMSessionControlVDCExt:        return 0x00;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_RemoteControl) //ID:67
    {
        switch(sigid) {
            case TCM_RemoteControl::TCMRemoteControl:            return 0x0000000000000000;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_TransportKey) //ID:69
    {
        switch(sigid) {
            case TCM_TransportKey::LMSessionKeyIDVDC:            return 0x0000;
            case TCM_TransportKey::LMHashEnTrnsportKeyVDC:       return 0x0000;
            case TCM_TransportKey::LMEncTransportKeyVDC_1:       return 0x0000000000000000;
            case TCM_TransportKey::LMEncTransportKeyVDC_2:       return 0x0000000000000000;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_App) //ID:73
    {
        switch(sigid) {
            case TCM_LM_App::LMAppTimeStampRMT:             return 0x0000000000000000;
            case TCM_LM_App::AppAccelerationX:               return 0x0000000000000000;
            case TCM_LM_App::AppAccelerationY:               return 0x0000000000000000;
            case TCM_LM_App::AppAccelerationZ:               return (uint64_t)AppAccelerationZ;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyID) //ID:75
    {
        switch(sigid) {
            case TCM_LM_KeyID::NOT_USED_ONE_BIT: /*1*/          return 0x0;
            case TCM_LM_KeyID::LMRotKeyChkACKVDC: /*3*/         return 0x0;
            case TCM_LM_KeyID::LMSessionKeyIDVDCExt: /*4*/      return 0x0;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyAlpha) //ID:70
    {
        switch(sigid) {
            case TCM_LM_KeyAlpha::LMHashEnRotKeyAlphaVDC: /*16*/    return 0x0000;
            case TCM_LM_KeyAlpha::LMEncRotKeyAlphaVDC_1: /*64*/     return 0x0000000000000000;
            case TCM_LM_KeyAlpha::LMEncRotKeyAlphaVDC_2: /*64*/     return 0x0000000000000000;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyBeta) //ID:71
    {
        switch(sigid) {
            case TCM_LM_KeyBeta::LMHashEncRotKeyBetaVDC: /*16*/    return 0x0000;
            case TCM_LM_KeyBeta::LMEncRotKeyBetaVDC_1: /*64*/      return 0x0000000000000000;
            case TCM_LM_KeyBeta::LMEncRotKeyBetaVDC_2: /*64*/      return 0x0000000000000000;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else if (header_id == HRD_ID_OF_TCM_LM_KeyGamma) //ID:72
    {
        switch(sigid) {
            case TCM_LM_KeyGamma::LMHashEncRotKeyGamaVDC: /*16*/    return 0x0000;
            case TCM_LM_KeyGamma::LMEncRotKeyGammaVDC_1: /*64*/     return 0x0000000000000000;
            case TCM_LM_KeyGamma::LMEncRotKeyGammaVDC_2: /*64*/     return 0x0000000000000000;
            default:
                printf("receive unknown signal:%d", sigid);
                return 0;
        }
    }
    else {
        printf("receive unknown signal:%d, header_id: %d", sigid, header_id);
    }

    return 0;
}

void SignalHandler::setASPSignal(uint8_t header_id, const int16_t& sigid, uint64_t value )
{
    // // leave for LG debugging.
    // printf("Header: %d, Sig: %d, Val: %lx\n", header_id, sigid, value);

    if (header_id == HRD_ID_OF_ASPM_LM) {
        switch(sigid) {
            //1===============================
            case ASPM_LM::LMAppConsChkASPM:         return;
            case ASPM_LM::ActiveAutonomousFeature:       ActiveAutonomousFeature = (ASP::ActiveAutonomousFeature)value;            return;
            case ASPM_LM::CancelAvailability:          return;
            case ASPM_LM::ConfirmAvailability:         ConfirmAvailability = (ASP::ConfirmAvailability)value;                return;
            case ASPM_LM::LongitudinalAdjustAvailability:    LongitudinalAdjustAvailability = (ASP::LongitudinalAdjustAvailability)value;      return;
            case ASPM_LM::ManeuverDirectionAvailability:      ManeuverDirectionAvailability = (ASP::ManeuverDirectionAvailability)value;          return;
            case ASPM_LM::ManeuverSideAvailability:        ManeuverSideAvailability = (ASP::ManeuverSideAvailability)value;              return;
            case ASPM_LM::ActiveManeuverOrientation:         ActiveManeuverOrientation = (ASP::ActiveManeuverOrientation)value;                return;
            case ASPM_LM::ActiveParkingMode:         ActiveParkingMode = (ASP::ActiveParkingMode)value;                return;
            case ASPM_LM::DirectionChangeAvailability:     DirectionChangeAvailability = (ASP::DirectionChangeAvailability)value;        return;
            //11===============================
            case ASPM_LM::ParkTypeChangeAvailability:    ParkTypeChangeAvailability = (ASP::ParkTypeChangeAvailability)value;      return;
            case ASPM_LM::ExploreModeAvailability:    ExploreModeAvailability = (ASP::ExploreModeAvailability)value;      return;
            case ASPM_LM::ActiveManeuverSide:                ActiveManeuverSide = (ASP::ActiveManeuverSide)value;                              return;
            case ASPM_LM::ManeuverStatus:              ManeuverStatus = (ASP::ManeuverStatus)value;                          return;
            case ASPM_LM::RemoteDriveOverrideState:         return;
            case ASPM_LM::ActiveParkingType:         ActiveParkingType = (ASP::ActiveParkingType)value;                return;
            case ASPM_LM::ResumeAvailability:          ResumeAvailability = (ASP::ResumeAvailability)value;
            case ASPM_LM::ReturnToStartAvailability:     ReturnToStartAvailability = (ASP::ReturnToStartAvailability)value;        return;
            case ASPM_LM::NNNNNNNNNN:               return;
            case ASPM_LM::KeyFobRange:         return;
            case ASPM_LM::LMDviceAliveCntAckRMT:    return;
            //21===============================
            case ASPM_LM::NoFeatureAvailableMsg:     NoFeatureAvailableMsg = (ASP::NoFeatureAvailableMsg)value;        return;
            case ASPM_LM::LMFrwdCollSnsType1RMT:    return;
            case ASPM_LM::LMFrwdCollSnsType2RMT:    return;
            case ASPM_LM::LMFrwdCollSnsType3RMT:    return;
            case ASPM_LM::LMFrwdCollSnsType4RMT:    return;
            case ASPM_LM::LMFrwdCollSnsZone1RMT:    return;
            case ASPM_LM::LMFrwdCollSnsZone2RMT:    return;
            case ASPM_LM::LMFrwdCollSnsZone3RMT:    return;
            case ASPM_LM::LMFrwdCollSnsZone4RMT:    return;
            case ASPM_LM::InfoMsg:            InfoMsg = (ASP::InfoMsg)value;                      return;
            //31===============================
            case ASPM_LM::InstructMsg:        InstructMsg = (ASP::InstructMsg)value;              return;
            case ASPM_LM::LateralControlInfo:        return;
            case ASPM_LM::LongitudinalAdjustLength:    LongitudinalAdjustLength = (ASP::LongitudinalAdjustLength)value;                         return;
            case ASPM_LM::LongitudinalControlInfo:       return;
            case ASPM_LM::ManeuverAlignmentAvailability:       ManeuverAlignmentAvailability = (ASP::ManeuverAlignmentAvailability)value;            return;
            case ASPM_LM::RemoteDriveAvailability:         RemoteDriveAvailability = (ASP::RemoteDriveAvailability)value;                return;
            case ASPM_LM::PauseMsg2:          PauseMsg2 = (ASP::PauseMsg2)value;                  return;
            case ASPM_LM::PauseMsg1:           PauseMsg1 = (ASP::PauseMsg1)value;                    return;
            case ASPM_LM::LMRearCollSnsType1RMT:    return;
            //41===============================
            case ASPM_LM::LMRearCollSnsType2RMT:    return;
            case ASPM_LM::LMRearCollSnsType3RMT:    return;
            case ASPM_LM::LMRearCollSnsType4RMT:    return;
            case ASPM_LM::LMRearCollSnsZone1RMT:    return;
            case ASPM_LM::LMRearCollSnsZone2RMT:    return;
            case ASPM_LM::LMRearCollSnsZone3RMT:    return;
            case ASPM_LM::LMRearCollSnsZone4RMT:    return;
            case ASPM_LM::LMRemoteFeatrReadyRMT:    return;
            case ASPM_LM::CancelMsg:          CancelMsg = (ASP::CancelMsg)value;                  return;
            case ASPM_LM::LMVehMaxRmteVLimRMT:      return;
            case ASPM_LM::ManueverPopupDisplay:           return;
            //51===============================
            case ASPM_LM::ManeuverProgressBar:   ManeuverProgressBar = (ASP::ManeuverProgressBar)value;    return;
            case ASPM_LM::MobileChallengeSend:    MobileChallengeSend = (ASP::MobileChallengeSend)value;      return;
            case ASPM_LM::LMRemoteResponseASPM:     return;
            default:
                printf("receive unknown signal:%d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_RemoteTarget) {
        switch(sigid) {
            case ASPM_RemoteTarget::TCMRemoteTarget:     return;
            default:
                printf("receive unknown signal:%d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_LM_Session) {
        switch(sigid) {
            case ASPM_LM_Session::LMEncrptSessionCntASPM_1:     return;
            case ASPM_LM_Session::LMEncrptSessionCntASPM_2:     return;
            case ASPM_LM_Session::LMEncryptSessionIDASPM_1:     return;
            case ASPM_LM_Session::LMEncryptSessionIDASPM_2:     return;
            case ASPM_LM_Session::LMTruncMACASPM:               return;
            case ASPM_LM_Session::LMTruncSessionCntASPM:        return;
            case ASPM_LM_Session::LMSessionControlASPM:         return;
            case ASPM_LM_Session::LMSessionControlASPMExt:      return;
            default:
                printf("receive unknown signal:%d", sigid);
                return;
        }
    }
    else if (header_id == HRD_ID_OF_ASPM_LM_ObjSegment) {
        switch(sigid) {
            case ASPM_LM_ObjSegment::ASPMXXXXX:                 return;       //not used
            case ASPM_LM_ObjSegment::ASPMFrontSegType1RMT:      threatTypeData.ASPMFrontSegType1RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist1RMT:      threatDistanceData.ASPMFrontSegDist1RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType2RMT:      threatTypeData.ASPMFrontSegType2RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist2RMT:      threatDistanceData.ASPMFrontSegDist2RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType3RMT:      threatTypeData.ASPMFrontSegType3RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist3RMT:      threatDistanceData.ASPMFrontSegDist3RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType4RMT:      threatTypeData.ASPMFrontSegType4RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist4RMT:      threatDistanceData.ASPMFrontSegDist4RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType5RMT:      threatTypeData.ASPMFrontSegType5RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist5RMT:      threatDistanceData.ASPMFrontSegDist5RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType6RMT:      threatTypeData.ASPMFrontSegType6RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist6RMT:      threatDistanceData.ASPMFrontSegDist6RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType7RMT:      threatTypeData.ASPMFrontSegType7RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist7RMT:      threatDistanceData.ASPMFrontSegDist7RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType8RMT:      threatTypeData.ASPMFrontSegType8RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist8RMT:      threatDistanceData.ASPMFrontSegDist8RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType9RMT:      threatTypeData.ASPMFrontSegType9RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist9RMT:      threatDistanceData.ASPMFrontSegDist9RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType10RMT:     threatTypeData.ASPMFrontSegType10RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist10RMT:     threatDistanceData.ASPMFrontSegDist10RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType11RMT:     threatTypeData.ASPMFrontSegType11RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist11RMT:     threatDistanceData.ASPMFrontSegDist11RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType12RMT:     threatTypeData.ASPMFrontSegType12RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist12RMT:     threatDistanceData.ASPMFrontSegDist12RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType13RMT:     threatTypeData.ASPMFrontSegType13RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist13RMT:     threatDistanceData.ASPMFrontSegDist13RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType14RMT:     threatTypeData.ASPMFrontSegType14RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist14RMT:     threatDistanceData.ASPMFrontSegDist14RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType15RMT:     threatTypeData.ASPMFrontSegType15RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist15RMT:     threatDistanceData.ASPMFrontSegDist15RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMFrontSegType16RMT:     threatTypeData.ASPMFrontSegType16RMT = (uint8_t)value;     return;
            case ASPM_LM_ObjSegment::ASPMFrontSegDist16RMT:     threatDistanceData.ASPMFrontSegDist16RMT = (uint8_t)value; return;
            case ASPM_LM_ObjSegment::ASPMRearSegType1RMT:       threatTypeData.ASPMRearSegType1RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist1RMT:       threatDistanceData.ASPMRearSegDist1RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType2RMT:       threatTypeData.ASPMRearSegType2RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist2RMT:       threatDistanceData.ASPMRearSegDist2RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType3RMT:       threatTypeData.ASPMRearSegType3RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist3RMT:       threatDistanceData.ASPMRearSegDist3RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType4RMT:       threatTypeData.ASPMRearSegType4RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist4RMT:       threatDistanceData.ASPMRearSegDist4RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType5RMT:       threatTypeData.ASPMRearSegType5RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist5RMT:       threatDistanceData.ASPMRearSegDist5RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType6RMT:       threatTypeData.ASPMRearSegType6RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist6RMT:       threatDistanceData.ASPMRearSegDist6RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType7RMT:       threatTypeData.ASPMRearSegType7RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist7RMT:       threatDistanceData.ASPMRearSegDist7RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType8RMT:       threatTypeData.ASPMRearSegType8RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist8RMT:       threatDistanceData.ASPMRearSegDist8RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType9RMT:       threatTypeData.ASPMRearSegType9RMT = (uint8_t)value;       return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist9RMT:       threatDistanceData.ASPMRearSegDist9RMT = (uint8_t)value;   return;
            case ASPM_LM_ObjSegment::ASPMRearSegType10RMT:      threatTypeData.ASPMRearSegType10RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist10RMT:      threatDistanceData.ASPMRearSegDist10RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMRearSegType11RMT:      threatTypeData.ASPMRearSegType11RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist11RMT:      threatDistanceData.ASPMRearSegDist11RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMRearSegType12RMT:      threatTypeData.ASPMRearSegType12RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist12RMT:      threatDistanceData.ASPMRearSegDist12RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMRearSegType13RMT:      threatTypeData.ASPMRearSegType13RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist13RMT:      threatDistanceData.ASPMRearSegDist13RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMRearSegType14RMT:      threatTypeData.ASPMRearSegType14RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist14RMT:      threatDistanceData.ASPMRearSegDist14RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMRearSegType15RMT:      threatTypeData.ASPMRearSegType15RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist15RMT:      threatDistanceData.ASPMRearSegDist15RMT = (uint8_t)value;  return;
            case ASPM_LM_ObjSegment::ASPMRearSegType16RMT:      threatTypeData.ASPMRearSegType16RMT = (uint8_t)value;      return;
            case ASPM_LM_ObjSegment::ASPMRearSegDist16RMT:      threatDistanceData.ASPMRearSegDist16RMT = (uint8_t)value;  return;
            default:
                printf("receive unknown signal:%d", sigid);
                return;
        }

    }
    else if (header_id == HRD_ID_OF_ASPM_LM_Trunc) {
        switch(sigid) {
            case ASPM_LM_Trunc::LMTruncEnPsPrasRotASPM_1:      return;
            case ASPM_LM_Trunc::LMTruncEnPsPrasRotASPM_2:      return;
            default:
                printf("receive unknown signal:%d", sigid);
                return;
        }
    }
    else {
        printf("Not valid header_id(%d)", header_id);
    }
}

uint16_t SignalHandler::encodeTCMSignalData(uint8_t* buffer)
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

    int pdu_header[NUMBER_OF_TCM_PDU] = {HRD_ID_OF_TCM_LM, HRD_ID_OF_TCM_LM_Session, HRD_ID_OF_TCM_RemoteControl, HRD_ID_OF_TCM_TransportKey,
        HRD_ID_OF_TCM_LM_App, HRD_ID_OF_TCM_LM_KeyID, HRD_ID_OF_TCM_LM_KeyAlpha, HRD_ID_OF_TCM_LM_KeyBeta,  HRD_ID_OF_TCM_LM_KeyGamma };

    memset(buffer, 0x00, UDP_BUF_MAX);

    for (kk = 0; kk<NUMBER_OF_TCM_PDU ; ++kk) {
        uint16_t index = 0;

        //printf("=> curr_packet addr : %p ", curr_packet);
        std::shared_ptr<SomePacket> packet = std::make_shared<SomePacket>(reinterpret_cast<char*>(curr_packet), UdpPacketType::SendTCMPacket);
        packet->putHeaderID(pdu_header[kk]);

        char* data = (char *)packet->getPayloadStartAddress();
        std::vector<std::shared_ptr<LMSignalInfo>>& vt_signal = get_TCM_vector(pdu_header[kk]);

        for (ii=0; ii < vt_signal.size(); ii++)
        {
            length = vt_signal.at(ii)->getBitLength();
            loop = (length-1)/8 ;

            for (jj = loop; jj >= 0; jj--)
            {
                unsigned long int x = 0xff; // 0xFF was int value in original code, this is not allowed to shift as int's max bit. So 0xFF should be defined as 8bytes(64bits)
                //currValue = ((m_RCDSignal->getSigValue(signalPack[ii])) & (0xFF << (8 * jj))) >> (8 * jj); //ORIGIN
                currValue = ( (getTCMSignal(packet->getHeaderID(), vt_signal.at(ii)->getIndex()) ) & (x << (8 * jj))) >> (8 * jj); //SANGGIL
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
                //LOGI("data[%d]: %02X", index
            }
        }

        uint16_t move_len = sizeof(pdu_header_t) + packet->getPayloadLength();

        size_total += move_len;
        curr_packet += move_len;
    }

    //LOGE("==========================================(size_total: %d)", size_total);
    return size_total;
}

void SignalHandler::decodeASPMSignalData(uint8_t* buffer)
{
    int8_t ii = 0, jj = 0;
    uint8_t length = 0;
    uint8_t bits_read = 0, bits_to_read = 0;
    uint8_t lmask = 0, rmask = 0, value_byte = 0;
    uint8_t bits_remaining = 0;
    uint64_t value = 0;
    uint8_t* curr_packet = buffer;
    uint16_t index = 0;

    for (ii = 0; ii < NUMBER_OF_ASPM_PDU; ++ii) {
        std::shared_ptr<SomePacket> packet = std::make_shared<SomePacket>(reinterpret_cast<char*>(curr_packet), UdpPacketType::ReadASPMPacket);
        char* data = (char *)packet->getPayloadStartAddress();
        index = 0;
        std::vector<std::shared_ptr<LMSignalInfo>>& vt_signal = get_ASP_vector(packet->getHeaderID());

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
            setASPSignal(packet->getHeaderID(), vt_signal[jj]->getIndex(), value);
        }
        curr_packet += sizeof(pdu_header_t) + packet->getPayloadLength();
    }
}

std::vector<std::shared_ptr<LMSignalInfo>>& SignalHandler::get_TCM_vector (int header_id)
{
    switch( header_id )
    {
        case HRD_ID_OF_TCM_LM:
            return this->vt_TCM_lm;
        case HRD_ID_OF_TCM_RemoteControl:
            return this->vt_TCM_remotecontrol;
        case HRD_ID_OF_TCM_LM_Session:
            return this->vt_TCM_lm_session;
        case HRD_ID_OF_TCM_TransportKey:
            return this->vt_TCM_transportkey;
        case HRD_ID_OF_TCM_LM_App:
            return this->vt_TCM_lm_app;
        case HRD_ID_OF_TCM_LM_KeyID:
            return this->vt_TCM_lm_keyid;
        case HRD_ID_OF_TCM_LM_KeyAlpha:
            return this->vt_TCM_lm_keyalpha;
        case HRD_ID_OF_TCM_LM_KeyBeta:
            return this->vt_TCM_lm_keybeta;
        case HRD_ID_OF_TCM_LM_KeyGamma:
            return this->vt_TCM_lm_keygamma;
    }

    return this->vt_TCM_lm;
}
std::vector<std::shared_ptr<LMSignalInfo>>& SignalHandler::get_ASP_vector (int header_id)
{
    switch( header_id )
    {
        case HRD_ID_OF_ASPM_LM:
            return this->vt_ASPM_lm;
        case HRD_ID_OF_ASPM_RemoteTarget:
            return this->vt_ASPM_remotetarget;
        case HRD_ID_OF_ASPM_LM_Session:
            return this->vt_ASPM_lm_session;
        case HRD_ID_OF_ASPM_LM_ObjSegment:
            return this->vt_ASPM_lm_objsegment;
        case HRD_ID_OF_ASPM_LM_Trunc:
            return this->vt_ASPM_lm_trunc;
    }

    return this->vt_ASPM_lm;
}
