vcc -I../common -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -I../common -P../common ../common/svcommon.vc ../common/svcommon.dat

vcc -I../common -P../common shared.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
