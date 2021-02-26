## Special Handling
**May be ignored by general status handlers:**
* The 3xx and 4xx series of Pause codes are only relevant during Selecting and Maneuvering.
* The 7xx series of PIN codes are only relevant during user verification.
* The 9xx series of Maneuver codes are only relevant during a maneuver.


# Status Codes

* NA indicates that the signal should not be sent to the phone

Status Code | Root Signal | Signal Text | App Action
--- | --- | --- | ---
100 | `ASP::NoFeatureAvailableMsg` | None |
101 | `ASP::NoFeatureAvailableMsg` | Not available system fault | show catastrophic error
102 | `ASP::NoFeatureAvailableMsg` | Not available sensor blocked | show catastrophic error
103 | `ASP::NoFeatureAvailableMsg` | Not available vehicle not started | NA
104 | `ASP::NoFeatureAvailableMsg` | Not available tilt too strong | show catastrophic error
105 | `ASP::NoFeatureAvailableMsg` | Not available trailer connected | suggest user action
106 | `ASP::NoFeatureAvailableMsg` | Not available ride height | NA
107 | `ASP::NoFeatureAvailableMsg` | Not available speed too high | suggest user action
108 | `ASP::NoFeatureAvailableMsg` | Not available ATPC | suggest user action (All Train Progress Control)
109 | `ASP::NoFeatureAvailableMsg` | Not available Tow Assist on | NA
110 | `ASP::NoFeatureAvailableMsg` | Not available Wade Assist on | NA
111 | `ASP::NoFeatureAvailableMsg` | Not available ACC+ on | suggest user action (Adaptive Cruise Ctrl, Country Road Assist)
112 | `ASP::NoFeatureAvailableMsg` | Not available TJP on | NA
113 | `ASP::NoFeatureAvailableMsg` | Not available vehicle on motorway | NA
114 | `ASP::NoFeatureAvailableMsg` | Reserved2 | NA
115 | `ASP::NoFeatureAvailableMsg` | Reserved1 | NA
200 | `ASP::CancelMsg` | None | for the series, cancel and return to home screen
201 | `ASP::CancelMsg` | Cancelled from loss of traction |
202 | `ASP::CancelMsg` | Cancelled max number of moves reached |
203 | `ASP::CancelMsg` | Cancelled paused for too long | Inactivty timer expired
204 | `ASP::CancelMsg` | Cancelled internal system failure | NA
205 | `ASP::CancelMsg` | Cancelled trailer connected | NA
206 | `ASP::CancelMsg` | Maneuver cancelled driver request | acknowledgement of requested cancellation
207 | `ASP::CancelMsg` | Maneuver cancelled vehicle driven on | acknowledgement that user is no longer using RPA
208 | `ASP::CancelMsg` | Cancelled speed too high | NA
209 | `ASP::CancelMsg` | Cancelled vehicle in motorway | NA
210 | `ASP::CancelMsg` | Reserved6 | NA
211 | `ASP::CancelMsg` | Reserved5 | NA
212 | `ASP::CancelMsg` | Reserved4 | NA
213 | `ASP::CancelMsg` | Reserved3 | NA
214 | `ASP::CancelMsg` | Reserved2 | NA
215 | `ASP::CancelMsg` | Reserved1 | NA
300 | `ASP::PauseMsg1` | None |
301 | `ASP::PauseMsg1` | Paused driver request | acknowledgement that maneuver has been paused
302 | `ASP::PauseMsg1` | Paused steering intervention |
303 | `ASP::PauseMsg1` | Paused manual gear change |
304 | `ASP::PauseMsg1` | Paused driver braked |
305 | `ASP::PauseMsg1` | Paused park brake |
306 | `ASP::PauseMsg1` | Paused accelerator pressed |
307 | `ASP::PauseMsg1` | Paused engine stalled | NA
308 | `ASP::PauseMsg1` | Paused door open |
309 | `ASP::PauseMsg1` | Paused boot open |
310 | `ASP::PauseMsg1` | Paused bonnet open |
311 | `ASP::PauseMsg1` | Paused obstacle detected |
312 | `ASP::PauseMsg1` | Pause ride height changed | NA
313 | `ASP::PauseMsg1` | Paused sensor performance | Camera Wash, Temporary Sensor Blockage, Stationary Timer Expired
314 | `ASP::PauseMsg1` | Remote communication lost | Inernal - may be delivered after the loss and reaquisition of wiFi signal
315 | `ASP::PauseMsg1` | Paused power low | NA
350 | `ASP::PauseMsg1` | DriveOn Initiated |
400 | `ASP::PauseMsg2` | None |
401 | `ASP::PauseMsg2` | Activity key in vehicle |
402 | `ASP::PauseMsg2` | Activity key outside legal distance | NA
403 | `ASP::PauseMsg2` | Activity key distance indeterminate |
404 | `ASP::PauseMsg2` | Activity key missing | NA
405 | `ASP::PauseMsg2` | Maximum distance reached | Keyfob out of range
406 | `ASP::PauseMsg2` | Maximum duration for operation reached | KeyFob was out of range for too long and ranging has stopped
407 | `ASP::PauseMsg2` | Application CRC failure | Internal - interrupt DMH, show oops
408 | `ASP::PauseMsg2` | Challenge and Response Mismatch | Internal - interrupt DMH, show oops
409 | `ASP::PauseMsg2` | DMH Invalid/Release | Internal - interrupt DMH, show oops
410 | `ASP::PauseMsg2` | Temporary System Failure | Internal - interrupt DMH, show oops
411 | `ASP::PauseMsg2` | Reserved | NA
500 | `ASP::InfoMsg` | None |
501 | `ASP::InfoMsg` | Searching for spaces | Inernal - should be overridden by the Scanning signal
502 | `ASP::InfoMsg` | Slow down search for spaces | NA
503 | `ASP::InfoMsg` | Slow down view spaces | NA
504 | `ASP::InfoMsg` | Drive forward space search | NA
505 | `ASP::InfoMsg` | Move suspension to normal height | NA
506 | `ASP::InfoMsg` | Space too small | NA
507 | `ASP::InfoMsg` | Narrow space available through remote only | NA
508 | `ASP::InfoMsg` | Space occupied | NA
509 | `ASP::InfoMsg` | Can't maneuver into space | NA
510 | `ASP::InfoMsg` | Bring vehicle to rest and apply brake | NA
511 | `ASP::InfoMsg` | Confirm and release brake to start | NA
512 | `ASP::InfoMsg` | Release brake to start | NA
513 | `ASP::InfoMsg` | Move indicator to change selection side | NA
514 | `ASP::InfoMsg` | No available park out | NA
515 | `ASP::InfoMsg` | Not enough space to park out | NA
516 | `ASP::InfoMsg` | Select park out maneuver | NA
517 | `ASP::InfoMsg` | Remote device connected and ready |
518 | `ASP::InfoMsg` | Remote device battery too low | NA
519 | `ASP::InfoMsg` | Remote maneuver ready | Inernal - should be indicated by default maneuver in available_maneuvers
520 | `ASP::InfoMsg` | Driver must be outside of vehicle | NA
521 | `ASP::InfoMsg` | Power reserves low | NA
522 | `ASP::InfoMsg` | Mind other road users disclaimer | NA
523 | `ASP::InfoMsg` | Engage reverse to start maneuver | NA
524 | `ASP::InfoMsg` | Approaching legal distance limit | NA
525 | `ASP::InfoMsg` | Approaching maximum distance | NA
526 | `ASP::InfoMsg` | Approaching maximum distance for operation | NA
527 | `ASP::InfoMsg` | Maneuver selected in vehicle | NA
600 | `ASP::InstructMsg` | None |
601 | `ASP::InstructMsg` | Bring vehicle to rest | NA
602 | `ASP::InstructMsg` | Release brakes to start | NA
603 | `ASP::InstructMsg` | Select R | NA
604 | `ASP::InstructMsg` | Select D | NA
605 | `ASP::InstructMsg` | Select First Gear | NA
606 | `ASP::InstructMsg` | Deselct R to display spaces | NA
607 | `ASP::InstructMsg` | Drive Forward | NA
608 | `ASP::InstructMsg` | Continue forward | NA
609 | `ASP::InstructMsg` | Drive backward | NA
610 | `ASP::InstructMsg` | Continue backward | NA
611 | `ASP::InstructMsg` | Stop | NA
612 | `ASP::InstructMsg` | Engage Park Brake | NA
613 | `ASP::InstructMsg` | Monitor manouevre in progress | NA
614 | `ASP::InstructMsg` | Remote manouevre in progress | Inernal - should be indicated by maneuver_status
615 | `ASP::InstructMsg` | Vehicle Stopping | NA
616 | `ASP::InstructMsg` | Vehicle stopped | NA
617 | `ASP::InstructMsg` | System Operation restricted capability reached | Cramp wheels operation failed
618 | `ASP::InstructMsg` | System operation restricted Occupant movement | NA
619 | `ASP::InstructMsg` | Press accelerator to resume | NA
620 | `ASP::InstructMsg` | Pause Manuouevre | NA
621 | `ASP::InstructMsg` | Reserved | NA
622 | `ASP::InstructMsg` | Reserved | NA
623 | `ASP::InstructMsg` | Reserved | NA
624 | `ASP::InstructMsg` | Reserved | NA
625 | `ASP::InstructMsg` | Reserved | NA
626 | `ASP::InstructMsg` | Reserved | NA
627 | `ASP::InstructMsg` | Reserved | NA
628 | `ASP::InstructMsg` | Reserved | NA
629 | `ASP::InstructMsg` | Reserved | NA
630 | `ASP::InstructMsg` | Reserved | NA
631 | `ASP::InstructMsg` | Reserved | NA
700 | `DCM::AcknowledgeRemotePIN` | None |
701 | `DCM::AcknowledgeRemotePIN` | Incorrect PIN |
702 | `DCM::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock 60s |
703 | `DCM::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock 300s |
704 | `DCM::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock 3600s |
705 | `DCM::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock Indefinite |
706 | `DCM::AcknowledgeRemotePIN` | PIN Expired | if Selecting or Maneuvering, require entry of PIN before proceeding. *not currently implemented
707 | `DCM::AcknowledgeRemotePIN` | Internal PIN is not yet set |
708 | `DCM::AcknowledgeRemotePIN` | Correct PIN |
800 | `DCM::ErrorMsg` | None |
801 | `DCM::ErrorMsg` | Electric charger connected to vehicle |
802 | `DCM::ErrorMsg` | Stolen vehicle tracking alert detected | show catastrophic error
803 | `DCM::ErrorMsg` | Max attempts to remote start vehicle reached | show catastrophic error
804 | `DCM::ErrorMsg` | Engine start not possible due to vehicle crash | show catastrophic error
805 | `DCM::ErrorMsg` | Vehicle fuel too low | show catastrophic error
806 | `DCM::ErrorMsg` | BCM error | show catastrophic error
807 | `DCM::ErrorMsg` | Multiple key fobs detected (ranging error) |
808 | `DCM::ErrorMsg` | Vehicle battery too low | show catastrophic error
809 | `DCM::ErrorMsg` | Remote session expiring soon |
810 | `DCM::ErrorMsg` | Reserved9 | NA
811 | `DCM::ErrorMsg` | Reserved8 | NA
812 | `DCM::ErrorMsg` | Reserved7 | NA
813 | `DCM::ErrorMsg` | Reserved6 | NA
814 | `DCM::ErrorMsg` | Reserved5 | NA
815 | `DCM::ErrorMsg` | Reserved4 | NA
816 | `DCM::ErrorMsg` | Reserved3 | NA
817 | `DCM::ErrorMsg` | Reserved2 | NA
818 | `DCM::ErrorMsg` | Reserved1 | NA
819 | `DCM::ErrorMsg` | Reserved0 | NA
820 | `DCM::ErrorMsg` | WiFi advertising to cease in 30 seconds | alert user to impending session end
821 | `DCM::ErrorMsg` | WiFi advertising to cease in 1 minute | alert user to impending session end
822 | `DCM::ErrorMsg` | WiFi advertising to cease in 2 minute | alert user to impending session end
823 | `DCM::ErrorMsg` | WiFi advertising to cease in 3 minute | alert user to impending session end
824 | `DCM::ErrorMsg` | WiFi advertising to cease in 4 minute | alert user to impending session end
825 | `DCM::ErrorMsg` | WiFi advertising to cease in 5 minute | occurs after session reaches 30 minutes - alert user to impending session end
900 | `ASP::ManeuverStatus` | Not Active |
901 | `ASP::ManeuverStatus` | Scanning | Internal - show scanning screen
902 | `ASP::ManeuverStatus` | Selecting | Internal - show available maneuvers
903 | `ASP::ManeuverStatus` | Confirming | Internal - show PIVI-selected maneuver
904 | `ASP::ManeuverStatus` | Maneuvering | Internal - show maneuvering screen
905 | `ASP::ManeuverStatus` | Interrupted | Internal - if user released DMH: igore, else: interrupt DMH, show oops
906 | `ASP::ManeuverStatus` | Finishing | Internal - show maneuver end screen
907 | `ASP::ManeuverStatus` | Ended | Internal - session has ended / vehicle is being secured / inactivity timer has expired
908 | `ASP::ManeuverStatus` | RCStartStop | NA
909 | `ASP::ManeuverStatus` | Holding | NA
910 | `ASP::ManeuverStatus` | Cancelled | Internal - show maneveur cancelled / end maneuver screen
911 | `ASP::ManeuverStatus` | Reserved5 | NA
912 | `ASP::ManeuverStatus` | Reserved4 | NA
913 | `ASP::ManeuverStatus` | Reserved3 | NA
914 | `ASP::ManeuverStatus` | Reserved2 | NA
915 | `ASP::ManeuverStatus` | Reserved1 | NA
