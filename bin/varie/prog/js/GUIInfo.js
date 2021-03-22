function GUIInfo()
{
	this.selInfo = [];
	for (var i=1; i<=64; i++)
	{
		this.selInfo[i] = {"short":"xxx", "name":""};
	}
}


/********************************************************
 * load
 */
GUIInfo.prototype.load = function ()
{
	//console.log ("GUIInfo::load()");
	rhea.filetransfer_startDownload ("guiinfo", this, GUIInfo_load_onStart, GUIInfo_load_onProgress, GUIInfo_load_onEnd);
}

function GUIInfo_load_onStart(userValue)			{ }
function GUIInfo_load_onProgress()					{ }
function GUIInfo_load_onEnd (theGUIInfo, reasonRefused, obj)
{
	if (reasonRefused != 0)
	{
		console.log ("error downloading guiinfo");
	}
	else
	{
		//console.log ("GUIInfo_load_onEnd: succes. File size[" +obj.fileSize +"]");
		var e = (utf8ArrayToStr(obj.fileBuffer)).split("|");
		for (var i=0; i<e.length;)
		{
			var iSel = (i/2) +1;
			theGUIInfo.selInfo[iSel].short = e[i++];
			theGUIInfo.selInfo[iSel].name = e[i++];
			
		}
	}
	
	//console.log (theGUIInfo.selInfo);
	onGUIInfoLoaded();
}
