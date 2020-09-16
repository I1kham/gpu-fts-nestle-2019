<?php
/***è****************************************
	saveUser.php (una post che usiamo in connessione per passarti l'utente)

	$_POST = 
	{
	  "id": 8,
	  "credit": 5
	}
	
	Io la uso anche per iniziare una sessione
​*/
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
header('Access-Control-Allow-Methods: GET, POST, PUT');

require_once "common.php";
require_once "DBConn.php";


$in = file_get_contents("php://input");
if (!isset($in) || $in == "")
{
	debug_log_prefix ("saveUser.php", "nothing in post, ANSWER: none\n");
	die();
}

$data = json_decode(json_decode($in)->userObj);
if (!isset($data))
{
	debug_log_prefix ("saveUser.php", "nothing in post, ANSWER: none\n");
	die();
}
debug_log_prefix ("saveUser.php", "***********************************\n" .print_r($data,true) ."\n");

//analizzo i dati in post
$app_userID = $data->id;
$app_credit = $data->credit;
$app_numDecimaliPrezzo = 2;
if (ISSET($data->numDecimali))
	$app_numDecimaliPrezzo = $data->numDecimali;

$app_simboloValuta = "";
if (ISSET($data->simboloValuta))
	$app_simboloValuta = $data->simboloValuta;

//verifico se posso aprire una sessione
$db = new DBConn();
$sessionID = sessionBegin ($db, $app_userID, $app_credit, $app_numDecimaliPrezzo, $app_simboloValuta);
if ($sessionID == 0)
{
	echo "KO";
	debug_log_prefix ("saveUser.php", "ANSWER: KO, a session is already running\n");	
}
else
{
	echo "OK";
	debug_log_prefix ("saveUser.php", "ANSWER: OK\n");	
}
?>
