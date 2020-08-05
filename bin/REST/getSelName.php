<?php
//***è************
require_once "common.php";

$selNum = intval(httpGetOrDefault("s", "1"));
$sok = rasPI_connect();
if (false === $sok)
{
    echo "Connection failed";
    die();
}

rasPI_sendString ($sok, "GET-SEL-NAME|" .$selNum);
rasPI_echoAnswer($sok);
rasPI_close ($sok);
?>
