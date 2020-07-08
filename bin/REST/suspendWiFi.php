<?php
//***è************
require_once "common.php";

$timeSec = httpGetOrDefault("timesec", "3");

$sok = rasPI_connect();
if (false === $sok)
{
    echo "KO";
    die();
}

rasPI_sendString ($sok, "SUSP-WIFI|$timeSec");
rasPI_close ($sok);
echo "OK";
?>
