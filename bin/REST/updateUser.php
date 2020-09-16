<?php
/***è****************************************
    updateUser.php?userId=xx (una post che usiamo dopo la syncUser per dirti che abbiamo ricevuto e puoi cancellare le operazioni dell'utente)
  
    devo eliminare le transazioni dell'utente userId

*/
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
header('Access-Control-Allow-Methods: GET, POST, PUT');

require_once "common.php";
require_once "DBConn.php";


$userID = GUtils::httpGetOrDefaultInt("userId", 0);
debug_log_prefix ("updateUser.php", "userID=$userID\n");

if ($userID > 0)
{
	$db = new DBConn();
	
	$s = sessionGetCurrent ($db);
	if (false === $s)
	{
		debug_log_prefix ("updateUser.php", "ANSWER: KO, no session running, ma rispondo lo stesso OK\n");
		echo "OK";
		die();
	}

	if ($s["app_userID"] != $userID)
	{
		debug_log_prefix ("updateUser.php", "ANSWER: KO, user [$userID] is not the current session user [user=" .$s["app_userID"] . "] [sess=" .$s["sessionID"] ."]\n");
		echo "KO";
		die();
	}
		
	$rst = $db->Exec ("UPDATE transazioni SET bAlive=0 WHERE app_userID=" .$s["app_userID"]);
	debug_log_prefix ("updateUser.php", "ANSWER: OK, all transaction for user [" .$s["app_userID"] ."] have been deleted\n");
	echo "OK";
	
	sessionEnd ($db, $s["sessionID"]);
	die();
}

debug_log_prefix ("updateUser.php", "ANSWER: KO, invalid param, ma rispondo lo stesso OK\n");
echo "OK";
die();

/*else
{
	//invalid user
	echo "KO";
	debug_log_prefix ("updateUser.php", "ANSWER: KO, invalid user id\n");
}
*/
?>
