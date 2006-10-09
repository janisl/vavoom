vcc -I../common -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -I../common -P../common ../common/linespec/classes.vc ../common/linespec.dat

vcc -I../common -P../common herdefs.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
