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
 * \file Launching point for development API.  This instantiates a simulated NFS.
 *
 * \author fdaniel
 */

#include "aspm.hpp"


/**
 * Entry point for the API.  The node will create on object of class MsgParser
 * and listen for messages from a mobile device.  The "initiateEventLoops( )"
 * is a blocking call; users must use Ctrl-C to exit this function.
 */
int main( int argc, char *argv[ ] )
{
    MANOUEVRE inputTestManeuver( MANOUEVRE::NADA );

    if( argc > 1 && (std::string)argv[1] == "CI_BYPASS" )
    {
        std::cout << "Bypassing server creation; terminating." << std::endl;

        return 0;
    }
    else if( argc > 1 )
    {
        inputTestManeuver = ASPM::parseManeuverString( (std::string)argv[1] );
    }

    ASPM aspm( SIMULATOR::BENCH, inputTestManeuver );

    aspm.initiateEventLoops( );

    std::cout << "Press Enter to Exit" << std::endl;
    std::string str;
    while (std::getline(std::cin, str) && !str.empty()) {}

    return 0;

}
