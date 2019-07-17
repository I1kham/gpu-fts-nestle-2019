/*
var rheaMainMenuIcons = [
	{
		selNum: 		[0 | 1..48],				//0=virtual selection, 1..48=selezione normale
		size:			"000"						//3 char per indicare se sono validi i size small, medium, large. Il carattere "0" indica che la relativa size è disabilitata
													//Nel caso di selNum>0, allora questa stringa contiene un solo "1" (es: "010" per medium size, "100" per small, "001" per large)
		
		//da qui in poi, sono presenti solo se selNum == 0
		
		dblShot:		[0 | 1]						//1 se l'opzione extra shot è abilitata
		grinder2:		[0 | 1]						//1 se l'opzione "scelta macina" è abilitata
		
		linkedSelection: 	[[						//linkedSelection[GRINDER][CUP_SIZE][SHOT_TYPE] = num selezione da erogare	
								[0, 0],		//grinder0 cup0 shot0, grinder0 cup0 shot1
								[0, 0],		//grinder0 cup1 shot0, grinder0 cup1 shot1
								[0, 0]		//grinder0 cup2 shot0, grinder0 cup2 shot1
							 ],
							 [
								[0, 0],		//grinder1 cup0 shot0, grinder1 cup0 shot1
								[0, 0],		//grinder1 cup1 shot0, grinder1 cup1 shot1
								[0, 0]		//grinder1 cup2 shot0, grinder1	 cup2 shot1
							 ]],

		
		defaultSelection: [grinder, cupsize, shottype]	//la selezione di default indica il record in linkedSelection da abilitare di default quando si entra per la prima volta in pagina
			 												//selezione
	}
];
*/



var rheaMainMenuIcons = [
	{
		selNum: 		1,
		size:			"100",
		pageMenuImg:	"Drink_americano.png",
		pageConfirmImg:	"Drink_americano.png"
	},
	
	{
		selNum: 		2,
		size:			"010",
		pageMenuImg:	"Drink_babyccino.png",
		pageConfirmImg:	"Drink_babyccino.png"
	},
	
	{
		selNum: 		3,
		size:			"001",
		pageMenuImg:	"Drink_blackTea.png",
		pageConfirmImg:	"Drink_blackTea.png"
	},

	{
		selNum: 		0,
		size:			"110",		
		pageMenuImg:	"Drink_cafeAuLait.png",
		pageConfirmImg:	"Drink_cafeAuLait.png",
		dblShot:		1,
		grinder2:		0,		
		linkedSelection: [[	[3, 4],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[5, 6],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		
		defaultSelection: [0,0,0]
	},
	
	{
		selNum: 		0,
		size:			"011",		
		pageMenuImg:	"Drink_cappuccino.png",
		pageConfirmImg:	"Drink_cappuccino.png",
		dblShot:		1,
		grinder2:		0,		
		linkedSelection: [[	[0, 0],
							[15, 16],
							[20, 21]],
							
						  [	[0, 0],
							[0, 0],
							[0, 0]]
						  ],

		
		defaultSelection: [0,1,0]
	},

	{
		selNum: 		0,
		size:			"111",		
		pageMenuImg:	"Drink_cortado.png",
		pageConfirmImg:	"Drink_cortado.png",
		dblShot:		1,
		grinder2:		0,		
		linkedSelection: [[	[20, 4],
							[13, 8],
							[12, 3]],
							
						  [	[0, 0],
							[0, 0],
							[0, 0]]
						  ],

		
		defaultSelection: [0,2,1]
	},
	
	{
		selNum: 		0,
		size:			"101",		
		pageMenuImg:	"Drink_darkChocolate.png",
		pageConfirmImg:	"Drink_darkChocolate.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[3, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[0, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[4, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[5, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[6, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		
		defaultSelection: [0,2,0]
	},
	
	{
		selNum: 		0,
		size:			"101",		
		pageMenuImg:	"Drink_doubleEspresso.png",
		pageConfirmImg:	"Drink_doubleEspresso.png",
		dblShot:		1,
		grinder2:		1,		
		linkedSelection: [[	[3, 4],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[5, 6],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[7, 8]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[9, 10],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[11, 12],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[13, 14]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		
		defaultSelection: [1,2,0]
	},
	
	{ selNum: 8, size: "100", pageMenuImg:"Drink_espresso.png", pageConfirmImg: "Drink_espresso.png"},
	{ selNum: 9, size: "100", pageMenuImg:"Drink_espressoLong.png", pageConfirmImg: "Drink_espressoLong.png"},
	{ selNum: 10, size: "010", pageMenuImg:"Drink_flatWhite.png", pageConfirmImg: "Drink_flatWhite.png"},
	{ selNum: 11, size: "010", pageMenuImg:"Drink_greenTea.png", pageConfirmImg: "Drink_greenTea.png"},
	{ selNum: 12, size: "010", pageMenuImg:"Drink_hotWater.png", pageConfirmImg: "Drink_hotWater.png"},
	{ selNum: 13, size: "010", pageMenuImg:"Drink_latteMacchiato.png", pageConfirmImg: "Drink_latteMacchiato.png"},
	{ selNum: 14, size: "010", pageMenuImg:"Drink_milkChocolate.png", pageConfirmImg: "Drink_milkChocolate.png"},
	{ selNum: 15, size: "010", pageMenuImg:"Drink_moccacino.png", pageConfirmImg: "Drink_moccacino.png"},
	{ selNum: 16, size: "010", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 17, size: "010", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 18, size: "010", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 19, size: "010", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},

	{ selNum: 20, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 21, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 22, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 23, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 24, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 25, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 26, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 27, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 28, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
	{ selNum: 29, size: "001", pageMenuImg:"Drink_ristretto.png", pageConfirmImg: "Drink_ristretto.png"},
];


