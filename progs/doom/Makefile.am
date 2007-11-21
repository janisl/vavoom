VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/doom1/progs
COMMON_PROGS_DIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat $(OUTDIR)/clprogs.dat

$(OUTDIR)/shared.dat: $(VCC) $(srcdir)/*.vc $(COMMON_PROGS_DIR)/engine.dat
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/doomdefs.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/game/*.vc $(srcdir)/game1/*.vc \
		$(OUTDIR)/shared.dat $(COMMON_PROGS_DIR)/linespec.dat
	$(VCC) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/svprogs.vc $@

$(OUTDIR)/clprogs.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/cgame/*.vc $(OUTDIR)/svprogs.dat $(COMMON_PROGS_DIR)/uibase.dat
	$(VCC) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/clprogs.vc $@

EXTRA_DIST = compile.bat \
	clprogs.vc \
	doomdefs.vc \
	svprogs.vc \
	cgame/classes.vc \
	cgame/ClientGame.vc \
	cgame/FinaleScreen.vc \
	cgame/IntermissionScreen.vc \
	cgame/MenuChoiceEpisode.vc \
	cgame/MenuScreenAdvancedVideoOptions.vc \
	cgame/MenuScreenControls.vc \
	cgame/MenuScreenControls2.vc \
	cgame/MenuScreenEpisode.vc \
	cgame/MenuScreenHelp.vc \
	cgame/MenuScreenJoinGame.vc \
	cgame/MenuScreenLoadGame.vc \
	cgame/MenuScreenMain.vc \
	cgame/MenuScreenMouseOptions.vc \
	cgame/MenuScreenMultiplayer.vc \
	cgame/MenuScreenNewNetGame.vc \
	cgame/MenuScreenOptions.vc \
	cgame/MenuScreenPlayerSetup.vc \
	cgame/MenuScreenSaveGame.vc \
	cgame/MenuScreenScreenResolution.vc \
	cgame/MenuScreenSinglePlayer.vc \
	cgame/MenuScreenSkill.vc \
	cgame/MenuScreenSList.vc \
	cgame/MenuScreenSoundOptions.vc \
	cgame/MenuScreenVideoOptions.vc \
	cgame/MenuSelector_Skull.vc \
	cgame/StatusBarScreen.vc \
	cgame/TitleScreen.vc \
	game/classes.vc \
	game/Actor.vc \
	game/BloodSplash.vc \
	game/BloodSplashBase.vc \
	game/BotPlayer.vc \
	game/DoomLevelInfo.vc \
	game/DoomWeapon.vc \
	game/DoomWorldInfo.vc \
	game/LavaSmoke.vc \
	game/LavaSplash.vc \
	game/MainGameInfo.vc \
	game/Player.vc \
	game/QuakeFocus.vc \
	game/SecurityCamera.vc \
	game/SlimeChunk.vc \
	game/SlimeSplash.vc \
	game/SludgeChunk.vc \
	game/SludgeSplash.vc \
	game/SoundSequence.vc \
	game/SoundSequenceSlot.vc \
	game/StaticLightSource.vc \
	game/StaticRGBLightSource.vc \
	game/WaterSplash.vc \
	game/WaterSplashBase.vc \
	game1/classes.vc \
	game1/Allmap.vc \
	game1/ArmorBonus.vc \
	game1/Backpack.vc \
	game1/BaronBall.vc \
	game1/BaronOfHell.vc \
	game1/Berserk.vc \
	game1/BFG9000.vc \
	game1/BFGBall.vc \
	game1/BFGExtra.vc \
	game1/BigTree.vc \
	game1/Blood.vc \
	game1/BloodyTwitch.vc \
	game1/BlueArmor.vc \
	game1/BlueCard.vc \
	game1/BlueSkull.vc \
	game1/BlueTorch.vc \
	game1/BlurSphere.vc \
	game1/BrainStem.vc \
	game1/BulletPuff.vc \
	game1/BurningBarrel.vc \
	game1/Cacodemon.vc \
	game1/CacodemonBall.vc \
	game1/Candelabra.vc \
	game1/Candlestick.vc \
	game1/Cell.vc \
	game1/CellPack.vc \
	game1/Chaingun.vc \
	game1/Chainsaw.vc \
	game1/Clip.vc \
	game1/ClipBox.vc \
	game1/ColonGibs.vc \
	game1/Column.vc \
	game1/Cyberdemon.vc \
	game1/DeadCacodemon.vc \
	game1/DeadDemon.vc \
	game1/DeadDoomImp.vc \
	game1/DeadLostSoul.vc \
	game1/DeadMarine.vc \
	game1/DeadShotgunGuy.vc \
	game1/DeadStick.vc \
	game1/DeadZombieMan.vc \
	game1/Demon.vc \
	game1/DoomImp.vc \
	game1/DoomImpBall.vc \
	game1/DoomKey.vc \
	game1/DoomPlayer.vc \
	game1/DoomUnusedStates.vc \
	game1/EvilEye.vc \
	game1/ExplosiveBarrel.vc \
	game1/Fist.vc \
	game1/FloatingSkull.vc \
	game1/GibbedMarine.vc \
	game1/GibbedMarineExtra.vc \
	game1/Gibs.vc \
	game1/GreenArmor.vc \
	game1/GreenTorch.vc \
	game1/Grenade.vc \
	game1/GrenadeLauncher.vc \
	game1/HangBNoBrain.vc \
	game1/HangNoGuts.vc \
	game1/HangTLookingDown.vc \
	game1/HangTLookingUp.vc \
	game1/HangTNoBrain.vc \
	game1/HangTSkull.vc \
	game1/HeadCandles.vc \
	game1/HeadOnAStick.vc \
	game1/HeadsOnAStick.vc \
	game1/HealthBonus.vc \
	game1/HeartColumn.vc \
	game1/Infrared.vc \
	game1/InvulnerabilitySphere.vc \
	game1/LiveStick.vc \
	game1/LostSoul.vc \
	game1/Meat2.vc \
	game1/Meat3.vc \
	game1/Meat4.vc \
	game1/Meat5.vc \
	game1/Medikit.vc \
	game1/NonsolidMeat2.vc \
	game1/NonsolidMeat3.vc \
	game1/NonsolidMeat4.vc \
	game1/NonsolidMeat5.vc \
	game1/NonsolidTwitch.vc \
	game1/Pistol.vc \
	game1/PlasmaBall.vc \
	game1/PlasmaRifle.vc \
	game1/RadSuit.vc \
	game1/RedCard.vc \
	game1/RedSkull.vc \
	game1/RedTorch.vc \
	game1/Rocket.vc \
	game1/RocketAmmo.vc \
	game1/RocketBox.vc \
	game1/RocketLauncher.vc \
	game1/Shell.vc \
	game1/ShellBox.vc \
	game1/ShortBlueTorch.vc \
	game1/ShortGreenColumn.vc \
	game1/ShortGreenTorch.vc \
	game1/ShortRedColumn.vc \
	game1/ShortRedTorch.vc \
	game1/Shotgun.vc \
	game1/ShotgunGuy.vc \
	game1/SkullColumn.vc \
	game1/SmallBloodPool.vc \
	game1/Soulsphere.vc \
	game1/Spectre.vc \
	game1/SpiderMastermind.vc \
	game1/Stalagtite.vc \
	game1/Stimpack.vc \
	game1/TallGreenColumn.vc \
	game1/TallRedColumn.vc \
	game1/TechLamp.vc \
	game1/TechLamp2.vc \
	game1/TechPillar.vc \
	game1/TeleportFog.vc \
	game1/TorchTree.vc \
	game1/YellowCard.vc \
	game1/YellowSkull.vc \
	game1/ZombieMan.vc \
	game2/classes.vc \
	game2/Arachnotron.vc \
	game2/ArachnotronPlasma.vc \
	game2/Archvile.vc \
	game2/ArchvileFire.vc \
	game2/BossBrain.vc \
	game2/BossEye.vc \
	game2/BossTarget.vc \
	game2/BrainState.vc \
	game2/ChaingunGuy.vc \
	game2/CommanderKeen.vc \
	game2/FatShot.vc \
	game2/Fatso.vc \
	game2/HellKnight.vc \
	game2/Megasphere.vc \
	game2/MegasphereHealth.vc \
	game2/PainElemental.vc \
	game2/Revenant.vc \
	game2/RevenantTracer.vc \
	game2/RevenantTracerSmoke.vc \
	game2/SpawnFire.vc \
	game2/SpawnShot.vc \
	game2/SuperShotgun.vc \
	game2/WolfensteinSS.vc

MOSTLYCLEANFILES = \
	$(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat $(OUTDIR)/clprogs.dat \
	$(OUTDIR)/shared.txt $(OUTDIR)/svprogs.txt $(OUTDIR)/clprogs.txt
