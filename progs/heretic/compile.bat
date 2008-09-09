vcc -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -P../common ../common/linespec/classes.vc ../common/linespec.dat
vcc -P../common ../common/uibase/classes.vc ../common/uibase.dat

vcc -I../common -P../common game/classes.vc game.dat
vcc -I../common -P../common cgame/classes.vc cgame.dat
