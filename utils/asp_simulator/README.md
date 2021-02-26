### `utils/asp_simulator`
This module builds a standalone ASP Module (ASPM) simulator node that is designed to communicate to the production API over UDP.  To run it, run the top-level `build.sh` script with the `-s` or `--sim` flags:

```bash
$ ./build.sh --sim
$ ./build/utils/asp_simulator/telematics-api-sim
```

#### Preferred Maneuver Support
By default, the ASPM outputs Exploratory (a.k.a. Push/Pull, StrFwd/StrRvs) maneuvers alongside random maneuvers.  To indicate a preferred test maneuver, pass the maneuver code at runtime, e.g.:
```bash
$ ./build/utils/asp_simulator/telematics-api-sim OutRgtFwd
```
for passing in the `"OutRgtFwd"` maneuver.  Alternatively, if building as a dependency, this can be added to the `ASPM` constructor e.g.:
```cpp
ASPM ASPM( SIMULATOR::EV3, MANOUEVRE::PORF );
```

The simulated scan will then output this maneuver as available for execution.  Supported maneuvers and respective arguments are viewable in the table below:

Maneuver Name | Run-Time Arguments | `ASPM` Constructor Arguments
--- | --- | ---
`"OutLftFwd"` | `OutLftFwd` OR `OLF` OR `POLF` | `MANOUEVRE::POLF`
`"OutLftRvs"` | `OutLftRvs` OR `OLR` OR `POLR` | `MANOUEVRE::POLR`
`"OutRgtFwd"` | `OutRgtFwd` OR `ORF` OR `PORF` | `MANOUEVRE::PORF`
`"OutRgtRvs"` | `OutRgtRvs` OR `ORR` OR `PORR` | `MANOUEVRE::PORR`
`"OutLftPrl"` | `OutLftPrl` OR `OLP` OR `POLP` | `MANOUEVRE::POLP`
`"OutRgtPrl"` | `OutRgtPrl` OR `ORP` OR `PORP` | `MANOUEVRE::PORP`
`"StrFwd"` | `StrFwd` OR `SF` | `MANOUEVRE::SF`
`"StrRvs"` | `StrRvs` OR `SR` | `MANOUEVRE::SR`

To change the preferred sim maneuver after instantiation, leverage the `ASPM::changePreferredSimMove( const std::string& maneuver )` function and pass a `std::string` according to the table above, e.g.:
```cpp
ASPM.changePreferredSimMove( "OutLftPrl" );
```
This will generate parallel maneuvers upon a new simulated vehicle proximity scan.
