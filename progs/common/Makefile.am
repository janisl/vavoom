VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/engine.dat $(OUTDIR)/linespec.dat $(OUTDIR)/uibase.dat

$(OUTDIR)/engine.dat: $(VCC) $(srcdir)/engine/*.vc
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(OUTDIR) $(srcdir)/engine/classes.vc $@

$(OUTDIR)/linespec.dat: $(VCC) $(OUTDIR)/engine.dat $(srcdir)/linespec/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/linespec/classes.vc $@

$(OUTDIR)/uibase.dat: $(VCC) $(OUTDIR)/engine.dat $(srcdir)/uibase/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/uibase/classes.vc $@

EXTRA_DIST = \
	engine/classes.vc \
	engine/ActorDisplayWindow.vc \
	engine/BasePlayer.vc \
	engine/ClientGameBase.vc \
	engine/Entity.vc \
	engine/GameInfo.vc \
	engine/Level.vc \
	engine/LevelInfo.vc \
	engine/Object.vc \
	engine/PlayerReplicationInfo.vc \
	engine/RootWidget.vc \
	engine/Thinker.vc \
	engine/Widget.vc \
	engine/WorldInfo.vc \
	linespec/classes.vc \
	linespec/ActorMover.vc \
	linespec/AimingCamera.vc \
	linespec/AmbientSound.vc \
	linespec/Ammo.vc \
	linespec/Armor.vc \
	linespec/BackpackItem.vc \
	linespec/BasicArmor.vc \
	linespec/BasicArmorBonus.vc \
	linespec/BasicArmorPickup.vc \
	linespec/CeilingMover.vc \
	linespec/CeilingWaggle.vc \
	linespec/Elevator.vc \
	linespec/EntityEx.vc \
	linespec/FakeInventory.vc \
	linespec/FireFlicker.vc \
	linespec/FloorMover.vc \
	linespec/FloorWaggle.vc \
	linespec/GlowingLight.vc \
	linespec/Health.vc \
	linespec/HealthPickup.vc \
	linespec/HexenArmor.vc \
	linespec/InterpolationPoint.vc \
	linespec/InterpolationSpecial.vc \
	linespec/Inventory.vc \
	linespec/ItemFog.vc \
	linespec/Key.vc \
	linespec/LightFlash.vc \
	linespec/Lighting.vc \
	linespec/LightEffect.vc \
	linespec/LineSpecialGameInfo.vc \
	linespec/LineSpecialLevelInfo.vc \
	linespec/LookAtCamera.vc \
	linespec/MapRevealer.vc \
	linespec/MapSpot.vc \
	linespec/MapSpotGravity.vc \
	linespec/PatrolPoint.vc \
	linespec/PatrolSpecial.vc \
	linespec/PathFollower.vc \
	linespec/PhasedLight.vc \
	linespec/PickupFlash.vc \
	linespec/Pillar.vc \
	linespec/PlaneWatcher.vc \
	linespec/Platform.vc \
	linespec/PlayerEx.vc \
	linespec/PointPuller.vc \
	linespec/PointPusher.vc \
	linespec/PolyobjDoor.vc \
	linespec/PolyobjMover.vc \
	linespec/PolyobjRotator.vc \
	linespec/PolyobjThinker.vc \
	linespec/PowerFlight.vc \
	linespec/PowerGhost.vc \
	linespec/PowerInvisibility.vc \
	linespec/PowerInvulnerable.vc \
	linespec/PowerIronFeet.vc \
	linespec/PowerLightAmp.vc \
	linespec/PowerMask.vc \
	linespec/PowerMinotaur.vc \
	linespec/PowerScanner.vc \
	linespec/PowerShadow.vc \
	linespec/PowerSpeed.vc \
	linespec/PowerStrength.vc \
	linespec/PowerTorch.vc \
	linespec/Powerup.vc \
	linespec/PowerupGiver.vc \
	linespec/PowerWeaponLevel2.vc \
	linespec/Pusher.vc \
	linespec/PuzzleItem.vc \
	linespec/QuakeFocus.vc \
	linespec/ScriptedEntity.vc \
	linespec/Scroller.vc \
	linespec/SectorMover.vc \
	linespec/SectorThinker.vc \
	linespec/SecurityCamera.vc \
	linespec/SoundSequence.vc \
	linespec/SoundSequenceSlot.vc \
	linespec/Spark.vc \
	linespec/StairStepMover.vc \
	linespec/Strobe.vc \
	linespec/SwitchableDecoration.vc \
	linespec/SwitchingDecoration.vc \
	linespec/TeleportDest.vc \
	linespec/TeleportDest2.vc \
	linespec/TeleportDest3.vc \
	linespec/TextureChangeDoor.vc \
	linespec/Unknown.vc \
	linespec/VerticalDoor.vc \
	linespec/WaterZone.vc \
	linespec/Weapon.vc \
	uibase/classes.vc \
	uibase/ClientGameShared.vc \
	uibase/HUDMessage.vc \
	uibase/HUDMessageFadeInOut.vc \
	uibase/HUDMessageFadeOut.vc \
	uibase/HUDMessageTypeOnFadeOut.vc \
	uibase/MenuBigTextButton.vc \
	uibase/MenuChoice.vc \
	uibase/MenuChoice_LoadSlot.vc \
	uibase/MenuChoice_OnOff.vc \
	uibase/MenuChoice_SaveSlot.vc \
	uibase/MenuChoiceEnum.vc \
	uibase/MenuChoiceSlider.vc \
	uibase/MenuChoiceSlot.vc \
	uibase/MenuControlKey.vc \
	uibase/MenuInputLine.vc \
	uibase/MenuModel.vc \
	uibase/MenuSaveSlot.vc \
	uibase/MenuScreen.vc \
	uibase/MenuScreenAdvancedVideoOptions.vc \
	uibase/MenuScreenControls.vc \
	uibase/MenuScreenJoinGame.vc \
	uibase/MenuScreenLoadGame.vc \
	uibase/MenuScreenMouseOptions.vc \
	uibase/MenuScreenSaveGame.vc \
	uibase/MenuScreenScreenResolution.vc \
	uibase/MenuScreenSList.vc \
	uibase/MenuScreenSoundOptions.vc \
	uibase/MenuScreenVideoOptions.vc \
	uibase/MenuSelector_Big.vc \
	uibase/MenuSelector_SmallLeft.vc \
	uibase/MenuSelector_SmallRight.vc \
	uibase/MenuSList.vc \
	uibase/MenuSmallTextButton.vc \
	uibase/MenuSpriteAnim.vc \
	uibase/MenuStaticAnim.vc \
	uibase/MenuStaticBitmap.vc \
	uibase/MenuTextButton.vc \
	uibase/MenuTitleText.vc \
	uibase/StatusBarShared.vc

MOSTLYCLEANFILES = \
	$(OUTDIR)/engine.dat $(OUTDIR)/engine.txt \
	$(OUTDIR)/linespec.dat $(OUTDIR)/linespec.txt \
	$(OUTDIR)/uibase.dat $(OUTDIR)/uibase.txt
