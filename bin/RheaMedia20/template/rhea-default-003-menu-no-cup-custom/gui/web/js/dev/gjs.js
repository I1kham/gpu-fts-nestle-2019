/**************************************************************************
 * URL ENCODER
 **************************************************************************/
var GENCODER_entities =     ['!'   ,'*'   ,"'"   ,"("   ,")"   ,";"   ,":"   ,"@"   ,"&"   ,"="   ,"+"   ,"$"   ,","   ,"/"   ,"?"   ,"#"   ,"["   ,"]"   ," "];
var GENCODER_replacements = ['%21' ,'%2A' ,'%27' ,'%28' ,'%29' ,'%3B' ,'%3A' ,'%40' ,'%26' ,'%3D' ,'%2B' ,'%24' ,'%2C' ,'%2F' ,'%3F' ,'%23' ,'%5B' ,'%5D' ,'%20'];
function GENCODER_encode(urlIN)
{
	var url = String(urlIN);
	var n = url.length;
	var ent_len = GENCODER_entities.length;
	var ret = "";
	for (var i=0;i<n;i++)
	{
		var c = url.substr(i,1);
		var i2=0;
		for (i2=0;i2<ent_len;i2++)
		{
			if (GENCODER_entities[i2] == c)
			{
				ret += GENCODER_replacements[i2];
				i2=9999;
				break;
			}
		}
		if (i2 != 9999)
			ret +=c;
	}
	return ret;
}

function GENCODER_decode(urlIN)
{
	var url = String(urlIN);
	url = url.replace(/%25/g, "%");
	var n = url.length;
	var rep_len = GENCODER_replacements.length;
	var ret = "";
	for (var i=0;i<n;i++)
	{
		var c = url.substr(i,1);
		if (c!="%")
			ret += c;
		else
		{
			var toFind = ("%" + url.substr(i+1,2)).toUpperCase();
			var i2=0;
			for (i2=0;i2<rep_len;i2++)
			{
				if (GENCODER_replacements[i2] == toFind)
				{
					ret += GENCODER_entities[i2];
					i2=9999;
					i+=2
					break;;
				}
			}
			if (i2 != 9999)
				ret += "%";
		}
	}
	return ret;
}