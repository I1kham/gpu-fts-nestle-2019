<?php
//***è************
require_once "common.php";

$selNum = GUtils::httpGetOrDefaultInt("selNum", 0);
$price  = GUtils::httpGetOrDefault("price", "100000");
if ($selNum>1 && $selNum<100 && price>="0")
{
    $sok = rasPI_connect();
    if (false === $sok)
    {
        echo "Connection failed";
        die();
    }

    rasPI_sendString ($sok, "START-PAID-SEL|" .$selNum ."|" .$price);
    rasPI_close ($sok);
}
?>
