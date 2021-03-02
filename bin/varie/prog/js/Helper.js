function helper_intToFixedOnePointDecimal(v)
{
	if (v > 0)
	{
		if (v<10)
			v = "0." +v;
		else
		{
			v = v.toString();
			v = v.substr(0, v.length-1) +"." +v.substr(v.length-1);
		}
	}
	return v;
}

function helper_decodeImpulsi(v)
{
	//prende i 3 digiti meno significativi e mette un punto dopo il primo digit
	var s = v.toString();
	while (s.length < 3) s = "0" +s;
	
	var n = s.length;
	if (n > 3)
		s = s.substr(n-3, 3);
		
	return s.substr(0,1) +"." +s.substr(1,2);
}

//fn per formattare una stringa del tipo "ciao {0}, io sono {1}" in modo da sostituire i vari {0}..{n} con i parametri passati alla fn.
//Es: "ciao {0}, io sono {1}".translateLang("amico", "Mario") diventa "ciao amico, io sono Mario".
//Questa fb è usata principalmente per tradurre le string di linguaggio che hanno parametri.
String.prototype.translateLang = String.prototype.translateLang || function () 
{
    "use strict";
    var str = this.toString();
    if (arguments.length) 
	{
        var t = typeof arguments[0];
        var key;
        var args = ("string" === t || "number" === t) ? Array.prototype.slice.call(arguments) : arguments[0];
        for (key in args)
            str = str.replace(new RegExp("\\{" + key + "\\}", "gi"), args[key]);
    }
    return str;
};