<?php
//***è************
require_once "common.php";

$selNum = GUtils::httpGetOrDefaultInt("s", 1);
$sok = rasPI_connect();
if (false === $sok)
{
    echo "Connection failed";
    die();
}

rasPI_sendString ($sok, "GET-SEL-PRICE|" .$selNum);
rasPI_echoAnswer($sok);
rasPI_close ($sok);
?>
