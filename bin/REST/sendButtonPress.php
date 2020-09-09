<?php
//***è************
require_once "common.php";

$btnNum = GUtils::httpGetOrDefaultInt("b", 0);
if ($btnNum>=1 && $btnNum<=12)
{
    $sok = rasPI_connect();
    if (false === $sok)
    {
        echo "Connection failed";
        die();
    }

    rasPI_sendString ($sok, "SND-BTN|" .$btnNum);
    rasPI_close ($sok);
}
?>
