<?php
//***è************
//error_reporting(~E_WARNING);

/******************************************************************
 * debug log
 */
function debug_log_prefix ($prefix, $what)
{
    $f = fopen ("/var/www/html/rhea/REST.log", "a");
    
    while (strlen($prefix) < 20)
        $prefix.= " ";
    
    fwrite ($f, date("Y-m-d H:i:s") ."    $prefix    $what");
    fclose($f);
}

function debug_log ($what)          { debug_log_prefix ("", $what); }


/******************************************************************
 * socket stuff
 */
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

function rasPI_send($socket, $buffer, $numBytesToSend)      { socket_write($socket, $buffer, $numBytesToSend); }    
function rasPI_sendString($socket, $s)                      { socket_write($socket, $s, strlen($s)); }    

function rasPI_waitAnswer($socket)
{
    socket_set_option($socket,SOL_SOCKET, SO_RCVTIMEO, array("sec"=>5, "usec"=>0));
    $result = trim(socket_read ($socket, 512, PHP_BINARY_READ));
    $result = trim(substr($result, 3));
    return $result;
}

function rasPI_echoAnswer($socket)      { $s = rasPI_waitAnswer($socket); echo $s; }
function rasPI_close($socket){ socket_close($socket); }


//******************************************************************
function httpGetOrDefault($nomeVarInGet, $defaultValue)
{
    if (isset($_GET[$nomeVarInGet]) && $_GET[$nomeVarInGet]!="")
        return $_GET[$nomeVarInGet];
    return $defaultValue;
}

//******************************************************************
function sessionBegin ($db, $app_userID, $app_credit)
{
    $timeNow = time();
    
    //verfico se c'è già una sessione aperta
    $rst = $db->Q ("SELECT sessionID,lastTimeUpdated FROM thesession");
    if (count($rst) != 0)
    {
        $bAtLeastOnAlive = 0;
        foreach ($rst as $r)
        {
            $sec = $timeNow - $r["lastTimeUpdated"];
            if ($sec >= 120)
            {
                //la sessione è rimasta "inattiva" per più di 120 sec, la posso eliminare
                debug_log_prefix ("  sessionBegin", "deleting old session [" .$r["sessionID"] ."] [timeDiff=$sec] [timeNow=$timeNow] [lastTime=" .$r["lastTimeUpdated"] ."]\n");
                $db->Exec ("DELETE FROM thesession where sessionID=" .$r["sessionID"]);
            }
            else
                $bAtLeastOnAlive = 1;
        }
        
        if ($bAtLeastOnAlive)
        {
            //c'è una sessione in corso, non posso iniziarne una nuova
            debug_log_prefix ("  sessionBegin", "WARN unable to start new session, there's one already open\n");
            return 0;
        }
    }
    
    //creo la nuova sessione
    $db->Exec ("INSERT INTO thesession (sessionID,app_userID,app_creditAtStart,creditCurrent,lastTimeUpdated) VALUES($timeNow,$app_userID,'$app_credit','$app_credit',$timeNow)");
    debug_log_prefix ("  sessionBegin", "new session accepted is [$timeNow]\n");
    return $timeNow;
}

function sessionGetCurrent ($db)
{
    $rst = $db->Q ("SELECT sessionID,app_userID,creditCurrent FROM thesession ORDER BY sessionID DESC LIMIT 1");
    if (count($rst))
    {
        return array (
            "sessionID"     => $rst[0]["sessionID"],
            "app_userID"    => $rst[0]["app_userID"],
            "creditCurrent" => $rst[0]["creditCurrent"]
        );
        return 1;
    }
    
    return false;
}

function sessionUpdateTimestamp ($db, $sessionID)                       { $ts = time(); $db->Exec ("UPDATE thesession SET lastTimeUpdated=$ts WHERE sessionID=$sessionID"); }
function sessionUpdateCurrentCredit ($db, $sessionID, $credit)          { $ts = time(); $db->Exec ("UPDATE thesession SET creditCurrent='$credit', lastTimeUpdated=$ts WHERE sessionID=$sessionID"); }

function sessionTryTransaction ($db, $sessionID, $app_userID, $price, $productName)
{
    $rst = $db->Q ("SELECT creditCurrent FROM thesession WHERE sessionID=$sessionID AND app_userID=$app_userID");
    if (count($rst) == 0)
    {
        debug_log_prefix (" sesTryTransaction", "invalid sessionID [$sessionID]\n");
        return 0;
    }
    $creditCurrent = $rst[0]["creditCurrent"];
    
    if ($creditCurrent < $price)
    {
        debug_log_prefix (" sesTryTransaction", "not enought credit [credit:$creditCurrent] [price:$price]\n");
        return 0;
    }
    
    $dt = date("YmdHis");
    $dateUTC = "";
    $nomeProdotto = $db->toDBString($productName);
    $q = "INSERT INTO transazioni (sessionID,datetime,app_userID,dateUTC,what,productName,price) VALUES($sessionID,$dt,$app_userID,'$dateUTC',1,'$nomeProdotto','$price')";
    $db->Exec ($q);
    
    sessionUpdateCurrentCredit ($db, $sessionID, ($creditCurrent - $price));
    return 1;
}

function sessionEnd ($db, $sessionID)
{
    $rst = $db->Q ("SELECT app_userID FROM thesession WHERE sessionID=$sessionID");
    if (count($rst) == 0)
        return;
    debug_log_prefix ("  sessionEnd", "session [$sessionID] closed\n");
    $db->Exec ("DELETE FROM thesession WHERE sessionID=$sessionID");
}
?>
