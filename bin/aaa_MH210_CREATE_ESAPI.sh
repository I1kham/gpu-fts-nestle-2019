#questo script genera il pacchetto GPU da caricare sulle schede SECO
#L'estensione del file è mh210, differente da mh6 usato per le versioni imx6
clear
DATA=`date '+%y%m%d'`
estensione="_commit.mh210"
filename="GPU_TS_v.2.4.5_$DATA$estensione"


FILE_GPU="./ROCKCHIP_RELEASE_GPU"
if [ ! -f "$FILE_GPU" ]; then
    echo "$FILE_GPU does not exist"
	cp ./aaa_MH210_CREATE_ESAPI.sh ./error_ROCKCHIP_RELEASE_GPU_does_not_exists
	exit 1
fi


#la roba che mi interessa la metto tutta in GPUPackage2019
rm -r ./GPUPackage2019
mkdir ./GPUPackage2019
mkdir ./GPUPackage2019/current
mkdir ./GPUPackage2019/current/lang
cp "$FILE_GPU" ./GPUPackage2019/GPUFusion
cp ./current/lang/*.* ./GPUPackage2019/current/lang
cp ../src/makeRheaServicePack.sh ./GPUPackage2019
rm ./varie/prog/lastUsedLang.txt
cp -r ./varie ./GPUPackage2019
chmod 777 ./GPUPackage2019/GPUFusion
chmod 777 ./GPUPackage2019/makeRheaServicePack.sh
chmod 777 ./GPUPackage2019/current/lang
rm ./GPUPackage2019/varie/prog/js/dev/compiler.jar

rm ./$filename
tar -czvf $filename ./GPUPackage2019
rm -r ./GPUPackage2019
