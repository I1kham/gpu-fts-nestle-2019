clear
DATA=`date '+%y%m%d'`
estensione="_commit.rheaRaspi"
filename="raspi_v.1.0.0_$DATA$estensione"


FILE_EXE="./RASPI_RELEASE_rasPISerial"
if [ ! -f "$FILE_EXE" ]; then
    echo "$FILE_EXE does not exist"
	cp ./aaa_RasPI_CREATE_package.sh ./error_EXE_DOES_NOT_EXISTS
	exit 1
fi


rm ./$filename
tar -czvf $filename $FILE_EXE ./REST ./gui_parts
