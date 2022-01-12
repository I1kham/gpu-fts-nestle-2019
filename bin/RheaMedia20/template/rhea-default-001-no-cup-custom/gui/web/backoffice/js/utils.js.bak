function isATemporaryPath()
{
	//cerca di capire se il patth attuale contiente /temp/yyyymmddhhmmss/web
	var curPath = rheaGetAbsolutePhysicalPath();
	var toFind = "/temp/" +dataOra_getYYYYMMDD("");
	var i = curPath.indexOf(toFind);
	if (i>0)
	{
		i+=6;
		var yyyymmddhhmmss = curPath.substr(i,14);
		return yyyymmddhhmmss;
	}
	return "";
}
	
//**************************************************************
function dataOra_getYYYYMMDD(sep)
{
	var d = new Date();
	var mm = (1 + d.getMonth()).toString();
	if (mm.length<2) mm = "0"+mm;
	
	var gg = d.getDate().toString();
	if (gg.length<2) gg = "0"+gg;
	
	return d.getFullYear() +sep +mm +sep +gg;
}

//**************************************************************
function dataOra_getHHMMSS(sep)
{
	var d = new Date();
	var hh = d.getHours().toString();
	if (hh.length<2) hh = "0"+hh;
	
	var mm = d.getMinutes().toString();
	if (mm.length<2) mm = "0"+mm;

	var ss = d.getSeconds().toString();
	if (ss.length<2) ss = "0"+ss;

	return hh +sep +mm +sep +ss;
}

//**************************************************************
function getFilenameFromPath(str)
{
	return str.split('\\').pop().split('/').pop();
}

//**************************************************************
function getUrlVars() 
{
	var vars = {}; 
	window.location.search.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) { vars[key] = value; }); 
	return vars;
}

//**************************************************************
function pleaseWait (bShowWait)
{
	if (bShowWait)
	{
		$("#main-content-wait-msg").html("");
		$("#main-content-wait").show();
		//$("#page-content").hide();
	}
	else
	{
		$("#main-content-wait").hide();
		$("#page-content").show();
	}
}

function pleaseWait_addMessage (msg)
{
	var h = $("#main-content-wait-msg").html();
	$("#main-content-wait-msg").html(h +"<br>" +msg);
}

//**************************************************************
function showHideElem(elemID)
{
	var d = $("#"+elemID);
	if (d.css("display")=="none") d.show(); else d.hide();
}

//**************************************************************
function onlyInteger(ev)
{
	var keyCode = parseInt(ev.keyCode);
	if (keyCode<48 || keyCode>57)
	{
		var d = ev.srcElement;
		var v="";
		for (var i=0; i<d.value.length; i++)
		{
			var c = d.value.substr(i,1);
			if (c>="0" && c<="9")
				v+=c;
		}
		d.value = v;
	}
	return true;
}

//**************************************************************
function preventDefaults (e) { e.preventDefault(); e.stopPropagation(); }




function saveAs (rawDataArrayBuffer, absDstFilePathWithoutSlashFinale, fileName, callback_onEnd)
{
	rhea.filetransfer_startUpload (rawDataArrayBuffer, "saveas:" +absDstFilePathWithoutSlashFinale +"/" +fileName, 
												fileName,
												null,	//user value 	
												null, 	//callback_onStart
												null,	//callback_onProgress
												callback_onEnd);

}

/**************************************************************
 * uploader
 */
function setupUploader (elemID, callback_onEnd)
{
	var callback_onStart = priv_upload_onStart;
	var callback_onProgress = priv_upload_onProgress;

	var divElem = document.getElementById(elemID);
	var accept = "";
	
	if (divElem) {
		accept = divElem.dataset.accept? " accept='" + divElem.dataset.accept + "'" : "";
	}
	
	divElem.classList.add("inputfile_wrapper");
	html =   "<input type='file' name='" + elemID +"_file' id='" + elemID +"_file' class='inputfile'" + accept + ">"
			+"<label id='" +elemID +"_label' for='" +elemID +"_file'>" +divElem.getAttribute("data-message") +"</label>"
			+"<div id='" +elemID +"_wait' class='inputfile_wait'><img src='img/animationRound32x32.gif'></div>";
	divElem.innerHTML=html;

	
	
	var dropArea = document.getElementById(elemID); // +"_label");
	['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => { dropArea.addEventListener(eventName, preventDefaults, false); });
	['dragenter', 'dragover'].forEach(eventName => { dropArea.addEventListener(eventName, function(){ rheaAddClassToElem(dropArea, "highlight"); }, false); });
	['dragleave', 'drop'].forEach(eventName => { dropArea.addEventListener(eventName, function(){ rheaRemoveClassToElem(dropArea, "highlight"); }, false); });
	dropArea.addEventListener('drop', function handleDrop(e)
										{
										  let dt = e.dataTransfer
										  let files = dt.files
										  priv_upload_begin(elemID, files[0], callback_onStart, callback_onProgress, callback_onEnd);
										}
										, false);
	
	var fileElem = document.getElementById(elemID +"_file");
	fileElem.addEventListener('change', 
		function priv_upload_onFileUploadChange(ev)
		{
			ev = ev || window.event;
			var target = ev.target || ev.srcElement;
			priv_upload_begin(elemID, target.files[0], callback_onStart, callback_onProgress, callback_onEnd);
		},
		false);
}


function priv_upload_begin (elemID, theFile, callback_onStart, callback_onProgress, callback_onEnd)
{
	$("#" +elemID +"_wait").show();
	var reader = new FileReader();
	var rawData = new ArrayBuffer();            
	reader.loadend = function() {}
	reader.onload = function(e) {
				rawData = e.target.result;
				rhea.filetransfer_startUpload (rawData, "saveas:" +rheaGetAbsolutePhysicalPath() +"/../upload/" +theFile.name, 
												theFile.name,
												elemID, 
												callback_onStart, 
												callback_onProgress, 
												callback_onEnd);
			}
	reader.readAsArrayBuffer(theFile);
}

function priv_upload_onStart(fileName, userValue)
{
	console.log ("fileupload_started [" +userValue +"]");
}

function priv_upload_onProgress(fileName, userValue, byteTransferred, bytesTotal)
{
	if (byteTransferred >= bytesTotal)
		$("#" +userValue +"_wait").hide();
	console.log ("onEvent_fileupload_progress [" +userValue +"][" +byteTransferred +"]/[" +bytesTotal+"]");
}

/**************************************************************
 * checkbox
 */
function setupCheckboxes()
{
	$("div.checkboxFive").each (function()
	{
		var d = $(this);
		var inputID = d.attr("data-input-id");
		d.append ("<input type='checkbox' value='1' id='" +inputID +"'><label for='" +inputID +"'></label>");
	});
}						

function wrapHTMLElement(elem, type, classes) {
	var wrapper = document.createElement( type || 'div' );
	
	if( classes ) { wrapper.className = classes; }
	
	var clone = elem.cloneNode(true);
	elem.parentNode.insertBefore(wrapper, elem);

	removeHTMLElement(elem);

	wrapper.appendChild( clone );

	return wrapper;
}

function removeHTMLElement(elem) {
    elem.parentNode.removeChild(elem);
    return false;
}
