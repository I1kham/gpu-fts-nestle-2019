/*	carica tutti i js necessari al corretto funzionamento.
	Quando ha finito, chiama la funzione onRheaBootstrapFinished(); che deve esistere nella pagina html
*/
var rhea = null;
function rheaBootstrap()
{
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step2(); };
	script.src = "js/promise.min.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step2()
{
	var script = document.createElement('script');
	script.onload = function () { rheaBootstrap_step3(); };
	script.src = "js/rheaUtils.js";
	document.head.appendChild(script);
}

function rheaBootstrap_step3()
{
	rheaLoadScript("js/store.legacy.min.js")
		.then( function() { return rheaLoadScript("config/mainMenuIcons.js"); })
		.then( function() { return rheaLoadScript("js/rhea.js"); })
		.then( function() { return rheaLoadScript("js/rheaSession.js"); })
		.then( function() { return rheaLoadScript("js/rheaSelection.js"); })
		.then( function() { return rheaLoadScript("js/rheaLang.js"); })
		.then( function() { return rheaLoadScript("js/rheaEvent.js"); })
		.then( function() { return rheaLoadScript("js/rheaMainMenuIcons.js"); })
		.then( function() { rhea = new Rhea(); return rhea.lang_loadLang(rhea.Session_getValue ("lang")); })
		.then( function() { return rhea.webSocket_connect(); })			
		.then( function() { onRheaBootstrapFinished(); } )
		.catch ( function(result) 
				{ 
						alert(result);
				});
}
