/*	carica tutti i js necessari al corretto funzionamento.
	Quando ha finito, chiama la funzione onRheaBootstrapFinished(); che deve esistere nella pagina html
*/
var rhea = null;

function rheaDebug_isEnabled()		{ return rhea.Session_getOrDefault("debug", 0); }
function rheaDebug_enableDebug()	{ rhea.Session_setValue("debug", 1); }

function rheaBootstrap()
{
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step2(); };
	script.src = "js/dev/promise.min.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step2()
{
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step3(); };
	script.src = "js/dev/rheaUtils.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step3()
{
	rheaLoadScript("js/dev/store.legacy.min.js")
		.then( function() { return rheaLoadScript("config/mainMenuIcons.js"); })
		.then( function() { return rheaLoadScript("js/dev/rhea.js"); })
		.then( function() { return rheaLoadScript("js/dev/rheaSession.js"); })
		.then( function() { return rheaLoadScript("js/dev/rheaSelection.js"); })
		.then( function() { return rheaLoadScript("js/dev/rheaLang.js"); })
		.then( function() { return rheaLoadScript("js/dev/rheaEvent.js"); })
		.then( function() { return rheaLoadScript("js/dev/rheaMainMenuIcons.js"); })
		.then( function() { rhea = new Rhea(); return rhea.lang_loadLang(rhea.Session_getValue ("lang")); })
		.then( function() { return rhea.webSocket_connect(); })			
		.then( function() 
			  	{
					if (rheaDebug_isEnabled())
					{
						console.log ("loading debug script");
						return rheaLoadScript("js/dev/rheaDebug.js").then( function() { rheaDebug_showWindow(); onRheaBootstrapFinished(); } );
					}
					else
						onRheaBootstrapFinished();
				} )
		
		.catch ( function(result) 
				{ 
					window.location = window.location;
				});
}
