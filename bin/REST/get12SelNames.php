<?php
//***è************
require_once "common.php";

$sok = rasPI_connect();
if (false === $sok)
{
    echo "Connection failed";
    die();
}
rasPI_sendString ($sok, "GET-12SEL-NAMES");
rasPI_echoAnswer($sok);
rasPI_close ($sok);
?>
