/*********************************************************
 *  In ogni momento, la GPU potrebbe segnalare degli specifici eventi degni di nota per la GUI.
 *	La pagina web può gestire questi eventi facendo "l'override" delle seguenti funzioni.
 *	Non è necessario che la pagina web gestista tutti questi eventi, può limitarsi a gestire quelli che le interessano.
 *
 *	Es:
	 	rhea.onEvent_selectionAvailabilityUpdated = function ()
			{
				alert ("sel avail updated");
			}
			
		rhea.onEvent_selectionPricesUpdated = function ()
			{
				alert ("sel prices update");
			}
 */

Rhea.prototype.onEvent_selectionAvailabilityUpdated = function()	{}

Rhea.prototype.onEvent_selectionPricesUpdated = function()	{}

Rhea.prototype.onEvent_creditUpdated = function()	{}

Rhea.prototype.onEvent_cpuMessage = function(msg, importanceLevel)	{}

Rhea.prototype.onEvent_selectionReqStatus = function(status)	{}

Rhea.prototype.onEvent_cpuStatus = function(statusID, statusStr)	{}

Rhea.prototype.onEvent_readDataAudit = function(status, kbSoFar, fileID)	{}

