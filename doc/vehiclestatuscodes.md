## Special Handling
**May be ignored by general status handlers:**
* The 3xx and 4xx series of Pause codes are only relevant during Selecting and Maneuvering.
* The 7xx series of PIN codes are only relevant during user verification.
* The 9xx series of Maneuver codes are only relevant during a maneuver.


# Status Codes

* NA indicates that the signal should not be sent to the phone

Status Code | Root Signal | Signal Text | App Action
--- | --- | --- | ---
100 | `Sensing::NoFeatureAvailableMsg` | None |
101 | `Sensing::NoFeatureAvailableMsg` | Not available system fault | show catastrophic error
102 | `Sensing::NoFeatureAvailableMsg` | Not available sensor blocked | show catastrophic error
103 | `Sensing::NoFeatureAvailableMsg` | Not available vehicle not started | NA
104 | `Sensing::NoFeatureAvailableMsg` | Not available tilt too strong | show catastrophic error
105 | `Sensing::NoFeatureAvailableMsg` | Not available trailer connected | suggest user action
106 | `Sensing::NoFeatureAvailableMsg` | Not available ride height | NA
107 | `Sensing::NoFeatureAvailableMsg` | Not available speed too high | suggest user action
108 | `Sensing::NoFeatureAvailableMsg` | Not available ATPC | suggest user action (All Train Progress Control)
109 | `Sensing::NoFeatureAvailableMsg` | Not available Tow Assist on | NA
110 | `Sensing::NoFeatureAvailableMsg` | Not available Wade Assist on | NA
111 | `Sensing::NoFeatureAvailableMsg` | Not available ACC+ on | suggest user action (Adaptive Cruise Ctrl, Country Road Assist)
112 | `Sensing::NoFeatureAvailableMsg` | Not available TJP on | NA
113 | `Sensing::NoFeatureAvailableMsg` | Not available vehicle on motorway | NA
114 | `Sensing::NoFeatureAvailableMsg` | Reserved2 | NA
115 | `Sensing::NoFeatureAvailableMsg` | Reserved1 | NA
200 | `Sensing::CancelMsg` | None | for the series, cancel and return to home screen
201 | `Sensing::CancelMsg` | Cancelled from loss of traction |
202 | `Sensing::CancelMsg` | Cancelled max number of moves reached |
203 | `Sensing::CancelMsg` | Cancelled paused for too long | Inactivty timer expired
204 | `Sensing::CancelMsg` | Cancelled internal system failure | NA
205 | `Sensing::CancelMsg` | Cancelled trailer connected | NA
206 | `Sensing::CancelMsg` | Maneuver cancelled driver request | acknowledgement of requested cancellation
207 | `Sensing::CancelMsg` | Maneuver cancelled vehicle driven on | acknowledgement that user is no longer using RPA
208 | `Sensing::CancelMsg` | Cancelled speed too high | NA
209 | `Sensing::CancelMsg` | Cancelled vehicle in motorway | NA
210 | `Sensing::CancelMsg` | Reserved6 | NA
211 | `Sensing::CancelMsg` | Reserved5 | NA
212 | `Sensing::CancelMsg` | Reserved4 | NA
213 | `Sensing::CancelMsg` | Reserved3 | NA
214 | `Sensing::CancelMsg` | Reserved2 | NA
215 | `Sensing::CancelMsg` | Reserved1 | NA
300 | `Sensing::PauseMsg1` | None |
301 | `Sensing::PauseMsg1` | Paused driver request | acknowledgement that maneuver has been paused
302 | `Sensing::PauseMsg1` | Paused steering intervention |
303 | `Sensing::PauseMsg1` | Paused manual gear change |
304 | `Sensing::PauseMsg1` | Paused driver braked |
305 | `Sensing::PauseMsg1` | Paused park brake |
306 | `Sensing::PauseMsg1` | Paused accelerator pressed |
307 | `Sensing::PauseMsg1` | Paused engine stalled | NA
308 | `Sensing::PauseMsg1` | Paused door open |
309 | `Sensing::PauseMsg1` | Paused boot open |
310 | `Sensing::PauseMsg1` | Paused bonnet open |
311 | `Sensing::PauseMsg1` | Paused obstacle detected |
312 | `Sensing::PauseMsg1` | Pause ride height changed | NA
313 | `Sensing::PauseMsg1` | Paused sensor performance | Camera Wash, Temporary Sensor Blockage, Stationary Timer Expired
314 | `Sensing::PauseMsg1` | Remote communication lost | Inernal - may be delivered after the loss and reaquisition of wiFi signal
315 | `Sensing::PauseMsg1` | Paused power low | NA
350 | `Sensing::PauseMsg1` | DriveOn Initiated |
400 | `Sensing::PauseMsg2` | None |
401 | `Sensing::PauseMsg2` | Activity key in vehicle |
402 | `Sensing::PauseMsg2` | Activity key outside legal distance | NA
403 | `Sensing::PauseMsg2` | Activity key distance indeterminate |
404 | `Sensing::PauseMsg2` | Activity key missing | NA
405 | `Sensing::PauseMsg2` | Maximum distance reached | Keyfob out of range
406 | `Sensing::PauseMsg2` | Maximum duration for operation reached | KeyFob was out of range for too long and ranging has stopped
407 | `Sensing::PauseMsg2` | Application CRC failure | Internal - interrupt DMH, show oops
408 | `Sensing::PauseMsg2` | Challenge and Response Mismatch | Internal - interrupt DMH, show oops
409 | `Sensing::PauseMsg2` | DMH Invalid/Release | Internal - interrupt DMH, show oops
410 | `Sensing::PauseMsg2` | Temporary System Failure | Internal - interrupt DMH, show oops
411 | `Sensing::PauseMsg2` | Reserved | NA
500 | `Sensing::InfoMsg` | None |
501 | `Sensing::InfoMsg` | Searching for spaces | Inernal - should be overridden by the Scanning signal
502 | `Sensing::InfoMsg` | Slow down search for spaces | NA
503 | `Sensing::InfoMsg` | Slow down view spaces | NA
504 | `Sensing::InfoMsg` | Drive forward space search | NA
505 | `Sensing::InfoMsg` | Move suspension to normal height | NA
506 | `Sensing::InfoMsg` | Space too small | NA
507 | `Sensing::InfoMsg` | Narrow space available through remote only | NA
508 | `Sensing::InfoMsg` | Space occupied | NA
509 | `Sensing::InfoMsg` | Can't maneuver into space | NA
510 | `Sensing::InfoMsg` | Bring vehicle to rest and apply brake | NA
511 | `Sensing::InfoMsg` | Confirm and release brake to start | NA
512 | `Sensing::InfoMsg` | Release brake to start | NA
513 | `Sensing::InfoMsg` | Move indicator to change selection side | NA
514 | `Sensing::InfoMsg` | No available park out | NA
515 | `Sensing::InfoMsg` | Not enough space to park out | NA
516 | `Sensing::InfoMsg` | Select park out maneuver | NA
517 | `Sensing::InfoMsg` | Remote device connected and ready |
518 | `Sensing::InfoMsg` | Remote device battery too low | NA
519 | `Sensing::InfoMsg` | Remote maneuver ready | Inernal - should be indicated by default maneuver in available_maneuvers
520 | `Sensing::InfoMsg` | Driver must be outside of vehicle | NA
521 | `Sensing::InfoMsg` | Power reserves low | NA
522 | `Sensing::InfoMsg` | Mind other road users disclaimer | NA
523 | `Sensing::InfoMsg` | Engage reverse to start maneuver | NA
524 | `Sensing::InfoMsg` | Approaching legal distance limit | NA
525 | `Sensing::InfoMsg` | Approaching maximum distance | NA
526 | `Sensing::InfoMsg` | Approaching maximum distance for operation | NA
527 | `Sensing::InfoMsg` | Maneuver selected in vehicle | NA
600 | `Sensing::InstructMsg` | None |
601 | `Sensing::InstructMsg` | Bring vehicle to rest | NA
602 | `Sensing::InstructMsg` | Release brakes to start | NA
603 | `Sensing::InstructMsg` | Select R | NA
604 | `Sensing::InstructMsg` | Select D | NA
605 | `Sensing::InstructMsg` | Select First Gear | NA
606 | `Sensing::InstructMsg` | Deselct R to display spaces | NA
607 | `Sensing::InstructMsg` | Drive Forward | NA
608 | `Sensing::InstructMsg` | Continue forward | NA
609 | `Sensing::InstructMsg` | Drive backward | NA
610 | `Sensing::InstructMsg` | Continue backward | NA
611 | `Sensing::InstructMsg` | Stop | NA
612 | `Sensing::InstructMsg` | Engage Park Brake | NA
613 | `Sensing::InstructMsg` | Monitor manouevre in progress | NA
614 | `Sensing::InstructMsg` | Remote manouevre in progress | Inernal - should be indicated by maneuver_status
615 | `Sensing::InstructMsg` | Vehicle Stopping | NA
616 | `Sensing::InstructMsg` | Vehicle stopped | NA
617 | `Sensing::InstructMsg` | System Operation restricted capability reached | Cramp wheels operation failed
618 | `Sensing::InstructMsg` | System operation restricted Occupant movement | NA
619 | `Sensing::InstructMsg` | Press accelerator to resume | NA
620 | `Sensing::InstructMsg` | Pause Manuouevre | NA
621 | `Sensing::InstructMsg` | Reserved | NA
622 | `Sensing::InstructMsg` | Reserved | NA
623 | `Sensing::InstructMsg` | Reserved | NA
624 | `Sensing::InstructMsg` | Reserved | NA
625 | `Sensing::InstructMsg` | Reserved | NA
626 | `Sensing::InstructMsg` | Reserved | NA
627 | `Sensing::InstructMsg` | Reserved | NA
628 | `Sensing::InstructMsg` | Reserved | NA
629 | `Sensing::InstructMsg` | Reserved | NA
630 | `Sensing::InstructMsg` | Reserved | NA
631 | `Sensing::InstructMsg` | Reserved | NA
700 | `Chassis::AcknowledgeRemotePIN` | None |
701 | `Chassis::AcknowledgeRemotePIN` | Incorrect PIN |
702 | `Chassis::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock 60s |
703 | `Chassis::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock 300s |
704 | `Chassis::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock 3600s |
705 | `Chassis::AcknowledgeRemotePIN` | Incorrect PIN 3x Lock Indefinite |
706 | `Chassis::AcknowledgeRemotePIN` | PIN Expired | if Selecting or Maneuvering, require entry of PIN before proceeding. *not currently implemented
707 | `Chassis::AcknowledgeRemotePIN` | Internal PIN is not yet set |
708 | `Chassis::AcknowledgeRemotePIN` | Correct PIN |
800 | `Chassis::ErrorMsg` | None |
801 | `Chassis::ErrorMsg` | Electric charger connected to vehicle |
802 | `Chassis::ErrorMsg` | Stolen vehicle tracking alert detected | show catastrophic error
803 | `Chassis::ErrorMsg` | Max attempts to remote start vehicle reached | show catastrophic error
804 | `Chassis::ErrorMsg` | Engine start not possible due to vehicle crash | show catastrophic error
805 | `Chassis::ErrorMsg` | Vehicle fuel too low | show catastrophic error
806 | `Chassis::ErrorMsg` | BCM error | show catastrophic error
807 | `Chassis::ErrorMsg` | Multiple key fobs detected (ranging error) |
808 | `Chassis::ErrorMsg` | Vehicle battery too low | show catastrophic error
809 | `Chassis::ErrorMsg` | Remote session expiring soon |
810 | `Chassis::ErrorMsg` | Reserved9 | NA
811 | `Chassis::ErrorMsg` | Reserved8 | NA
812 | `Chassis::ErrorMsg` | Reserved7 | NA
813 | `Chassis::ErrorMsg` | Reserved6 | NA
814 | `Chassis::ErrorMsg` | Reserved5 | NA
815 | `Chassis::ErrorMsg` | Reserved4 | NA
816 | `Chassis::ErrorMsg` | Reserved3 | NA
817 | `Chassis::ErrorMsg` | Reserved2 | NA
818 | `Chassis::ErrorMsg` | Reserved1 | NA
819 | `Chassis::ErrorMsg` | Reserved0 | NA
820 | `Chassis::ErrorMsg` | WiFi advertising to cease in 30 seconds | alert user to impending session end
821 | `Chassis::ErrorMsg` | WiFi advertising to cease in 1 minute | alert user to impending session end
822 | `Chassis::ErrorMsg` | WiFi advertising to cease in 2 minute | alert user to impending session end
823 | `Chassis::ErrorMsg` | WiFi advertising to cease in 3 minute | alert user to impending session end
824 | `Chassis::ErrorMsg` | WiFi advertising to cease in 4 minute | alert user to impending session end
825 | `Chassis::ErrorMsg` | WiFi advertising to cease in 5 minute | occurs after session reaches 30 minutes - alert user to impending session end
900 | `Sensing::ManeuverStatus` | Not Active |
901 | `Sensing::ManeuverStatus` | Scanning | Internal - show scanning screen
902 | `Sensing::ManeuverStatus` | Selecting | Internal - show available maneuvers
903 | `Sensing::ManeuverStatus` | Confirming | Internal - show PIVI-selected maneuver
904 | `Sensing::ManeuverStatus` | Maneuvering | Internal - show maneuvering screen
905 | `Sensing::ManeuverStatus` | Interrupted | Internal - if user released DMH: igore, else: interrupt DMH, show oops
906 | `Sensing::ManeuverStatus` | Finishing | Internal - show maneuver end screen
907 | `Sensing::ManeuverStatus` | Ended | Internal - session has ended / vehicle is being secured / inactivity timer has expired
908 | `Sensing::ManeuverStatus` | RCStartStop | NA
909 | `Sensing::ManeuverStatus` | Holding | NA
910 | `Sensing::ManeuverStatus` | Cancelled | Internal - show maneveur cancelled / end maneuver screen
911 | `Sensing::ManeuverStatus` | Reserved5 | NA
912 | `Sensing::ManeuverStatus` | Reserved4 | NA
913 | `Sensing::ManeuverStatus` | Reserved3 | NA
914 | `Sensing::ManeuverStatus` | Reserved2 | NA
915 | `Sensing::ManeuverStatus` | Reserved1 | NA
