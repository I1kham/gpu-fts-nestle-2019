<?php
//***************è********+
require_once "../common/DBConn.php";

$db = new DBConn();

$nome = DBConn::toDBString ("L'aria in città è molto sporca");
$cognome = DBConn::toDBString ("你好! (nǐ hǎo) Hi!");
$db->Exec ("INSERT INTO test (Nome,Cognome) VALUE ('$nome', '$cognome')");

$rst = $db->Q ("SELECT * FROM test");
echo "<pre>";print_r($rst); echo "</pre>";
?>
