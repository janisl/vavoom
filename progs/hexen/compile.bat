vcc -I../common -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -I../common -P../common ../common/linespec/classes.vc ../common/linespec.dat
vcc -I../common -P../common ../common/server/classes.vc ../common/svcommon.dat

vcc -I../common -P../common hexndefs.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
