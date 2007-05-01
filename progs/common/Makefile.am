VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/engine.dat $(OUTDIR)/linespec.dat

$(OUTDIR)/engine.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/engine/*.vc
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(OUTDIR) $(srcdir)/engine/classes.vc $@

$(OUTDIR)/linespec.dat: $(VCC) $(OUTDIR)/engine.dat $(srcdir)/linespec/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/linespec/classes.vc $@

EXTRA_DIST = macros.vc \
	engine/BasePlayer.vc \
	engine/Entity.vc \
	engine/classes.vc \
	engine/LevelInfo.vc \
	engine/Thinker.vc \
	engine/GameInfo.vc \
	engine/Object.vc \
	engine/GC.vc \
	engine/Level.vc \
	engine/ClientGameBase.vc \
	engine/WorldInfo.vc \
	engine/ModalWindow.vc \
	engine/RootWindow.vc \
	engine/Window.vc \
	engine/PlayerReplicationInfo.vc \
	linespec/classes.vc \
	linespec/SectorThinker.vc \
	linespec/SectorMover.vc \
	linespec/CeilingWaggle.vc \
	linespec/LineSpecialLevelInfo.vc \
	linespec/VerticalDoor.vc \
	linespec/FloorMover.vc \
	linespec/FloorWaggle.vc \
	linespec/CeilingMover.vc \
	linespec/StairStepMover.vc \
	linespec/Platform.vc \
	linespec/Pillar.vc \
	linespec/Elevator.vc \
	linespec/PolyobjThinker.vc \
	linespec/PolyobjRotator.vc \
	linespec/PolyobjMover.vc \
	linespec/PolyobjDoor.vc \
	linespec/FireFlicker.vc \
	linespec/Lighting.vc \
	linespec/GlowingLight.vc \
	linespec/LightFlash.vc \
	linespec/Strobe.vc \
	linespec/LightEffect.vc \
	linespec/PhasedLight.vc \
	linespec/Scroller.vc \
	linespec/TextureChangeDoor.vc \
	linespec/PlaneWatcher.vc \
	linespec/EntityEx.vc \
	linespec/Pusher.vc \
	linespec/LineSpecialGameInfo.vc
