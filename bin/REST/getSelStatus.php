<?php
//***è************
$sok = rasPI_connect();
if (false === $sok)
{
    echo "Connection failed";
    die();
}

rasPI_sendString ($sok, "GET-SEL-STATUS");
rasPI_echoAnswer($sok);
rasPI_close ($sok);
?>
