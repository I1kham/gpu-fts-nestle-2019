@echo off

REM cancella dalla cartella superiore le cose inutili
del ..\\rhea_final*

REM concatena i vari js in un file unico di nome rhea_final.js
type promise.min.js store.legacy.min.js rheaUtils.js rhea.js rheaSession.js rheaSelection.js rheaEvent.js > rhea_final.js

REM li ottimizza rhea_final.js e lo copia nella cartella superiore
java -jar compiler.jar --js rhea_final.js --js_output_file ../rhea_final.min.js
del rhea_final.js

REM ottimizza rheaBootstrap-release.js e lo copia nella cartella superiore con nome  rheaBootstrap
java -jar compiler.jar --js rheaBootstrap-release.js --js_output_file ../rheaBootstrap.js

REM ottimizza gjs.js e lo copia nella cartella superiore con nome gjs-min.js
java -jar compiler.jar --js gjs.js --js_output_file ../gjs-min.js
