vcc -I../common -P../common ../common/common.vc ../common/common.dat
vcc -I../common -P../common ../common/clcommon.vc ../common/clcommon.dat
vcc -I../common -P../common ../common/svcommon.vc ../common/svcommon.dat

vcc -I../common -P../common hexndefs.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
