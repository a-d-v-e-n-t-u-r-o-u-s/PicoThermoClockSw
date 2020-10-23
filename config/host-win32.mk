RM_DIR := rmdir /Q /S
RM := del /Q
CP := copy
MV := move
MKDIR := mkdir
FIND := find
SED := sed\bin\sed.exe
DELIM := \ 
DELIM := $(strip $(DELIM))
CMDQUIET := >nul 2>nul & verify>nul

