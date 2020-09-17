<?php
/***è****************************************
    syncUser.php?userId=xx (una get che usiamo in disconesione per chiederti l'utente aggiornato)
  
    In sostanza, devo riportare tutte le transazioni dell'utente che ho ancora in pancia

*/
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
header('Access-Control-Allow-Methods: GET, POST, PUT');

require_once "common.php";
require_once "DBConn.php";

$userID = GUtils::httpGetOrDefaultInt("userId", 0);
debug_log_prefix ("syncUser.php", "userID=$userID\n");

if ($userID >= 0)
{
	$db = new DBConn();

	$s = sessionGetCurrent ($db);
	if (false === $s)
	{
		debug_log_prefix ("syncUser.php", "ANSWER: KO, no session running\n");
		echo "KO";
		die();
	}

	if ($s["app_userID"] != $userID)
	{
		debug_log_prefix ("syncUser.php", "ANSWER: KO, user [$userID] is not the current session user [user=" .$s["app_userID"] . "] [sess=" .$s["sessionID"] ."]\n");
		echo "KO";
		die();
	}
	
	
	$result = array (
		"id" 			=> 	$s["app_userID"],
		"credit"		=>	$s["creditCurrent"],
		"operations"	=> array()
	);
	
	$rst = $db->Q ("SELECT dateUTC,what,productName,price FROM transazioni WHERE app_userID=" .$s["app_userID"] ." AND bAlive=1 ORDER BY sessionID, datetime");
	foreach ($rst as $r)
	{
		$detail = array(
			"userId" 	=> $s["app_userID"],
			"dateUtc" 	=> $r["dateUTC"],
			"type" 		=> $r["what"],
			"product" 	=> $r["productName"],
			"credit" 	=> $r["price"]
		);
		
		array_push ($result["operations"], $detail);
	}
	
	$answer = json_encode ($result, JSON_NUMERIC_CHECK);
	debug_log_prefix ("syncUser.php", "ANSWER: $answer\n");
	echo $answer;	
}
else
{
	//invalid user
	echo "KO";
	debug_log_prefix ("syncUser.php", "ANSWER: KO, invalid user id\n");
}

//$data = gmdate("Y-m-d\TH:i:s\Z");

?>
