del /s /q project\obj\*.*
rmdir project\obj
del /s /q project\lst\*.*
rmdir project\lst
del /s /q project\FLASH_RUN\*.*
rmdir /s /q project\FLASH_RUN

del /s /q project\lpc17xx_chanfat_*.Bak
del /s /q project\lpc17xx_chanfat_*.dep 
del /s /q project\lpc17xx_chanfat_*.plg 

del .metadata\.log
del /s /q .metadata\.plugins\org.eclipse.cdt.core
rmdir /s /q .metadata\.plugins\org.eclipse.core.resources\.history\
del /s /q .metadata\.plugins\org.eclipse.epp.usagedata.recording\*.*
del /s /q .metadata\.plugins\org.eclipse.help.ui\scope_sets\*.hist
rmdir /s /q .metadata\.plugins\org.eclipse.ltk.core.refactoring\.refactorings\.workspace\
del /s /q .metadata\.plugins\org.eclipse.cdt.make.core\*.*
del /s /q .metadata\.plugins\org.eclipse.cdt.ui\*.*
del /s /q .metadata\.plugins\org.eclipse.search\*.*
del /s /q .metadata\.plugins\org.eclipse.core.resources\.projects\project\.indexes\*.*
rmdir /s /q .metadata\.plugins\org.eclipse.core.resources\.projects\project\.indexes

rem NONO rmdir /s /q .metadata\.plugins\org.eclipse.core.resources 
rem pause