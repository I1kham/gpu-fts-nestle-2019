var rhea = null;
var rheaPathPrefix = "";

function rheaDebug_isEnabled()		{ return Rhea_session_getOrDefault("debug", 0); }
function rheaDebug_enableDebug()	{ Rhea_session_setValue("debug", 1); }

function rheaBootstrap() { rheaBootstrapWithPathPrefix(""); }
function rheaBootstrapWithPathPrefix (pathPrefix)
{
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
			if (rheaDebug_isEnabled())
			{
				console.log ("loading debug script");
				return rheaLoadScript(rheaPathPrefix + "js/rheaDebug.js").then( function() { rheaDebug_showWindow(); onRheaBootstrapFinished(); } );
			}
			else
				onRheaBootstrapFinished();
		})
		.catch ( function(result) 
			{ 
				console.log ("rheaBootstrap FAILED:" +result);
				setTimeout (function() {window.location = "startup.html";}, 200);
			});	

}
