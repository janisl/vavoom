vcc -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -P../common ../common/linespec/classes.vc ../common/linespec.dat
vcc -P../common ../common/uibase/classes.vc ../common/uibase.dat

cd..
cd common
copy engine.dat D:\korax\vavoom\source\svn\basev\common\progs\
copy linespec.dat D:\korax\vavoom\source\svn\basev\common\progs\
cd..
cd..
cd basev
cd common

vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\vavoom\basev\common\"

cd..
cd..
cd progs
cd doom

vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat

copy svprogs.dat D:\korax\vavoom\source\svn\basev\doom1\progs\
copy clprogs.dat D:\korax\vavoom\source\svn\basev\doom1\progs\

cd..
cd..
cd basev
cd doom

vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\vavoom\basev\doom\"

cd..
cd doom1

vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\vavoom\basev\doom1\"