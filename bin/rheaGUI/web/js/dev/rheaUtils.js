function rheaLog(s)
{
	console.log (s);
}

/**************************************************
 * RheaAjaxAnswer
 *
 * oggetto di comodo usato internamente
 */
function RheaAjaxAnswer (requestID)
{
	this.requestID = requestID;
	this.rcv = null;
}


/**************************************************
 * utf8ArrayToStr
 *
 * converte un ArrayBuffer in una stringa
 */
var utf8ArrayToStr = (function () 
{
    var charCache = new Array(128);  // Preallocate the cache for the common single byte chars
    var charFromCodePt = String.fromCodePoint || String.fromCharCode;
    var result = [];

    return function (array) {
        var codePt, byte1;
        var buffLen = array.length;

        result.length = 0;

        for (var i = 0; i < buffLen;) {
            byte1 = array[i++];

            if (byte1 <= 0x7F) {
                codePt = byte1;
            } else if (byte1 <= 0xDF) {
                codePt = ((byte1 & 0x1F) << 6) | (array[i++] & 0x3F);
            } else if (byte1 <= 0xEF) {
                codePt = ((byte1 & 0x0F) << 12) | ((array[i++] & 0x3F) << 6) | (array[i++] & 0x3F);
            } else if (String.fromCodePoint) {
                codePt = ((byte1 & 0x07) << 18) | ((array[i++] & 0x3F) << 12) | ((array[i++] & 0x3F) << 6) | (array[i++] & 0x3F);
            } else {
                codePt = 63;    // Cannot convert four byte code points, so use "?" instead
                i += 3;
            }

            result.push(charCache[codePt] || (charCache[codePt] = charFromCodePt(codePt)));
        }

        return result.join('');
    };
})();




/****************************************************************
 * rheaLoadScript
 *
 *	ritorna una promise
 */
function rheaLoadScript (url)
{
	return new Promise( function(resolve, reject) 
	{
		//console.log ("rheaLoadScript: " +url);
		var script = document.createElement( "script" );
		script.src = url;
		script.onload = resolve;
		script.onerror = reject;
		document.head.appendChild( script );
	});
}

/****************************************************************
 * rheaGetElemByID
 *
 */
function rheaGetElemByID (divID)
{
	return document.getElementById(divID);
}

/****************************************************************
 * rheaSetDivHTMLByName
 *
 */
function rheaSetDivHTMLByName (divID, html)
{
	rheaGetElemByID(divID).innerHTML = html;
}

/****************************************************************
 * rheaGetElemWidth
 *
 *	[elem] deve essere ottenuto da rheaGetElemByID()
 */
function _rheaGetElemProp (p)			{ if (p===undefined) return 0; if (p=="") return 0; return parseInt(p); }
function rheaGetElemWidth (elem)		{ return _rheaGetElemProp(elem.offsetWidth); }
function rheaGetElemHeight (elem)		{ return _rheaGetElemProp(elem.offsetHeight); }
function rheaGetElemLeft (elem)			{ return _rheaGetElemProp(elem.style.left); }
function rheaGetElemTop (elem)			{ return _rheaGetElemProp(elem.style.top); }

function rheaSetElemHTML (elem, html)	{ elem.innerHTML = html; }
function rheaSetElemWidth (elem, pixel)	{ elem.style.width= pixel +"px"; }
function rheaSetElemHeight (elem, pixel){ elem.style.height= pixel +"px"; }
function rheaSetElemLeft (elem, pixel)	{ elem.style.left= pixel +"px"; }
function rheaSetElemTop (elem, pixel)	{ elem.style.top= pixel +"px"; }
function rheaSetDisplayMode (elem, mode) { elem.style.display=mode;}

function rheaSetElemHREF (elem, url)	{ elem.setAttribute("href",url); }

function rheaSetElemBackgroundImage (elem, url) { elem.style.backgroundImage="url(" +url +")"; }
function rheaSetElemBackgroundColor (elem, strColor) { elem.style.backgroundColor=strColor; }
function rheaSetElemTextColor (elem, strColor) { elem.style.color=strColor; }

function rheaAddClassToElem (elem, cssClass)	{ elem.classList.add(cssClass); }
function rheaRemoveClassToElem (elem, cssClass)	{ elem.classList.remove(cssClass); }

function rheaHideElem (elem)	{ rheaSetDisplayMode(elem, "none");}
function rheaShowElem (elem)	{ rheaSetDisplayMode(elem, "block");}



/****************************************************************
 * rheaEasingFn
 *
 */
function rheaEase_linear(t01)		{ return t01; };
function rheaEase_outCubic(t01)		{ return (--t01)*t01*t01+1; }

/****************************************************************
 * rheaSmoothScrollElemLeft
 *
 *	[elem] deve essere ottenuto da rheaGetElemByID()
 *	Scrolla la posizione style.left dal valore attuale al valore [to] nel tempo [durationMSec]
 */
function rheaSmoothScrollElemLeft (elem, to, durationMSec) 
{
    var start = rheaGetElemLeft(elem);
	var change = to - start;
	var currentTimeMSec = 0;
	var stepMSec = 20;
        
    var animateScroll = function()
	{
        currentTimeMSec += stepMSec;
        var t = currentTimeMSec / durationMSec;
		
		if (t >= 1)
		{
			clearInterval(hInterval);
			t = 1;
			rheaSetElemLeft(elem, start+change);
		}
		else
		{
			t = rheaEase_outCubic(t);
			val = start + change*t;
			rheaSetElemLeft(elem,val);
		}
    };
	
	var hInterval = setInterval(animateScroll, stepMSec);
	return hInterval;
}

/****************************************************************
 * rheaGetURLParamOrDefault
 *
 *	cerca nella queryString il parametro [paramName] e ne ritorna il valore
 *	Se non lo trova, ritorna [defValue]
 */
function rheaGetURLParamOrDefault(paramName, defValue)
{
    var result = null;
    var items = location.search.substr(1).split("&");
    for (var i = 0; i < items.length; i++)
	{
        var tmp = items[i].split("=");
        if (tmp[0] === paramName) 
			return decodeURIComponent(tmp[1]);
    }
    return defValue;
}

/****************************************************************
 * rheaSetDivAsScrollabelByGesture
 *
 *	il div diventa scrollabile su e giu usando i gesti di trascinamento
 */
function RheaScrollDivByGestureInfo(theDiv) { this.theDiv = theDiv; this.mouse_pressed =0; this.mouse_y = 0; this.totalH = rheaGetElemHeight(theDiv); }
function rheaSetDivAsScrollabelByGesture (divID, maxScrollH)
{
	if (maxScrollH <= 0)
		return;
	console.log ("maxScrollH="+maxScrollH);
		
		
	var theDiv = rheaGetElemByID(divID);
	var info = new RheaScrollDivByGestureInfo(theDiv);
	
	theDiv.addEventListener('mousedown', function (ev)
	{
		info.mouse_pressed = 1;
		info.mouse_y = ev.clientY;
		//console.log ("===================== down y=" +info.mouse_y);
	}, true);


	theDiv.addEventListener('mouseup', function (ev) 
	{
		//console.log ("===================== up");
		info.mouse_pressed = 0;
	}, true);

	theDiv.addEventListener('mousemove', function (ev) 
	{
		if (!info.mouse_pressed)
			return;
			
		var y = ev.clientY;
		var offset = y - info.mouse_y;
		info.mouse_y = y;
		//console.log ("y=" +y +", offset:" +offset);
		
		var top = rheaGetElemTop(info.theDiv);
		//console.log ("current top:" +top);
		top += offset;
		if (top >= 0)
			top = 0;
		if (top < -maxScrollH)
			top = -maxScrollH;
		rheaSetElemTop(info.theDiv, top);
		//console.log ("new top:" +top);
		
	}, true);
}