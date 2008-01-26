set(VLUMPY ${CMAKE_BINARY_DIR}/../utils/bin/vlumpy)
set(ACC ${CMAKE_BINARY_DIR}/../utils/bin/acc)

add_subdirectory(common)
add_subdirectory(doom)
add_subdirectory(doom1)
add_subdirectory(doom2)
add_subdirectory(heretic)
add_subdirectory(hexen)
add_subdirectory(plutonia)
add_subdirectory(strife)
add_subdirectory(tnt)

#nobase_dist_basev_DATA = games.txt
