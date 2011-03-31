add_library( gme STATIC

    s_deltat.c
    s_logtbl.c
    s_opl.c
    s_opltbl.c
    blarrg_common.cpp
    blarrg_errors.cpp
    Resampler.cpp
    Rom_Data.cpp
    Downsampler.cpp
    Blip_Buffer.cpp

    Classic_Emu.cpp

    Data_Reader.cpp

    Dual_Resampler.cpp

    Effects_Buffer.cpp

    Fir_Resampler.cpp

    gme.cpp
    Gme_Loader.cpp

    Gme_File.cpp

    M3u_Playlist.cpp

    Multi_Buffer.cpp

    Music_Emu.cpp

    fmopl.cpp
    emu2413.cpp
    Ay_Apu.cpp

    Ay_Cpu.cpp

    Ay_Emu.cpp
    Ay_Core.cpp

    Gb_Apu.cpp

    Gb_Cpu.cpp

    Gb_Oscs.cpp

    Gbs_Emu.cpp

    Gbs_Core.cpp
    Gbs_Cpu.cpp
    Gym_Emu.cpp

    Hes_Apu.cpp

    Hes_Cpu.cpp
    Hes_Core.cpp
    Hes_Emu.cpp

    Hes_Apu_Adpcm.cpp
    Kss_Cpu.cpp

    Kss_Core.cpp
    Kss_Emu.cpp

    Kss_Scc_Apu.cpp

    Nes_Apu.cpp

    Nes_Cpu.cpp

    Nes_Fds_Apu.cpp
    Nes_Vrc7_Apu.cpp
    Nes_Fme7_Apu.cpp

    Nes_Namco_Apu.cpp

    Nes_Oscs.cpp

    Nes_Vrc6_Apu.cpp

    Nes_Impl.cpp
    Nsf_Core.cpp
    Nsf_Cpu.cpp
    Nsf_Emu.cpp

    Nsfe_Emu.cpp

    Sap_Apu.cpp
    Sap_Core.cpp

    Sap_Cpu.cpp

    Sap_Emu.cpp

    Sms_Apu.cpp

    Snes_Spc.cpp

    Sgc_Core.cpp
    Sgc_Cpu.cpp
    Sgc_Emu.cpp
    Sgc_Impl.cpp
    Sms_Fm_Apu.cpp
    Spc_Filter.cpp
    Track_Filter.cpp
    Upsampler.cpp
    Vgm_Core.cpp
    ym2413.c
    ymdeltat.c
    Z80_Cpu.cpp
    Spc_Cpu.cpp

    Spc_Dsp.cpp

    Spc_Emu.cpp

    Vgm_Emu.cpp

    Vgm_Emu_Impl.cpp

    Ym2413_Emu.cpp

    Ym2612_Emu.cpp
    Opl_Apu.cpp )
