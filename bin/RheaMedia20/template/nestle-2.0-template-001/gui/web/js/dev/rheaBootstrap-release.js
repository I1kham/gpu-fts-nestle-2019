var rhea = null;
var rheaPathPrefix = "";

function rheaBootstrap() { rheaBootstrapWithPathPrefix(""); }
function rheaBootstrapWithPathPrefix (pathPrefix)
{
	//nasconde il mouse (serve per le rhTT)
	var tmpCanvas = document.createElement("canvas");
    tmpCanvas.setAttribute("width",1);
    tmpCanvas.setAttribute("height",1);
    document.body.style.cursor = "url('"+tmpCanvas.toDataURL()+"'), pointer";
	
	rheaPathPrefix = pathPrefix;
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step2(); };
	script.src = rheaPathPrefix + "js/rhea_final.min.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step2()
{
	var scriptTagExists = document.getElementById("rheaBootstrapTag");
	if (null != scriptTagExists)
	{
		if (scriptTagExists.getAttribute("data-ses-clear") == "1")
			Rhea_clearSessionData();
	}
	rhea = new Rhea(); 
	
	rhea.webSocket_connect()
		.then( function() 
		{
			onRheaBootstrapFinished();
		})
		.catch ( function(result) 
			{ 
				console.log ("rheaBootstrap FAILED:" +result +"[" +window.location +"]");
				setTimeout (function() {window.location = "startup.html";}, 200);
				
			});	

}
