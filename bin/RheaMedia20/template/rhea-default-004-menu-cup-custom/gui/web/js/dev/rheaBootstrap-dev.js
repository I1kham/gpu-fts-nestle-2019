/*	carica tutti i js necessari al corretto funzionamento.
	Quando ha finito, chiama la funzione onRheaBootstrapFinished(); che deve esistere nella pagina html
*/
var rhea = null;
var rheaPathPrefix = "";

function rheaBootstrap() { rheaBootstrapWithPathPrefix(""); }
function rheaBootstrapWithPathPrefix (pathPrefix)
{
	rheaPathPrefix = pathPrefix;
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step2(); };
	script.src = rheaPathPrefix + "js/dev/promise.min.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step2()
{
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step3(); };
	script.src = rheaPathPrefix + "js/dev/rheaUtils.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step3()
{
	rheaLoadScript(rheaPathPrefix + "js/dev/store.legacy.min.js")
		.then( function() { return rheaLoadScript(rheaPathPrefix + "js/dev/rhea.js"); })
		.then( function() { return rheaLoadScript(rheaPathPrefix + "js/dev/rheaSession.js"); })
		.then( function() { return rheaLoadScript(rheaPathPrefix + "js/dev/rheaSelection.js"); })
		.then( function() { return rheaLoadScript(rheaPathPrefix + "js/dev/rheaEvent.js"); })
		.then( function() { 
							var scriptTagExists = document.getElementById("rheaBootstrapTag");
							if (null != scriptTagExists)
							{
								if (scriptTagExists.getAttribute("data-ses-clear") == "1")
								{
									Rhea_clearSessionData();
								}
							}		
							
							rhea = new Rhea(); 
							return rhea.webSocket_connect();
						  })
		.then( function() 
			  	{
					onRheaBootstrapFinished();
				} )
		
		.catch ( function(result) 
				{ 
					console.log ("rheaBootstrap FAILED:" +result);
				});
}
