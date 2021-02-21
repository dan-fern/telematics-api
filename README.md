# **`telematics-api`**

### version: `0.0.17`

This is an example repository for demonstrating communications layer architecture between a mobile device and an on-board, vehicle-agnostic telematics ECU.  The classes and signal set are abstracted.  

These classes:
*  statefully observe the signals broadcast by a sensory ECU regarding maneuvers and vehicle state (`Sensing` --> `Remote`),
*  implement the [JSON API](doc/api/API.md) for communicating between a mobile phone and a vehicle telematics ECU (`Remote` --> `Sensing`),
*  implement keyfob ranging requests to a chassis ECU based on the current activity being undertaken (`Chassis` <--> `Remote`), and
*  perform all atomic signal conversions for the above processes.

## Build Instructions

Install required dependencies by opening a terminal and calling:
```bash
$ sudo apt-get update -qy
$ sudo apt-get install git python cmake doxygen graphviz gcovr -qy
```

Clone the repository to your machine by calling:

```bash
$ git clone https://github.com/dan-fern/telematics-api.git
```

If prompted, enter your GitHub login credentials.

Navigate to the repository directory and build the project by calling:

```bash
$ cd telematics-api/
$ ./build.sh
```

To build the repository with unit tests, documentation, and a sensory ECU simulator, run:
```bash
$ ./build.sh --test --docs --sim
```
or pass `--test`, `--docs`, or `--sim` to build individual features.

## Run Instructions

The C++ API is built in the `build` directory.  Launch it by calling:

```bash
$ ./build/telematics-api
```

Of course, without target hardware in the loop, this app will not return usable data.  To develop the mobile app alongside the API, you probably need to use the app alongside the sensory simulator.

## Run Instructions (Development Mode)

Running the simulator makes use of separate terminal windows.  First off, make sure that you have built the simulator by passing the `--sim` or `-s` flags to the top level build script:

```bash
$ ./build.sh --sim
```

Now, in one terminal, launch the simulator by calling:
```bash
$ ./build/utils/sensing_simulator/telematics-api-sim
```

The simulated sensory ECU is awaiting connection to the telematics ECU API.  You can launch that by opening a second terminal and calling:

```bash
$ ./build/telematics-api
```

The API will spin up and the simulated sensory ECU will await message input.  Nothing will happen since there is no mobile device connected.  If you don't have a functional mobile app handy, never fear.  You can pass a message to the simulated system by opening a third terminal, navigating to the `utils` directory, and running the python handler by calling:

```bash
$ cd utils/
$ python mobile_placeholder.py
```

**NOTE**: You can also run `mobile_placeholder.py` in verbose mode like so (this will display JSON for `mobile_challenge`/`mobile_response` calls):

```bash
$ python mobile_placeholder.py -v
```

#### Under the Hood

The python script acts as a mobile device in this example.  When the main menu appears on the python side, pass `vehicle_status` message to the API.  The C++ App will display the current vehicle status.  If no other messages have been passed prior, the status should read `0` or `NotActive`.

Run the python handler again and pass the `send_pin` and `mobile_init` messages to the API by selecting them.  The App will report back various vehicle signals that are set as a result of initialization processes being sent to the sensory ECU.  To see the vehicle status another time, again pass the `vehicle_status` to the API, and the app should return `1` or `2` for `Scanning` or `Selecting`, respectively.

**NOTE**: If a valid PIN has not been passed via `send_pin` or if terms were not agreed upon in `mobile_init`, then the vehicle may report a different status.

#### Keyfob Ranging
Keyfob ranging requests will be handled as a consequence of the state as follows:

| App Connected | In Maneuver | Range Request Frequency |
| -- | -- | -- |
| No | No | None |
| Yes | No | 5 sec |
| Yes | Yes | 500 msec |

The `SignalHandler` class is built with several event loops running in parallel.  One called `updateSignalEventLoop_( )` is used to update all `XXX` signals every 30ms, or according to `XXX_REFRESH_RATE`.  The second called `rangingRequestEventLoop_( )` is used to request key fob ranging detection at a variable rate, depending on the state of the application, or:

```cpp
enum class FobRangeRequestRate : unsigned int
{
    // Holds the fob ranging request rate values, in μs
    None = 0,
    DefaultRate = 5000000,
    DeadmanRate = 500000
};
```

where the `DefaultRate` of 5 seconds is used at any point a mobile device is connected to the API and the `DeadmanRate` of 0.5 seconds is used during any maneuver execution.  Fob ranging reverts to `None` upon disconnecting from a client.

#### Vehicle Status Codes

The API transmits the `vehicle_status` message when requested or when there is a change in the following signals:
* `Sensing::NoFeatureAvailableMsg` -- 1xx
* `Sensing::CancelMsg` -- 2xx
* `Sensing::PauseMsg1` -- 3xx
* `Sensing::PauseMsg2` -- 4xx
* `Sensing::InfoMsg` -- 5xx
* `Sensing::InstructMsg` -- 6xx
* `Chassis::AcknowledgeRemotePIN` -- 7xx
* `Chassis::ErrorMsg` -- 8xx
* `Sensing::ManeuverStatus` -- 9xx

and those codes are reported according to [this table](doc/vehiclestatuscodes.md).

**NOTE**: `Chassis::AcknowlegeRemotePIN` and `Chassis::ErrorMessagesApp` are signals which originate from the chassis module and not the sensory module.

## Run Tests

To run the unit tests, build the repository using the `--tests` or `-t` flag, or:
```bash
$ cd telematics-api/
$ ./build.sh --tests
$ ./build/test/telematics-api-tests
```

## Coverage Report

To generate an HTML coverage report, use the `--coverage` flag in addition to the `--tests` flag while building:

```bash
$ cd telematics-api/
$ ./build.sh --tests --coverage
```

This will automatically run the unit tests and generate a report.  Open `build/coverage/coverage.html` in your browser to see which parts of the code have been covered by the unit tests.

## Run Docs

To view advanced documentation, build the repository using the `--docs` or `-d` flag, or:
```bash
$ cd telematics-api/
$ ./build.sh --docs
```
and navigate to the `doc/html/index.html` homepage.

## Conclusions

This demonstration presents an example methodology for signal abstraction at the mobile device layer.
