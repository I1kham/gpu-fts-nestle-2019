<?php
//***è************
require_once "common.php";

$sok = rasPI_connect();
if (false === $sok)
{
    echo "6";
    die();
}

rasPI_sendString ($sok, "GET-SEL-STATUS");
rasPI_echoAnswer($sok);
rasPI_close ($sok);
?>
