# Telematics API Docs  

See [API](API.md) for full communication specifications.  
API contains links to [Challenge-CRC](Challenge-CRC.md) and [Vehicle Status Codes](./Vehicle_Status_Codes.md)  


**Communication round-trip from Mobile to Vehicle and back**

Mobile <-> vehicle signals are conveyed over a TLS-secured TCP socket using JSON. This is a proven, reliable, secure combination of technologies to ensure consistent communications.  

Telematics <-> Sensory signals are conveyed over UDP. This is an error-prone and unreliable technology that must be error-checked and repeated in order to ensure consistent communications.  

Mobile <-> vehicle signals are sent one time each because of the robustness of the technologies employed.  
Example signal:
```json
{
    "maneuver" : "StrFwd"
}
```

In the Telematics ECU resides a translation Library which takes the JSON signals and converts them to the list of signals used by the Sensory ECU. In the other direction, it looks for changes in the constantly-emitted atomic signals and converts them to aggregate JSON signals.

Example JSON signal translated to atomic signals:
```
        InvalidSig = 255,
        None = 0,
        NoseFirst = 1,
        RearFirst = 0,
        Reserved1 = 0

```

Telematics -> Sensory signals are sent once (or until reflected in the NFS signals) to alert the Sensory ECU to changes from the Mobile Device.  
Sensory -> Telematics signals are ALL sent every 30ms regardless of whether any changes have occurred.  


**Making changes to the API**
* The version number is in the version file and should only be updated there
* All JSON code is located in the .json files and should only be updated in the approprite file
* Data for API, Challenge-CRC and Vehicle Status Codes are located in their similarly named _template files and should only be updated there
* After changes are completed, run the merge_json_to_md script to integrate all these data sources into the comprehensive files shown to the reader.
