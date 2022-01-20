<?php
require_once "../REST/DBConn.php";
require_once "../REST/common.php";
$db = new DBConn();
$ses = session_debug_begin($db);

header ("Location: startup.html");
?>
