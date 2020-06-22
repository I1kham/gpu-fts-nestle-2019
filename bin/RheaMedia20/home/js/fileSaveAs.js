//****è**************
function FileSaveAs (elemID)
{
	this.elemID = elemID;
	this.plsWaitID = elemID +"_plsWait";
	this.driveList = elemID +"_dlst";
	this.cntID = elemID +"_cnt";
	this.closeID = elemID +"_close";
	this.saveBtnID = elemID +"_save";
	this.customTextAtTop = elemID +"_cstText";
	
	var html = "<div class='fileBrowser_container'>"
					+"<div class='fileBrowser_customTextAtTop' id='" +this.customTextAtTop +"'></div>"
					+"<div id='" +this.driveList +"' class='fileBrowser_driveList'><b>DRIVE LIST</b></div>"
					+"<div id='" +this.plsWaitID +"'><br><br><br><br><br><br><br><br><br><br><center><img src='img/animationRound.gif'></center></div>"
					+"<div id='" +this.cntID +"' class='fileBrowser_fileList'></div>"
					+"<div id='" +this.closeID +"' class='fileBrowser_closeBtn'>&nbsp;</div>"
			+"</div>";
	rheaSetDivHTMLByName(this.elemID, html);
	
	var me = this;
	var node = rheaGetElemByID(this.closeID);
	node.addEventListener("click", function (ev)
	{
		me.priv_showPleaseWait(1);
		setTimeout ( function() { me.priv_onFinish("");}, 10);
	}, 
	true);	
}

//*********************************************************
FileSaveAs.prototype.open = function (pathNoSlash, fnToCallOnEnd, customTextAtTop)
{
	this.priv_showPleaseWait(1);
	this.priv_showBackground(1);
	this.fnToCallOnEnd = fnToCallOnEnd;
	
	var divCustomText = rheaGetElemByID(this.customTextAtTop);
	if (customTextAtTop=="")
		rheaHideElem(divCustomText);
	else
	{
		rheaSetElemHTML(divCustomText, customTextAtTop);
		rheaShowElem(divCustomText);
	}
	
	var me = this;
	rhea.ajax ("FSDrvList", {})
		.then( function(result)
		{
			var e = JSON.parse(result);
			
			var html = "<ul class='FSDriveList'><li><b>DRIVE LIST</b></li>";
			
			if (e.desktop != "")
			{
				var id= me.elemID +"drvDesktop";
				html += "<li id='" +id +"' data-drive='" +e.desktop +"'>Desktop</li>";
			}
			
			for (var i=0; i<e.drivePath.length; i++)
			{
				var id= me.elemID +"drv" +i;
				html += "<li id='" +id +"' data-drive='" +e.drivePath[i] +"'>" +e.drivePath[i] +" (" +e.driveLabel[i] +")</li>";
			}
			html += "</ul>";
			rheaSetDivHTMLByName(me.driveList, html);
			
			
			if (e.desktop != "")
			{
				var id= me.elemID +"drvDesktop";
				var node = rheaGetElemByID(id);
				node.addEventListener("click", function (ev)
				{
					var node = this;
					var drive = node.getAttribute("data-drive");
					setTimeout ( function() { me.priv_queryFolderList(drive);}, 10);
				}, 
				true);
			}
			
			for (var i=0; i<e.drivePath.length; i++)
			{
				var id= me.elemID +"drv" +i;
				var node = rheaGetElemByID(id);
				node.addEventListener("click", function (ev)
				{
					var node = this;
					var drive = node.getAttribute("data-drive");
					setTimeout ( function() { me.priv_queryFolderList(drive);}, 10);
				}, 
				true);				
			}			
			
			setTimeout (function() {me.priv_queryFolderList(pathNoSlash);}, 10);
		})
		.catch ( function(result)
		{
			console.log ("FileSaveAs::open => err [" +result +"]");
			me.priv_showBackground(0);
		});
}
	
//*********************************************************
FileSaveAs.prototype.priv_onFinish = function (selectedPath)
{
	this.priv_showBackground(0);
	this.fnToCallOnEnd(selectedPath);
}


//*********************************************************
FileSaveAs.prototype.priv_queryFolderList = function (pathNoSlash)
{
	var me = this;
	rhea.ajax ("FSList", { "path":pathNoSlash, "jolly":"x.y"})
		.then( function(result)
		{
			setTimeout (function() {me.priv_onAjaxFSFinished(result);}, 100);
		})
		.catch ( function(result)
		{
			console.log ("FileSaveAs::open => err [" +result +"]");
			me.priv_showBackground(0);
		});
}

//*********************************************************
FileSaveAs.prototype.priv_showBackground = function (bShow)
{
	if (bShow)
		rheaShowElem(rheaGetElemByID(this.elemID));
	else
		rheaHideElem(rheaGetElemByID(this.elemID));
}


//*********************************************************
FileSaveAs.prototype.priv_showPleaseWait = function (bShow)
{
	if (bShow)
	{
		rheaHideElem(rheaGetElemByID(this.driveList));
		rheaHideElem(rheaGetElemByID(this.cntID));
		rheaShowElem(rheaGetElemByID(this.plsWaitID));
	}
	else
	{
		rheaHideElem(rheaGetElemByID(this.plsWaitID));
		rheaShowElem(rheaGetElemByID(this.cntID));
		rheaShowElem(rheaGetElemByID(this.driveList));
	}
}

//*********************************************************
FileSaveAs.prototype.priv_onAjaxFSFinished = function (fileList)
{
	var e = JSON.parse(fileList);
	this.currentFolder = e.path;
	
	var html = "<div style='margin:5px 0 5px 0;'>"
				+"<div style='float:left'><b style='color:#fff;'>Current folder:</b> " +this.currentFolder +"</div>"
				+"<div class='fileBrowser_saveBtn' id='" +this.saveBtnID +"' style='float:right; margin-right:5px'>SAVE HERE</div>"
				+"<div class='clear'></div>"
				+"</div><ul class='FSFileList'>";
	
	var bFolderUpExists = (parseInt(e.up) == 1);
	if (bFolderUpExists)
	{
		var id = this.elemID +"_folderUP";
		html += "<li id='" +id +"'><span class='fileBrowser_folderIcon'>&nbsp;</span><p>..</p></li>";
	}
	
	if (e.folderList != "")
	{
		var folderList = e.folderList.split("§");
		for (var i=0; i<folderList.length; i++)
		{
			var id = this.elemID +"_folder" +i;
			html += "<li id='" +id +"' data-foldername='" +folderList[i] +"'><span class='fileBrowser_folderIcon'>&nbsp;</span><p>" +folderList[i] +"</p></li>";
		}
	}
	
	html += "</ul>";

	rheaSetDivHTMLByName(this.cntID, html);
	this.priv_showPleaseWait(0);
	
	//bindo gli eventi on click
	var me = this;
	
	var node = rheaGetElemByID(this.saveBtnID);
	node.addEventListener("click", function (ev)
	{
		setTimeout ( function() { me.priv_onFinish(me.currentFolder);}, 10);
	}, 
	true);

	
	if (bFolderUpExists)
	{
			var id = me.elemID +"_folderUP";
			var node = rheaGetElemByID(id);
			node.addEventListener("click", function (ev)
			{
				var node = this;
				var path = me.currentFolder +"/..";
				me.priv_showPleaseWait(1);
				setTimeout ( function() { me.priv_queryFolderList(path);}, 10);
			}, 
			true);				
	}

	if (e.folderList != "")
	{
		for (var i=0; i<folderList.length; i++)
		{
			var id = me.elemID +"_folder" +i;
			//console.log (id);
			var node = rheaGetElemByID(id);
			node.addEventListener("click", function (ev)
			{
				var node = this;
				var subFolder = node.getAttribute("data-foldername");
				var path = me.currentFolder +"/" +subFolder;
				me.priv_showPleaseWait(1);
				setTimeout ( function() { me.priv_queryFolderList(path);}, 10);
			}, 
			true);				
		}
	}
}
