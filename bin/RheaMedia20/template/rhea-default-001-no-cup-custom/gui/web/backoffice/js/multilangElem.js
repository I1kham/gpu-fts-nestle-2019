/*********************************************************
 * MultilangElem
 *
 * inputType: [EDIT | TEXTAREA]
 */
function MultilangElem (elemID, dbUID, dbWHAT, inputType, widthOfInputBox, languageList)
{
	this.elemID = elemID;
	this.dbUID = dbUID;
	this.dbWHAT = dbWHAT;
	this.allLang = languageList.split(",");
	
	var w = "width:" +widthOfInputBox;
	var idOfDiv1 = elemID +"_other";
	
	var iso = this.allLang[0];
	var html =   "<div>"
				+"	<div style='float:left; padding-top:4px'><img class='flag' src='img/flags/" +iso +".svg'></div>"
				+"	<div style='float:left'>" +this.priv_getHTMLForInputElem(inputType, widthOfInputBox, iso) +"</div>"
				+"	<div style='float:left'><a href='javascript:showHideElem(\"" +idOfDiv1 +"\")'><img src='img/flagsButton.png'></a></div>"
				+"	<div class='clear'></div>"
				+"</div>"
				+"<div id='" +idOfDiv1 +"' style='display:none'><span class='tip'>If you leave a language blank, then it will automatically default to " +this.allLang[0] +"</span>"
				+"<table border='0' cellspacing='0' cellpadding='2'>";
				
				for (var i=1; i<this.allLang.length; i++)
				{
					var iso = this.allLang[i];
					html += "	<tr valign='middle'><td><img class='flag' src='img/flags/" +iso +".svg'></td><td>" 
							+ this.priv_getHTMLForInputElem(inputType, widthOfInputBox, iso) +"</td></tr>"
				}
				html += "</table>";
				
	$("#" +elemID).html (html);
}

//*********************************************************
MultilangElem.prototype.priv_getHTMLForInputElem = function (inputType, widthOfInputBox, isoLanguage)
{
	if (inputType=="EDIT")
		return "<input type='text' id='" +this.elemID +"_" +isoLanguage +"' class='edit' style='width:" +widthOfInputBox +"' value=''>";
	return "<textarea id='" +this.elemID +"_" +isoLanguage +"' class='edit' style='height:80px; width:" +widthOfInputBox +"'></textarea>";
}

//*********************************************************
MultilangElem.prototype.load = function(db)
{
	var me = this;
	return db.q("SELECT ISO,Message FROM lang WHERE UID='" +me.dbUID +"' AND What='" +me.dbWHAT +"'")
		.then( function(rst)
		{
			var langFoundInDB = [];
			for (var r=0; r<rst.getNumRows(); r++)
			{
				var iso = rst.valByColName(r, "ISO");
				
				langFoundInDB.push(iso);
				for (var i=0; i<me.allLang.length; i++)
				{
					if (me.allLang[i] == iso)
					{
						var msg = rst.valByColName(r, "Message");
						if (msg == "NULL")
							msg="";
						$("#" +me.elemID +"_" +iso).val (msg);
						break;
					}
				}
			}
			
			//aggiungo al db un record per ogni lingua di "me.allLang" che non sia già esistente nel DB stesso
			var sql = "";
			for (var i=0; i<me.allLang.length; i++)
			{
				var iso = me.allLang[i];
				if (langFoundInDB.indexOf(iso) == -1)
				{
					if (sql != "")
						sql += ",";
					sql += "('" +me.dbUID +"','" +iso +"','" +me.dbWHAT +"','')";
				}
			}
				
			if (sql != "")
			{
				sql = "INSERT INTO lang (UID,ISO,What,Message) VALUES " +sql +";"; 
				console.log(sql);
				return db.exec(sql);
			}
			
		});
}

//*********************************************************
MultilangElem.prototype.save = function(db)
{
	var me = this;
	
	var sql = [];
	for (var i=0; i<me.allLang.length; i++)
	{
		var iso = me.allLang[i];
		var s = db.toDBString($("#" +me.elemID +"_" +iso).val());
		//var p  = db.exec("UPDATE lang SET Message='" +s +"' WHERE UID='" +me.dbUID +"' AND ISO='" +iso +"' AND What='" +me.dbWHAT +"';")
		var p  = db.exec("DELETE FROM lang WHERE UID='" +me.dbUID +"' AND ISO='" +iso +"' AND What='" +me.dbWHAT +"'");
		sql.push (p);
		
		var p2 = db.exec("INSERT INTO lang (UID,ISO,What,Message) VALUES('" +me.dbUID +"','" +iso +"','" +me.dbWHAT +"','" +s +"')");
		sql.push (p2);
	}
	
	
	
	return Promise.all(sql);
}