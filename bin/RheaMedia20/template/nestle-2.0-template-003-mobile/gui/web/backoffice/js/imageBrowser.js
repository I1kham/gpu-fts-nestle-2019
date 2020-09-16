//***è**********************
function ImageBrowser (elemID)
{
	this.elemID = elemID;
	this.plsWaitID = elemID +"_plsWait";
	this.cntID = elemID +"_cnt";
	this.closeID = elemID +"_close";
	
	var html = "<div id='" +this.plsWaitID +"'><br><br><br><br><br><br><br><br><br><br><center><img src='img/animationRound.gif'></center></div>"
				+"<div id='" +this.cntID +"'></div>"
				+"<div id='" +this.closeID +"' style='position:absolute; top:5px; right:5px'><img src='img/close32x32.png'></div>";
	rheaSetDivHTMLByName(this.elemID, html);
	
	var me = this;
	var node = rheaGetElemByID(this.closeID);
	node.addEventListener("click", function (ev)
	{
		me.priv_showPleaseWait(1);
		setTimeout ( function() { me.priv_onFinish("", "");}, 10);
	}, 
	true);	
}

//*********************************************************
ImageBrowser.prototype.open = function (pathNoSlash, jolly, fnToCallOnEnd)
{
	this.priv_showPleaseWait(1);
	this.priv_showBackground(1);
	this.jolly = jolly;
	this.folderHistory = [];
	this.fnToCallOnEnd = fnToCallOnEnd;
	this.priv_queryFileList(pathNoSlash, jolly, 0);
}
	
//*********************************************************
ImageBrowser.prototype.priv_onFinish = function (selectedFileName, selectedPath)
{
	this.priv_showBackground(0);
	this.fnToCallOnEnd(selectedFileName, selectedPath);
}

//*********************************************************
ImageBrowser.prototype.priv_queryFileList = function (pathNoSlash, jolly, folderLevel)
{
	var me = this;
	this.folderHistory[folderLevel] = pathNoSlash;
	this.folderLevel = folderLevel;
	
	rhea.ajax ("FSList", { "path" : pathNoSlash, "jolly":jolly})
		.then( function(result)
		{
			//console.log ("[" +result +"]");
			setTimeout (function() {me.priv_onAjaxFSFinished(result, folderLevel);}, 100);
		})
		.catch ( function(result)
		{
			console.log ("ImageBrowser::open => err [" +result +"]");
			me.priv_showBackground(0);
		});
}

//*********************************************************
ImageBrowser.prototype.priv_showBackground = function (bShow)
{
	if (bShow)
		rheaShowElem(rheaGetElemByID(this.elemID));
	else
		rheaHideElem(rheaGetElemByID(this.elemID));
}


//*********************************************************
ImageBrowser.prototype.priv_showPleaseWait = function (bShow)
{
	if (bShow)
	{
		rheaHideElem(rheaGetElemByID(this.cntID));
		rheaShowElem(rheaGetElemByID(this.plsWaitID));
	}
	else
	{
		rheaHideElem(rheaGetElemByID(this.plsWaitID));
		rheaShowElem(rheaGetElemByID(this.cntID));
	}
}

//*********************************************************
ImageBrowser.prototype.priv_onAjaxFSFinished = function (fileList, folderLevel)
{
	var e = JSON.parse(fileList);
	var path = e.path;
	

	var html = "<ul class='thumbList'>";

	if (this.folderLevel > 0)
	{
		var id = this.elemID +"_folderUP";
		html += "<li id='" +id +"'><img src='img/iconFolderUP.png'></li>";
	}
	
	if (e.folderList != "")
	{
		var folderList = e.folderList.split("§");
		for (var i=0; i<folderList.length; i++)
		{
			var id = this.elemID +"_folder" +i;
			html += "<li id='" +id +"' data-foldername='" +folderList[i] +"'><img src='img/iconFolder.png'><p>" +folderList[i] +"</p></li>";
		}
	}
	
	if (e.fileList != "")
	{
		var imgList = e.fileList.split("§");
		for (var i=0; i<imgList.length; i++)
		{
			var id = this.elemID +"_li" +i;
			html += "<li id='" +id +"' data-filename='" +imgList[i] +"'><img src='" +path+"/"+imgList[i] +"'><p>" +imgList[i] +"</p></li>";
			
		}
	}
	html += "</ul>";

	rheaSetDivHTMLByName(this.cntID, html);
	this.priv_showPleaseWait(0);
	
	//bindo gli eventi on click
	var me = this;
	
	if (this.folderLevel > 0)
	{
			var id = this.elemID +"_folderUP";
			var node = rheaGetElemByID(id);
			node.addEventListener("click", function (ev)
			{
				var node = this;
				var folderLevel = me.folderLevel-1;
				var path = me.folderHistory[folderLevel];
				me.priv_showPleaseWait(1);
				
				setTimeout ( function() { me.priv_queryFileList(path, me.jolly, folderLevel);}, 10);
			}, 
			true);				
	}

	if (e.folderList != "")
	{
		for (var i=0; i<folderList.length; i++)
		{
			var id = this.elemID +"_folder" +i;
			var node = rheaGetElemByID(id);
			node.addEventListener("click", function (ev)
			{
				var node = this;
				var subFolder = node.getAttribute("data-foldername");
				
				var curFolder = me.folderHistory[me.folderLevel];
				me.priv_showPleaseWait(1);
				setTimeout ( function() { me.priv_queryFileList(curFolder+"/" +subFolder, me.jolly, me.folderLevel+1);}, 10);
			}, 
			true);				
		}
	}
	
	if (e.fileList != "")
	{
		for (var i=0; i<imgList.length; i++)
		{
			var id = this.elemID +"_li" +i;	
			var node = rheaGetElemByID(id);
			node.addEventListener("click", function (ev)
			{
				var node = this;
				var fileName = node.getAttribute("data-filename");
				var curFolder = me.folderHistory[me.folderLevel];
				me.priv_showPleaseWait(1);
				setTimeout ( function() { me.priv_onFinish(fileName, curFolder);}, 10);
			}, 
			true);
		}
	}	
	
}
