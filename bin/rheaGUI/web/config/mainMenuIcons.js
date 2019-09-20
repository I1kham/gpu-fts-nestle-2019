var rheaCleaningSelection = 48;
var rheaMainMenuIcons = [
	{
		//ristretto
		selNum: 		0,
		size:			"100",
		pageMenuImg:	"Drink_ristretto.png",
		pageConfirmImg:	"Drink_ristretto.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[1, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[0, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[2, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,0,0]			//grinder, cup-size, shot-type
	},

	{
		//espresso
		selNum: 		0,
		size:			"100",		
		pageMenuImg:	"Drink_espresso.png",
		pageConfirmImg:	"Drink_espresso.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[3, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[0, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[4, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,0,0]			//grinder, cup-size, shot-type
	},

	{
		//double Espresso
		selNum: 		0,
		size:			"100",		
		pageMenuImg:	"Drink_doubleEspresso.png",
		pageConfirmImg:	"Drink_doubleEspresso.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[5, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[0, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[6, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,0,0]			//grinder, cup-size, shot-type
	},

	{
		//Lungo
		selNum: 		0,
		size:			"010",		
		pageMenuImg:	"Drink_espressoLong.png",
		pageConfirmImg:	"Drink_espressoLong.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[7, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[8, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},
	
	{
		//cappuccino
		selNum: 		0,
		size:			"011",
		pageMenuImg:	"Drink_cappuccino.png",
		pageConfirmImg:	"Drink_cappuccino.png",
		dblShot:		1,
		grinder2:		1,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[9, 13],		//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[10, 14]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[11, 15],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[12, 16]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},	
	
	{
		//latte macchiato
		selNum: 		0,
		size:			"011",		
		pageMenuImg:	"Drink_latteMacchiato.png",
		pageConfirmImg:	"Drink_latteMacchiato.png",
		dblShot:		1,
		grinder2:		1,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[17, 21],		//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[18, 22]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[19, 23],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[20, 24]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},	

	{
		//Mocha
		selNum: 		0,
		size:			"011",		
		pageMenuImg:	"Drink_moccacino.png",
		pageConfirmImg:	"Drink_moccacino.png",
		dblShot:		1,
		grinder2:		0,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[25, 27],		//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[26, 28]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},
	
	{
		//Americano
		selNum: 		0,
		size:			"010",
		pageMenuImg:	"Drink_americano.png",
		pageConfirmImg:	"Drink_americano.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[29, 0],		//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[30, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},	
	
	{
		//Hot Chocolate
		selNum: 		0,
		size:			"011",
		pageMenuImg:	"Drink_darkChocolate.png",
		pageConfirmImg:	"Drink_darkChocolate.png",
		dblShot:		0,
		grinder2:		0,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[31, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[32, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},
	
	{
		//Flat white
		selNum: 		0,
		size:			"011",
		pageMenuImg:	"Drink_cafeAuLait.png",
		pageConfirmImg:	"Drink_cafeAuLait.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[33, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[34, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[35, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[36, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},
	
	{
		//Espresso macchiato
		selNum: 		0,
		size:			"010",
		pageMenuImg:	"Drink_cortado.png",
		pageConfirmImg:	"Drink_cortado.png",
		dblShot:		0,
		grinder2:		1,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[37, 0],			//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[0, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[38, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},	
	
	{
		//Hot Water
		selNum: 		0,
		size:			"011",
		pageMenuImg:	"Drink_hotWater.png",
		pageConfirmImg:	"Drink_hotWater.png",
		dblShot:		0,
		grinder2:		0,		
		linkedSelection: [[	[0, 0],			//grinder0 cup0 shot0, grinder0 cup0 shot1
							[43, 0],		//grinder0 cup1 shot0, grinder0 cup1 shot1	
							[44, 0]],		//grinder0 cup2 shot0, grinder0 cup2 shot1	
							
						  [	[0, 0],			//grinder1 cup0 shot0, grinder1 cup0 shot1
							[0, 0],			//grinder1 cup1 shot0, grinder1 cup1 shot1
							[0, 0]]			//grinder1 cup2 shot0, grinder1 cup2 shot1
						  ],

		defaultSelection: [0,1,0]			//grinder, cup-size, shot-type
	},	
];


