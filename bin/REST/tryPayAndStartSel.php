<?php
//***è************
require_once "common.php";
$selNum = GUtils::httpGetOrDefaultInt("selNum", 0);

debug_log_prefix ("  tryPayAndStartSel", "[selNum=$selNum]\n");
if ($selNum>=1 && $selNum<100)
{
    $sok = rasPI_connect();
    if (false === $sok)
    {
        echo "Connection failed";
        die();
    }

    rasPI_sendString ($sok, "START-SEL|" .$selNum);
    rasPI_close ($sok);
    echo "OK";
    debug_log_prefix ("  tryPayAndStartSel", "OK\n");
    die();
}
debug_log_prefix ("  tryPayAndStartSel", "KO\n");
echo "KO";

/*	2020-09-16 
	commentato perchè vogliamo eliminare il discorso del pagamento

require_once "common.php";
require_once "DBConn.php";
	
$sessionID = GUtils::httpGetOrDefaultInt("sesID", 0);
$app_userID = GUtils::httpGetOrDefaultInt("app_userID", 0);
$selNum = GUtils::httpGetOrDefaultInt("selNum", 0);
$price = floatval(GUtils::decodeURL(GUtils::httpGetOrDefault("price", "-1")));
$selName = GUtils::decodeURL(GUtils::httpGetOrDefault("selName", ""));



debug_log_prefix ("  tryPayAndStartSel", "[sesID=$sessionID][appUserID=$app_userID][selNum=$selNum][price=$price][selName=$selName]\n");


if ($selNum>=1 && $selNum<=48 && $price>=0)
{
	$db = new DBConn();
	if (0 == sessionTryTransaction ($db, $sessionID, $app_userID, $price, $selName))
	{
		echo "Not enough credit";
		debug_log_prefix ("  tryPayAndStartSel", "Not enough credit\n");
		die();
	}	


    $sok = rasPI_connect();
    if (false === $sok)
    {
        echo "Connection failed";
        debug_log_prefix ("  tryPayAndStartSel", "Connection failed\n");
        die();
    }

    rasPI_sendString ($sok, "START-PAID-SEL|" .$selNum ."|" .$price);
    
    rasPI_close ($sok);
    echo "OK";
    debug_log_prefix ("  tryPayAndStartSel", "OK\n");
    die();
}

debug_log_prefix ("  tryPayAndStartSel", "Invalid parameters\n");
echo "Invalid parameters [sesID=$sessionID][appUserID=$app_userID][selNum=$selNum][price=$price][selName=$selName]";
die();
*/
?>
