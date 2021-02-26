### To be used in the project's `utils` folder.

import sys
import json
import time
import random
import socket
import select
import struct
import hashlib
import os.path
import threading
from multiprocessing import Queue

HOST, PORT = "localhost", 8063
VERBOSE = False

def make_new_socket( ):
    return socket.socket( socket.AF_INET, socket.SOCK_STREAM )


def print_menu( menu ):
    options = menu.keys( )
    options.sort( )
    for entry in options:
        print( "%s %s" % ( entry, menu[entry] ) )


def send_message( socket, data, header ):

    if ( data != 'disconnectMobile' and (header["group"] != "mobile_response" or VERBOSE)):
        print( json.dumps( header, indent=4, sort_keys=True ) )
        print( json.dumps( data, indent=4, sort_keys=True ) )

    header_out = json.dumps( header, sort_keys=True )
    header_out.encode( encoding='utf_8', errors='strict' )
    header_len = struct.pack( '!i', len( header_out ) )

    data_out = json.dumps( data, sort_keys=True )
    data_out.encode( encoding='utf_8', errors='strict' )
    data_len = struct.pack( '!i', len( data_out ) )

    #print( json.dumps( msg_out, indent=4, sort_keys=True ) )
    if data is None:
        data_len = struct.pack( '!i', 0 )
        data_out = None
        while header_len:
            header_len = header_len[ socket.send( header_len ): ]
        while data_len:
            data_len = data_len[ socket.send( data_len ): ]
        while header_out:
            header_out = header_out[ socket.send( header_out ): ]

    else:
        while header_len:
            header_len = header_len[ socket.send( header_len ): ]
        while data_len:
            data_len = data_len[ socket.send( data_len ): ]
        while header_out:
            header_out = header_out[ socket.send( header_out ): ]
        while data_out:
            data_out = data_out[ socket.send( data_out ): ]

    # time.sleep( 0.25 )

    return


def receive_message( socket ):

    header_size = struct.unpack( ">i", socket.recv( 4 ) )[0]
    body_size = struct.unpack( ">i", socket.recv( 4 ) )[0]

    total_msg = socket.recv( (header_size + body_size) )

    msg_header = total_msg[ :header_size ]
    msg_body = total_msg[ -body_size: ]

    try:
        header_json = json.loads( msg_header )
        body_json = json.loads( msg_body )

    except ValueError:
        print( "ValueError on return_msg; could not parse JSON." )
        print( msg_header )
        print( msg_body )
        print( "---" )
        return json.loads({"none"}), json.loads({"none"})


    return header_json, body_json


def get_crc_json( ):
    pass


t = time.time( )

hdr_queue = Queue( )
bdy_queue = Queue( )

challenge_response = '0'
threading_condition = threading.Condition( )

test_sock = make_new_socket( )

try:
    test_sock.connect( ( HOST, PORT ) )
    test_sock.setblocking( 0 )
    test_sock.close( )
    print( "Test socket created and closed." )
except:
    print( "No available host.  Disconnecting!!")
    sys.exit( 0 )

def calc_response(packed_bytes):
    response = int(packed_bytes)
    index = (response & 0xFF00000000000000) >> (8 * 7)
    part1 = (response & 0x00FF000000000000) >> (8 * 6)
    part2 = (response & 0x0000FF0000000000) >> (8 * 5)
    part3 = (response & 0x000000FF00000000) >> (8 * 4)
    resp = (((((part1 * part2) >> (part3 & 0x0F)) + ((part3 ^ part1) << ((part2 - part1) & 0x07)))) // (0x03))
    resp = resp | (index << (8 * 3))
    return str((resp << (8 * 4)))


class Comms( threading.Thread ):
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name

    def run(self):

        global hdr_queue
        global bdy_queue
        global challenge_response

        sock = make_new_socket( )

        try:
            sock.connect( ( HOST, PORT ) )
            sock.setblocking( 0 )
            print( "Mobile device connected. Ready send/receive JSON." )
        except:
            print( "No available host.  Try again!!")
            sys.exit( 0 )
            return


        while True:

            while hdr_queue.qsize( ) > 0:

                # print( hdr_queue.qsize( ) )
                # print( bdy_queue.qsize( ) )

                threading_condition.acquire( )

                send_header = hdr_queue.get( )
                send_body = bdy_queue.get( )

                send_message( sock, send_body, send_header )
                threading_condition.release( )

                if ( send_body == 'disconnectMobile' ):
                    sock.close( )
                    sys.exit( 0 )
                    return

                # message_log.append( queue.get( ) )

            try:
                ready = select.select( [sock], [], [], 0.33 )

                if ready[0]:
                    header_json, body_json = receive_message( sock )
                else:
                    # print( "No message on the wire." )
                    continue

                # hdr_queue.put( header_json )

                if header_json[ "group" ] == "mobile_challenge":
                    crc_header = header_json.copy( )
                    with open( "example_json/mobile_response.json" ) as file:
                        crc_body = json.load( file )
                    threading_condition.acquire( )
                    challenge_response = calc_response(body_json[ "packed_bytes" ])
                    threading_condition.release( )
                    crc_header[ "group" ] = "mobile_response"
                    crc_body[ "response_to_challenge" ] = challenge_response
                    send_message( sock, crc_body, crc_header )
                    if not VERBOSE:
                        continue

                print( "Receipt confirmed from TCM: " )
                print( json.dumps( header_json, indent=4, sort_keys=True ) )
                print( json.dumps( body_json, indent=4, sort_keys=True ) )
                print( "---" )

            except:
                print( "Socket error attempting to receive message; retrying." )
                print( "---" )


        return


class Prompt( threading.Thread ):
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name

    def run(self):

        global hdr_queue
        global bdy_queue
        global challenge_response

        message_log = [ ]
        msg_group = ''
        json_path = ''
        no_body = False
        # expecting_response = True

        # Load msg header
        with open( 'example_json/header.json' ) as file:
            # msg_header = json.dumps( json.load( file ), indent=4, sort_keys=True )
            msg_header = json.load( file )

        while True:

            try:
                # '../res/write/xxx.json'
                if no_body:
                    msg_body = None
                    # bdy_queue.put("null")
                else:
                    with open( json_path ) as file:
                        # json_out = json.dumps( json.load( file ), indent=4, sort_keys=True )
                        msg_body = json.load( file )

                msg_header[ "group" ] = msg_group

                if msg_group == 'send_pin':
                    pin_input = raw_input( "Please enter PIN value: " )
                    msg_body[ "pin" ] = hashlib.sha256(pin_input.encode()).hexdigest()

                elif msg_group == 'mobile_init':
                    terms_input = None
                    while terms_input not in ("y", "n", "yes", "no"):
                        terms_input = raw_input( "Do you accept terms (Enter yes or no): ")
                        if terms_input == "yes" or terms_input == "y":
                            msg_body[ "terms_accepted" ] = True
                        elif terms_input == "no" or terms_input == "n":
                            msg_body[ "terms_accepted" ] = False
                        else:
                        	print("Please enter yes or no.")

                elif msg_group == 'maneuver_init':
                    # maneuvers = [ "StrRvs", "StrFwd" ]
                    # msg_body[ "maneuver" ] = random.choice( maneuvers )

                    move_menu = { }
                    move_menu[' 0. '] = "StrFwd"
                    move_menu[' 1. '] = "StrRvs"
                    move_menu[' 2. '] = "OutLftFwd"
                    move_menu[' 3. '] = "OutLftRvs"
                    move_menu[' 4. '] = "OutRgtFwd"
                    move_menu[' 5. '] = "OutRgtRvs"
                    move_menu[' 6. '] = "OutLftPrl"
                    move_menu[' 7. '] = "OutRgtPrl"
                    move_menu[' 8. '] = "RtnToOgn"

                    print(" * * * Supported Maneuvers * * * ")
                    print_menu( move_menu )
                    print( "---" )

                    selection = raw_input( "Please Select: " )

                    maneuver = None
                    if selection =='0':
                        msg_body[ "maneuver" ] = "StrFwd"
                    elif selection =='1':
                        msg_body[ "maneuver" ] = "StrRvs"
                    elif selection =='2':
                        msg_body[ "maneuver" ] = "OutLftFwd"
                    elif selection == '3':
                        msg_body[ "maneuver" ] = "OutLftRvs"
                    elif selection == '4':
                        msg_body[ "maneuver" ] = "OutRgtFwd"
                    elif selection == '5':
                        msg_body[ "maneuver" ] = "OutRgtRvs"
                    elif selection == '6':
                        msg_body[ "maneuver" ] = "OutLftPrl"
                    elif selection == '7':
                        msg_body[ "maneuver" ] = "OutRgtPrl"
                    elif selection == '8':
                        msg_body[ "maneuver" ] = "RtnToOgn"

                elif msg_group == 'deadmans_handle':
                    dmh_input = None
                    while dmh_input not in ("y", "n", "yes", "no"):
                        dmh_input = raw_input( "Valid DMH gesture (Enter yes or no): ")
                        if dmh_input == "yes" or dmh_input == "y":
                            msg_body[ "enable_vehicle_motion" ] = True
                        elif dmh_input == "no" or dmh_input == "n":
                            msg_body[ "enable_vehicle_motion" ] = False
                        else:
                        	print("Please enter yes or no.")

                    msg_body[ "dmh_gesture_progress" ] = random.randint( 95, 100 )
                    msg_body[ "dmh_horizontal_touch" ] = random.uniform( 0.0, 2048.0 )
                    msg_body[ "dmh_vertical_touch" ] = random.uniform( 0.0, 4096.0 )

                    msg_body[ "crc_value" ] = int( msg_body[ "dmh_gesture_progress" ] )
                    msg_body[ "crc_value" ] += int( msg_body[ "dmh_horizontal_touch" ] )
                    msg_body[ "crc_value" ] += int( msg_body[ "dmh_vertical_touch" ] )

                    # the below will cause memory overflows with certain challenges
                    # since there is no CRC parsing in the ASP Simulator, leave out
                    # for now until CRC parsing is built.
                    # msg_body[ "crc_value" ] += int( msg_body[ "response_to_challenge" ] )

                elif msg_group == 'cabin_commands':
                    engine_off_input = None
                    while engine_off_input not in ('t', 'f'):
                        engine_off_input = raw_input( "engine_off? (t for true/f for false): " )
                        if engine_off_input == 't':
                            msg_body[ "engine_off" ] = True
                        elif engine_off_input == 'f':
                            msg_body[ "engine_off" ] = False
                        else:
                            print ("Please enter 't' or 'f'.")
                    doors_locked_input = None
                    while doors_locked_input not in ('t', 'f'):
                        doors_locked_input = raw_input( "doors_locked? (t for true/f for false): " )
                        if doors_locked_input == 't':
                            msg_body[ "doors_locked" ] = True
                        elif doors_locked_input == 'f':
                            msg_body[ "doors_locked" ] = False
                        else:
                            print ("Please enter 't' or 'f'.")

                print( msg_header )
                print( msg_body )

                print( "Sending '%s' to TCM; awaiting receipt." % json_path )
                print( "---" )

                threading_condition.acquire( )

                hdr_queue.put( msg_header )
                bdy_queue.put( msg_body )
                # message_log.append( queue.get( ) )

                threading_condition.release( )

                # send_message( sock, msg_body, msg_header )
                # queue.clear( )

                json_path = ''
                msg_group = ''
                no_body = False


            except:

                # threading_condition.acquire( )
                # time.sleep( 1.25 )
                # if expecting_response:
                #     message_log.append( queue.get( ) )
                # expecting_response = True
                # time.sleep( 0.25 )

                # while queue.qsize( ) > 0:
                #     message_log.append( queue.get( ) )
                #     print("ahh!")

                # threading_condition.release( )

                print( "Total JSON messages received: %s" % ( len(message_log)-1 ) )
                # time.sleep( 0.25 )
                print( "---" )
                print( "Send a sample JSON message by selecting one of these options." )
                print( "---" )

                write_menu = { }
                write_menu[' 0. '] = "get_api_version"
                write_menu[' 1. '] = "send_pin"
                write_menu[' 2. '] = "mobile_init"
                write_menu[' 3. '] = "get_threat_data"
                write_menu[' 4. '] = "list_maneuvers"
                write_menu[' 5. '] = "maneuver_init"
                write_menu[' 6. '] = "deadmans_handle"
                write_menu[' 7. '] = "cancel_maneuver"
                write_menu[' 8. '] = "get_cabin_status"
                write_menu[' 9. '] = "cabin_commands"
                write_menu['10. '] = "cancel_drive_on"


                read_menu = { }
                read_menu['11. '] = "vehicle_status"
                read_menu['12. '] = "maneuver_status"


                print(" * * * Test Write Functions * * * ")
                print_menu( write_menu )
                print( "---" )
                print( " * * * Test Return Calls * * * ")
                print_menu( read_menu )
                print( "---" )
                print( " 99. Exit" )

                selection = raw_input( "Please Select: " )

                if selection =='0':
                    no_body = True
                    msg_group = "get_api_version"
                elif selection =='1':
                    json_path = "example_json/send_pin.json"
                    msg_group = "send_pin"
                elif selection =='2':
                    json_path = "example_json/mobile_init.json"
                    msg_group = "mobile_init"
                elif selection == '3':
                    no_body = True
                    msg_group = "get_threat_data"
                elif selection == '4':
                    no_body = True
                    msg_group = "list_maneuvers"
                elif selection == '5':
                    json_path = "example_json/maneuver_init.json"
                    msg_group = "maneuver_init"
                elif selection == '6':
                    json_path = "example_json/deadmans_handle.json"
                    # expecting_response = False
                    msg_group = "deadmans_handle"
                elif selection == '7':
                    no_body = True
                    msg_group = "cancel_maneuver"
                elif selection == '8':
                    no_body = True
                    msg_group = "get_cabin_status"
                elif selection == '9':
                    json_path = "example_json/cabin_commands.json"
                    msg_group = "cabin_commands"
                elif selection == '10':
                    no_body = True
                    msg_group = "cancel_drive_on"

                elif selection == '11':
                    no_body = True
                    msg_group = "vehicle_status"
                elif selection == '12':
                    no_body = True
                    msg_group = "maneuver_status"

                elif selection == '99':

                    threading_condition.acquire( )
                    bdy_queue.put( "disconnectMobile" )
                    hdr_queue.put( str(len(message_log)) )
                    threading_condition.release( )

                    print( "Disconnecting mobile from TCM." )
                    break

                else:
                    print( " * * * Unknown Option Selected!! * * * " )
                    # expecting_response = False

        return


thr1 = Comms( "socket_node" )
thr2 = Prompt( "menu_node" )

if (len(sys.argv) > 1 and sys.argv[1] == '-v'):
    VERBOSE = True

thr1.start( )
thr2.start( )

thr1.join( )
thr2.join( )

sys.exit( 0 )
