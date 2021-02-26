def print_menu( menu ):
    options = menu.keys( )
    options.sort( )
    for entry in options:
        print( "%s %s" % ( entry, menu[entry] ) )


# print( "Total JSON messages received: %s" % ( len(message_log)-1 ) )
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
write_menu[' 5. '] = "maneuver_init.json"
write_menu[' 6. '] = "deadmans_handle.json"
write_menu[' 7. '] = "cancel_maneuver"
write_menu[' 8. '] = "get_cabin_status"
write_menu[' 9. '] = "cabin_commands"

read_menu = { }
read_menu[' 11. '] = "vehicle_status"
read_menu[' 12. '] = "maneuver_status"


print(" * * * Test Write Functions * * * ")
print_menu( write_menu )
print( "---" )
print( " * * * Test Return Calls * * * ")
print_menu( read_menu )
print( "---" )
print( " 99. Exit" )

print( "---" )
print( "Select one of the above options in the mobile_placeholder terminal." )
