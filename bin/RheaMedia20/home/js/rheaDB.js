/*è********************************************************
 * RheaDB
 *
 * costruttore
 */
function RheaDB(absoluteDBFilePathAndName)
{
	this.absoluteDBFilePathAndName = absoluteDBFilePathAndName;
	this.dbHandle = 0;
}

//*********************************************************
RheaDB.prototype.toDBString = function (str)
{
	var s = encodeURI(str);
	return s.replace(/\'/g,"''");
}


//*********************************************************
RheaDB.prototype.q = function(sql)
{
	if (this.dbHandle != 0)
	{
		return rhea.ajax ("DBQ", { "h" : this.dbHandle, "sql" : sql }).then( function(result)
						{
							return new RheaRST(result);
						});
	}
	else
	{
		var me = this;
		return rhea.ajax ("DBC", { "path" : this.absoluteDBFilePathAndName})
				.then( function(result)
				{
					var j = JSON.parse(result);
					me.dbHandle = j.handle;
					return rhea.ajax ("DBQ", { "h" : me.dbHandle, "sql" : sql }).then( function(result)
						{
							return new RheaRST(result);
						}
					);
				});
	}
}

//*********************************************************
RheaDB.prototype.exec = function(sql)
{
	if (this.dbHandle != 0)
	{
		return rhea.ajax ("DBE", { "h" : this.dbHandle, "sql" : sql });
	}
	else
	{
		var me = this;
		return rhea.ajax ("DBC", { "path" : this.absoluteDBFilePathAndName})
				.then( function(result)
				{
					var j = JSON.parse(result);
					me.dbHandle = j.handle;
					return rhea.ajax ("DBE", { "h" : this.dbHandle, "sql" : sql });

				});
	}
}

//*********************************************************
RheaDB.prototype.dbConsistency = async function(rst, dbTbl, dbCol, dbColType, dfColVal) {
	if ( !rst || !dbTbl || !dbCol || !dbColType ) { return false; }

	let exist = rst.hasColName( dbCol );

	if( !exist ) {
		var def = dfColVal !== undefined? " DEFAULT " + dfColVal : "";
		var q = "ALTER TABLE " + dbTbl + " ADD " + dbCol +" " + dbColType + def;

		await this.exec(q);
	}
}

/*********************************************************
 * RheaRST
 *
 * costruttore
 */
function RheaRST(rstFromDBQuery)
{
	if (rstFromDBQuery=="KO")
	{
		this.err = "KO";
		this.nRows = 0;
		this.nCols = 0;
		this.offsetColNames = 0;
		this.offsetFirstRow = 0;
		
	}
	else
	{
		this.err = "";
		this.data = rstFromDBQuery.split("§");
		this.nRows = parseInt(this.data[0]);
		this.nCols = parseInt(this.data[1]);
		this.offsetColNames = 2;
		this.offsetFirstRow = 2 + this.getNumCols();
	}
}

/***************************************
 * ritorn "" se this ? valido, altrimenti una string con un codice di errore
 */
RheaRST.prototype.hasError = function()				{ return this.err; }

 
//***************************************
RheaRST.prototype.getNumRows = function()			{ return this.nRows; }
RheaRST.prototype.getNumCols = function()			{ return this.nCols; }


/***************************************
 * ritorna il nome della colona i-esima con iCol>=0 && iCol<NUM-COL
 */
RheaRST.prototype.getColName = function(iCol)		
{ 
	var i = parseInt(iCol);
	if (i>=0 && i<this.nCols)
		return this.data[this.offsetColNames + iCol]; 
	return "err::getColName(" +iCol +")";
}

/***************************************
 * ritorna il valore dell' i-esimo campo della n+esima riga iRow>=0 && iRow<NUM-ROW e iCol>=0 && iCol<NUM-COL
 */
RheaRST.prototype.val = function (iRow, iCol)
{ 
	var r = parseInt(iRow);
	var c = parseInt(iCol);
	if (r>=0 && r<this.nRows && c>=0 && c<this.nCols)
		return this.data[this.offsetFirstRow + iRow*this.nCols + iCol];
	return "err";
}

//***************************************
RheaRST.prototype.fromColNameToColIndex = function (colName)
{
	for (var i=0; i<this.getNumCols(); i++)
	{
		if (this.getColName(i) === colName)
			return i;
	}
	return -1;	
}

/***************************************
 */
RheaRST.prototype.valByColName = function (iRow, colName)
{ 
	var iCol = this.fromColNameToColIndex(colName);
	if (iCol<0)
		return "err::valByColName(" +iRow +"," +colName +")";
	return this.val(iRow,iCol);
}

/***************************************
 */
 RheaRST.prototype.hasColName = function (colName) { 
	var iCol = this.fromColNameToColIndex(colName);
	
	return (iCol >= 0);
}