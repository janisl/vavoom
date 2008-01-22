find_package(ZLIB REQUIRED)

add_subdirectory(acc)
add_subdirectory(fixmd2)
add_subdirectory(glbsp)
add_subdirectory(glvis)
add_subdirectory(vcc)
add_subdirectory(vlaunch)
add_subdirectory(vlumpy)

# EXTRA_DIST = \
# 	editing/acs/strfhelp.acs \
# 	editing/acs/stspec.acs \
# 	editing/acs/vcommon.acs \
# 	editing/acs/vdefs.acs \
# 	editing/acs/vspecial.acs \
# 	editing/acs/vwvars.acs \
# 	editing/dbuilder/Vavoom_Doom.cfg \
# 	editing/dbuilder/Vavoom_Doom2.cfg \
# 	editing/dbuilder/Vavoom_Heretic.cfg \
# 	editing/dbuilder/Vavoom_Hexen.cfg \
# 	editing/dbuilder/Vavoom_Strife.cfg \
# 	editing/wauthor/vdoom.wcf \
# 	editing/wauthor/vdoom2.wcf \
# 	editing/wauthor/vheretic.wcf \
# 	editing/wauthor/vhexen.wcf \
# 	editing/wauthor/vstrife.wcf
