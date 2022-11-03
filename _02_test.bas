DIM J%
LET J%=2
DIM I%(1)
LET I%(1)= 12:LET I%(2)=45
REM PRINT I%
REM PRINT I%(1)
PRINT I%(2-J%): REM Fails Ok, expected positive index.

