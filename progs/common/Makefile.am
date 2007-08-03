VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/engine.dat $(OUTDIR)/linespec.dat

$(OUTDIR)/engine.dat: $(VCC) $(srcdir)/engine/*.vc
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(OUTDIR) $(srcdir)/engine/classes.vc $@

$(OUTDIR)/linespec.dat: $(VCC) $(OUTDIR)/engine.dat $(srcdir)/linespec/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/linespec/classes.vc $@

EXTRA_DIST = \
	engine/classes.vc \
	engine/ActorDisplayWindow.vc \
	engine/BasePlayer.vc \
	engine/ClientGameBase.vc \
	engine/Entity.vc \
	engine/GameInfo.vc \
	engine/GC.vc \
	engine/Level.vc \
	engine/LevelInfo.vc \
	engine/ModalWindow.vc \
	engine/Object.vc \
	engine/PlayerReplicationInfo.vc \
	engine/RootWindow.vc \
	engine/Thinker.vc \
	engine/Window.vc \
	engine/WorldInfo.vc \
	linespec/Ammo.vc \
	linespec/BackpackItem.vc \
	linespec/classes.vc \
	linespec/CeilingMover.vc \
	linespec/CeilingWaggle.vc \
	linespec/Elevator.vc \
	linespec/EntityEx.vc \
	linespec/FireFlicker.vc \
	linespec/FloorMover.vc \
	linespec/FloorWaggle.vc \
	linespec/GlowingLight.vc \
	linespec/Inventory.vc \
	linespec/ItemFog.vc \
	linespec/LightFlash.vc \
	linespec/Lighting.vc \
	linespec/LightEffect.vc \
	linespec/LineSpecialGameInfo.vc \
	linespec/LineSpecialLevelInfo.vc \
	linespec/PhasedLight.vc \
	linespec/PickupFlash.vc \
	linespec/Pillar.vc \
	linespec/PlaneWatcher.vc \
	linespec/Platform.vc \
	linespec/PlayerEx.vc \
	linespec/PolyobjDoor.vc \
	linespec/PolyobjMover.vc \
	linespec/PolyobjRotator.vc \
	linespec/PolyobjThinker.vc \
	linespec/Pusher.vc \
	linespec/PuzzleItem.vc \
	linespec/ScriptedEntity.vc \
	linespec/Scroller.vc \
	linespec/SectorMover.vc \
	linespec/SectorThinker.vc \
	linespec/StairStepMover.vc \
	linespec/Strobe.vc \
	linespec/TextureChangeDoor.vc \
	linespec/VerticalDoor.vc \
	linespec/Weapon.vc

MOSTLYCLEANFILES = \
	$(OUTDIR)/engine.dat $(OUTDIR)/engine.txt \
	$(OUTDIR)/linespec.dat $(OUTDIR)/linespec.txt
