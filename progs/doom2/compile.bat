vcc -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -P../common ../common/linespec/classes.vc ../common/linespec.dat
vcc -P../common ../common/uibase/classes.vc ../common/uibase.dat

vcc -I../common -P../common shared.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
