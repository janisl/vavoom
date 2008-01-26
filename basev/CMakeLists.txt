get_target_property(VLUMPY vlumpy LOCATION)
get_target_property(ACC acc LOCATION)

add_subdirectory(common)
add_subdirectory(doom)
add_subdirectory(doom1)
add_subdirectory(doom2)
add_subdirectory(heretic)
add_subdirectory(hexen)
add_subdirectory(plutonia)
add_subdirectory(strife)
add_subdirectory(tnt)

install(FILES games.txt DESTINATION ${DATADIR}/basev)
