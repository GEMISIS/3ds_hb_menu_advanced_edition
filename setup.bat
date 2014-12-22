@echo off
copy 3ds_hb_menu.3dsx F:\
echo transferred!
del F:\boot.3dsx
echo Old file deleted!
ren F:\3ds_hb_menu.3dsx boot.3dsx
echo Success!
pause