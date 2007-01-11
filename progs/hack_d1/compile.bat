vcc -I../common -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -I../common -P../common ../common/linespec/classes.vc ../common/linespec.dat

vcc -I../common -P../common -I../doom ../doom/doomdefs.vc shared.dat
vcc -I../common -P../common -I../doom svprogs.vc svprogs.dat
vcc -I../common -P../common -I../doom clprogs.vc clprogs.dat
