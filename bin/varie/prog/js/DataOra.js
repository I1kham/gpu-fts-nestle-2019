/***************************************************************************
 *
 * fn che si occupano di gestire la data/ora visualizzata nell'angolo in alto a dx
 *
 */
var dataora_mm = 1;
var dataora_dd = 1;
var dataora_yy = 2018;
var dataora_hh = 0;
var dataora_min = 0;
var dataora_sec = 0;
var dataora_interval = null;

function dataOra_setup ()
{
	if (null != dataora_interval)
	{
		clearInterval(dataora_interval);
		dataora_interval = null;
	}
		
	//console.log ("dataOra_setup::ajax::getTime");
	rhea.ajax ("getTime", "").then( function(result)
	{
		var data = JSON.parse(result);
		dataora_hh = data.h;
		dataora_min = data.m;
		dataora_sec = data.s;
	
		//console.log ("dataOra_setup::ajax::getDate");	
		rhea.ajax ("getDate", "").then( function(result)
		{
			var data = JSON.parse(result);
			dataora_dd = data.d;
			dataora_mm = data.m;
			dataora_yy = data.y;
			dataOra_run();
		})
		.catch( function(result)
		{
			//console.log ("dataOra_setup::ajax::getDate::err");
			setTimeout ( function() {dataOra_setup();}, 200);
		});	
	})
	.catch( function(result)
	{
		console.log ("dataOra_setup::ajax::getTime::err[" +result +"]");
		setTimeout ( function() {dataOra_setup();}, 200);
	});
}

//******* ritorna una stringa con la data attualmente visualizzata
function dataOra_getYYYYMMDD()
{
	var mm = dataora_mm.toString();
	if (mm.length<2) mm = "0"+mm;
	
	var gg = dataora_dd.toString();
	if (gg.length<2) gg = "0"+gg;
	return dataora_yy +"/" +mm +"/" +gg;
}

//******* ritorna una stringa con l'ora attualmente visualizzata
function dataOra_getHHMM(sep)
{
	var hh = dataora_hh.toString();
	if (hh.length<2) hh = "0"+hh;
	
	var mm = dataora_min.toString();
	if (mm.length<2) mm = "0"+mm;
	return hh +sep +mm;
}

//******************************************
function dataOra_run ()
{
	if (null != dataora_interval)
		clearInterval(dataora_interval);

	rheaSetDivHTMLByName("divHeaderR_data", dataOra_getYYYYMMDD());
	
	dataora_interval = setInterval (function() 
					{ 
						dataora_sec++;
						if (dataora_sec >= 60)
						{
							dataora_sec = 0;
							dataora_min++;
							if (dataora_min >= 60)
							{
								dataora_min = 0;
								if (dataora_hh==23)
								{
									dataOra_setup();
									return;
								}
								dataora_hh++;
							}
						}
						
						var hhmm = ""
						if (dataora_sec % 2 == 0)
							hhmm = dataOra_getHHMM("&nbsp;");
						else
						hhmm = dataOra_getHHMM(":");
						rheaSetDivHTMLByName("divHeaderR_ora", hhmm);
					}, 
					1000);
}
