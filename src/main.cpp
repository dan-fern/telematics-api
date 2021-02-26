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
 * \file Launching point for production API.
 *
 * \author fdaniel, trice2
 */

#include "remotedevicehandler.hpp"


/**
 * Entry point for the API.  The node will create a RemoteDeviceHandler object
 * and listen for messages from a mobile device.  The "spin( )" is a blocking
 * call; users must use Ctrl-C to exit this function.
 */
int main( int argc, char *argv[ ] )
{

    if( argc > 1 && (std::string)argv[1] == "CI_BYPASS" )
    {
        std::cout << "Bypassing server creation; terminating." << std::endl;

        return 0;
    }

    RemoteDeviceHandler jsonParser( std::make_shared <SignalHandler>( ) );

    jsonParser.spin( );


    return 0;

}
