import qrcode

def getMAC(interface='wlan0'):
	# Return the MAC address of the specified interface
	try:
    		str = open('/sys/class/net/%s/address' %interface).read()
  	except:
    		str = "00:00:00:00:00:00"
  	return str[0:17]



#recupero il mac address ma gli tolgo i ":"
wlan0IP="10.3.141.1"
macAddress = getMAC('wlan0')
cleanMACAddress = ""
for i in range(0, len(macAddress)):
	if (macAddress[i] != ':'):
		cleanMACAddress += macAddress[i]


#nome dell'hotspot e password
hotspotName = "rhea-" +cleanMACAddress
hotspotPassw = "fertysnd7789"
saveToFolder = "/home/pi/rhea/gpu-fts-nestle-2019/bin"

#creo il QR code per le macchine TP e lo salvo in una png
qrCodeForTPMachine = hotspotName + "|||" +wlan0IP +"/rhea/GUITP/startup.html"
qr = qrcode.QRCode(version=1, error_correction=qrcode.constants.ERROR_CORRECT_L, box_size=10, border=4)
qr.add_data(qrCodeForTPMachine)
qr.make(fit=True)
img = qr.make_image(fill_color="black", back_color="white")
img.save(saveToFolder +"/qrcodeTP.png")
#print "QRCODE:" +qrCodeForTPMachine


#creo il file di configurazione per dare un nome e una pwd all'hotspot wifi
hotspotConfig = """driver=nl80211
ctrl_interface=/var/run/hostapd
ctrl_interface_group=0
auth_algs=1
wpa_key_mgmt=WPA-PSK
beacon_int=100
ssid="""
hotspotConfig += hotspotName
hotspotConfig += """
channel=1
hw_mode=g
ieee80211n=0
wpa_passphrase="""
hotspotConfig += hotspotPassw
hotspotConfig += """
interface=wlan0
wpa=2
wpa_pairwise=CCMP
country_code=AF
ignore_broadcast_ssid=0"""

f = open ("/etc/hostapd/hostapd.conf", "w")
f.write (hotspotConfig)
f.close()

#salvo un file contenente il nome dell'hotspot in modo che il prg rasPI conosca il nome dell'hotspot e lo possa comunicare
#alla GPU
f = open (saveToFolder +"/hotspotname.txt", "w")
f.write (hotspotName)
f.close()

#salvo un file con l'IP in modo che il prg rasPI lo possa conoscere dato che parte prima che i servizi di rete siano stati caricati e non puo' interrogare il sistema per conoscere l'IP
f = open (saveToFolder+"/ip.txt", "w")
f.write (wlan0IP)
f.close()

