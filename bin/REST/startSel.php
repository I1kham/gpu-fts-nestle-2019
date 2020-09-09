<?php
//***è************
require_once "common.php";

$selNum = GUtils::httpGetOrDefaultInt("selNum", 0);
if ($selNum>1 && $selNum<100)
{
    $sok = rasPI_connect();
    if (false === $sok)
    {
        echo "Connection failed";
        die();
    }

    rasPI_sendString ($sok, "START-SEL|" .$selNum);
    rasPI_close ($sok);
}
?>
