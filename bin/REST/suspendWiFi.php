<?php
//***è************
require_once "common.php";

$timeSec = GUtils::httpGetOrDefaultInt("timesec", 0);
$sok = rasPI_connect();
if (false === $sok)
{
    echo "KO";
	debug_log_prefix ("suspendWifi", "KO\n");
    die();
}

rasPI_sendString ($sok, "SUSP-WIFI|$timeSec");
rasPI_close ($sok);

echo "OK";
debug_log_prefix ("suspendWifi", "OK\n");
?>
