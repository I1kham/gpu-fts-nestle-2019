var rhea = null;

function rheaDebug_isEnabled()		{ return rhea.Session_getOrDefault("debug", 0); }
function rheaDebug_enableDebug()	{ rhea.Session_setValue("debug", 1); }

function rheaBootstrap()
{
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step2(); };
	script.src = "js/rhea_final.min.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step2()
{
	rheaLoadScript("config/mainMenuIcons.js")
	.then ( function() { rhea = new Rhea(); return rhea.lang_loadLang(rhea.Session_getValue ("lang")); })
	.then( function() { return rhea.webSocket_connect(); })			
	.then( function() 
			{
				if (rheaDebug_isEnabled())
				{
					console.log ("loading debug script");
					return rheaLoadScript("js/rheaDebug.js").then( function() { rheaDebug_showWindow(); onRheaBootstrapFinished(); } );
				}
				else
					onRheaBootstrapFinished();
			} )
	
	.catch ( function(result) 
			{ 
					alert(result);
			});	

}
