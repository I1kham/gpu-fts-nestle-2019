<?php
//***è************
//error_reporting(~E_WARNING);

//******************************************************************
function rasPI_connect()
{
    if (! ($socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) )
        return false;
    
    $address = "localhost";
    $service_port = 2281;
    if (!socket_connect($socket, $address, $service_port))
        return false;
    //socket_set_nonblock($socket);
    socket_set_block($socket);
    return $socket;
}

//******************************************************************
function rasPI_send($socket, $buffer, $numBytesToSend)
{
    socket_write($socket, $buffer, $numBytesToSend);
}    

function rasPI_sendString($socket, $s)
{
    socket_write($socket, $s, strlen($s));
}    

//******************************************************************
function rasPI_waitAnswer($socket)
{
    socket_set_option($socket,SOL_SOCKET, SO_RCVTIMEO, array("sec"=>5, "usec"=>0));
    $result = trim(socket_read ($socket, 512, PHP_BINARY_READ));
    $result = trim(substr($result, 3));
    return $result;
}

function rasPI_echoAnswer($socket)
{
    $s = rasPI_waitAnswer($socket);
    echo $s;
}

//******************************************************************
function rasPI_close($socket)
{
    socket_close($socket);
}


//******************************************************************
function httpGetOrDefault($nomeVarInGet, $defaultValue)
{
    if (isset($_GET[$nomeVarInGet]) && $_GET[$nomeVarInGet]!="")
        return $_GET[$nomeVarInGet];
    return $defaultValue;
}

?>
