@echo off

REM cancella dalla cartella superiore le cose inutili
del ..\\rhea_final*
del ..\\rheaBootstrap.js

copy rheaBootstrap-dev.js ..\\rheaBootstrap.js