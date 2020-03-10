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