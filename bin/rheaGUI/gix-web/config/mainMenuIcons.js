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
								[0, 0],				//	GRINDER = 0 | 1			(0=grinder1, 1=grinder2)
								[0, 0],				//	CUP_SIZE = 0 | 1 | 2	(0=small, 1=medium, 2=large)
								[0, 0]				//	SHOT_TYPE = 0 | 1		(0=single shot, 1=extra shot)
							 ],
							 [
								[0, 0],
								[0, 0],
								[0, 0]
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
		pageMenuImg:	"Drink_blackCoffee.png",
		pageConfirmImg:	"Drink_blackCoffee_XL.png"
	},
	
	{
		selNum: 		2,
		size:			"010",
		pageMenuImg:	"Drink_blackCoffeeLong.png",
		pageConfirmImg:	"Drink_blackCoffeeLong_XL.png"
	},
	
	{
		selNum: 		0,
		size:			"110",		
		pageMenuImg:	"Drink_cappuccino.png",
		pageConfirmImg:	"Drink_cappuccino_XL.png",
		dblShot:		1,
		grinder2:		0,		
		linkedSelection: [[	[3, 4],
							[5, 6],
							[0, 0]],
							
						  [	[0, 0],
							[0, 0],
							[0, 0]]
						  ],

		
		defaultSelection: [0,0,0]
	},
	
	{
		selNum: 		0,
		size:			"011",		
		pageMenuImg:	"Drink_cappuccino.png",
		pageConfirmImg:	"Drink_cappuccino_XL.png",
		dblShot:		0,
		grinder2:		1,		
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
		size:			"101",		
		pageMenuImg:	"Drink_cappuccino.png",
		pageConfirmImg:	"Drink_cappuccino_XL.png",
		dblShot:		1,
		grinder2:		1,		
		linkedSelection: [[	[20, 4],
							[13, 8],
							[12, 3]],
							
						  [	[1, 8],
							[5, 17],
							[21, 16]]
						  ],

		
		defaultSelection: [0,0,0]
	},
	
	{ selNum: 6, size: "100", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 7, size: "100", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 8, size: "100", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 9, size: "100", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 10, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 11, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 12, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 13, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 14, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 15, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 16, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 17, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 18, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 19, size: "010", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},

	{ selNum: 20, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 21, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 22, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 23, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 24, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 25, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 26, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 27, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 28, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
	{ selNum: 29, size: "001", pageMenuImg:"Drink_chocolate.png", pageConfirmImg: "Drink_chocolate_XL.png"},
];


