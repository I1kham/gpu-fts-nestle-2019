var MMI_GRINDER_0	= 0;
var MMI_GRINDER_1	= 1;

var MMI_CUP_SIZE_SMALL = 0;
var MMI_CUP_SIZE_MEDIUM = 1;
var MMI_CUP_SIZE_LARGE = 2;

var MMI_SHOT_TYPE_SINGLE = 0;
var MMI_SHOT_TYPE_DBL = 1;

/*********************************************************
 * MMI_getCount
 *
 *	ritorna il numero di icone da visualizzare nel main menu
 */
function MMI_getCount ()														{ return rheaMainMenuIcons.length; }

/*********************************************************
 * MMI_getDisplayName
 *
 *	ritorna la stringa da utilizzare per visualizzare il nome dell'icona.
 *	La stringa è già tradotta nel linguaggio corrente
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getDisplayName (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return rheaLang_mainMenuIconName[iIcon];
}

/*********************************************************
 * MMI_getPrice
 *
 *	ritorna una stringa con il prezzo già formattato con il separatore decimale al posto giusto
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getPrice (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum == 0)
	{
		//Questa è una "virtual selection". Ritorno il prezzo della selezione di default
		var grinder = rheaMainMenuIcons[iIcon].defaultSelection[0];
		var cupSize = rheaMainMenuIcons[iIcon].defaultSelection[1];
		var shotType = rheaMainMenuIcons[iIcon].defaultSelection[2];
		
		selNum = rheaMainMenuIcons[iIcon].linkedSelection[grinder][cupSize][shotType];
	}
	
	return rhea.selection_getBySelNumber(selNum).price;
}

/*********************************************************
 * MMI_getImgForPageMenu
 *
 *	ritorna il nome della img da usare in pagina menu
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getImgForPageMenu (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return "upload/" +rheaMainMenuIcons[iIcon].pageMenuImg;
}


/*********************************************************
 * MMI_getImgForPageConfirm
 *
 *	ritorna il nome della img da usare in pagina confirm
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getImgForPageConfirm (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return "upload/" +rheaMainMenuIcons[iIcon].pageConfirmImg;
}


/*********************************************************
 * MMI_getLinkedSelectionNumber
 *
 *	ritorna la selezione [1..48] associata all'icona.
 *	Se ritorna 0, vuol dire che l'icona è associata ad una "selezione virtuale", ovvero una sorta di sottomenu di selezioni
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getLinkedSelectionNumber (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return rheaMainMenuIcons[iIcon].selNum;
}

/*********************************************************
 * MMI_canUseSmallCup /MediumCup / LargeCup
 *
 *	ritorna 1 se la selezione associata all'icona può usare la tazza piccola/media/ grande, 0 altrimenti
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_canUseSmallCup (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	if (rheaMainMenuIcons[iIcon].size.charAt(0) == "1")
		return 1;
	return 0;
}

function MMI_canUseMediumCup (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	if (rheaMainMenuIcons[iIcon].size.charAt(1) == "1")
		return 1;
	return 0;
}

function MMI_canUseLargeCup (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	if (rheaMainMenuIcons[iIcon].size.charAt(2) == "1")
		return 1;
	return 0;
}

/*********************************************************
 * MMI_hasDoubleShot
 *
 *	ritorna 1 se questa icona prevede l'extra shot
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_hasDoubleShot (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return 0;
		
	return rheaMainMenuIcons[iIcon].dblShot;
}

/*********************************************************
 * MMI_hasDblGrinder
 *
 *	ritorna 1 se questa icona prevede la scelta tra grinder 1 e 2
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_hasDblGrinder (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return 0;
		
	return rheaMainMenuIcons[iIcon].grinder2;
}

/*********************************************************
 * MMI_getLinkedSelection
 *
 *	date le 3 opzioni (grinder, cup_size, shot_type) ritorna il numero di selezione da erogare
 *	
 *	[iIcon] va da 0 a MMI_getCount()-1
 *	[GRINDER] = MMI_GRINDER_0 | MMI_GRINDER_1
 *	[CUP_SIZE] = MMI_CUP_SIZE_SMALL | MMI_CUP_SIZE_MEDIUM | MMI_CUP_SIZE_LARGE
 *	[SHOT_TYPE] = MMI_SHOT_TYPE_SINGLE | MMI_SHOT_TYPE_DBL
 */
function MMI_getLinkedSelection (iIcon, GRINDER, CUP_SIZE, SHOT_TYPE)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return selNum;
		
	return rheaMainMenuIcons[iIcon].linkedSelection[GRINDER][CUP_SIZE][SHOT_TYPE];
}

/*********************************************************
 * MMI_getDefaultLinkedSelectionOptions
 *
 *	Ritorna un oggetto che descrive le opzioni della selezione di default di una icona "virtuale".
 *	Per opzioni, si intende: quale grinder? quale cupSize? quale shotType?
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getDefaultLinkedSelectionOptions (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	
	var ret = {
		grinder: MMI_GRINDER_0,
		cupSize: MMI_CUP_SIZE_SMALL,
		shotType: MMI_SHOT_TYPE_SINGLE
	};


	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
	{
		//questo non dovrebbe accadere perchè solo le selezioni "virtuali" hanno opzioni e linkedSelection
		return ret;
	}
	
	//recupero la selezione di default
	ret.grinder = rheaMainMenuIcons[iIcon].defaultSelection[0];
	ret.cupSize = rheaMainMenuIcons[iIcon].defaultSelection[1];
	ret.shotType = rheaMainMenuIcons[iIcon].defaultSelection[2];	
	return ret;
}


/*********************************************************
 * MMI_getLinkedSelectionOptions
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 *	[selNum] va da 1 a 48
 */
function MMI_getLinkedSelectionOptions (iIcon, selNum)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;

	var ret = {
		grinder: MMI_GRINDER_0,
		cupSize: MMI_CUP_SIZE_SMALL,
		shotType: MMI_SHOT_TYPE_SINGLE
	};


	//in teoria non si dovrebbe mai chiamare questa fn su mainIcon che non siano "virtual"
	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return ret;
		
	//cerco la [selNum] all'interno di linkedSelection
	for (var grinder=0; grinder<=1; grinder++)
	{
		for (var cupSize=0; cupSize<=2; cupSize++)
		{
			for (var shotType=0; shotType<=1; shotType++)
			{
				if (selNum == rheaMainMenuIcons[iIcon].linkedSelection[grinder][cupSize][shotType])
				{
					ret.grinder = grinder;
					ret.cupSize = cupSize;
					ret.shotType = shotType;
					return ret;
				}
				
			}
		}
	}
	
	return ret;
}

/*********************************************************
 * MMI_isEnabled
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 *
 *	ritorna 1 se almeno una delle selezioni associate all'icona è abilitata
 */
function  MMI_isEnabled (iIcon)
{
	if (iIcon < 0) return 0;
	if (iIcon >= MMI_getCount()) return 0;

	var selNum = MMI_getLinkedSelectionNumber(iIcon);
	if (selNum != 0)
		return rhea.selection_getBySelNumber(selNum).enabled;
	
	for (var grinder=0; grinder<2; grinder++)
	{
		for (var cup=0; cup<3; cup++)
		{
			for (var shotType=0; shotType<2; shotType++)
			{
				selNum = MMI_getLinkedSelection  (iIcon, grinder, cup, shotType);			
				if (selNum > 0)
				{
					if (rhea.selection_getBySelNumber(selNum).enabled > 0)
						return 1;
				}
			}
			
		}
	}
	return 0;
}


/*********************************************************
 * MMI_fromSelNumToIconNum
 *
 *	Data una [selNum], ritorna l'indice della iconNum che la contiene
 */
function MMI_fromSelNumToIconNum (selNum)
{
	for (var iIcon=0; iIcon<MMI_getCount(); iIcon++)
	{
		if (rheaMainMenuIcons[iIcon].selNum == 0)
		{
			for (var grinder=0; grinder<=1; grinder++)
			{
				for (var cupSize=0; cupSize<=2; cupSize++)
				{
					for (var shotType=0; shotType<=1; shotType++)
					{
						if (selNum == rheaMainMenuIcons[iIcon].linkedSelection[grinder][cupSize][shotType])
							return iIcon;
					}
				}
			}
		}
		else
		{
			if (rheaMainMenuIcons[iIcon].selNum == selNum)
				return iIcon;			
		}
	}	
	return 0;
}