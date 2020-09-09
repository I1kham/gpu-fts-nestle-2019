<?php
/****è*****************************************************
 * DBConn
 ******************************************************/
class	DBConn
{
	var $conn;
	var	$debug_lastQ;
	var	$bTransactionInProgress;
	
	static function toDBString ($strIN) 		
	{ 
		$ret = str_replace ("'","''",$strIN); 
		return str_replace ("\\","\\\\",$ret);  
	}

	
	function __construct()
	{
		$this->bTransactionInProgress = 0;
		$this->Connect ("localhost", "raspiuser", "hofd93jl","raspi");
	}
	
	//*****************************
	function Connect ($strHost, $strUser, $strPwd, $strNomeDB)				
	{	
		$this->conn = mysqli_connect( $strHost, $strUser, $strPwd, $strNomeDB);
		if (mysqli_connect_errno ($this->conn))
			die ("Failed to connect to MySQL: " . mysqli_connect_error());
		mysqli_set_charset($this->conn, "utf8");
	}


	//*****************************
	function Q ($strQ)	
	{
		$this->debug_lastQ = $strQ;
		$res = mysqli_query ($this->conn, $strQ);
		if ($res === FALSE)	
		{
			if ($this->bTransactionInProgress)
				$this->rollback();
			echo "ERRORE NELLA QUERY: $strQ<br>";
			die();
		}

		$e = array();
		for ($i=0; $i<$res->num_rows; $i++)
			$e[$i] = mysqli_fetch_array  ($res, MYSQLI_ASSOC);
		mysqli_free_result ($res);
		return $e;
	}
	
	//*****************************
	function	getAutoIncrID()	
	{
		return mysqli_insert_id ($this->conn);
	}
	
	//*****************************
	function Exec ($strQ)								
	{ 
		if (!mysqli_query ($this->conn, $strQ))  
		{
			if ($this->bTransactionInProgress)
				$this->rollback();
			die ("ERRORE NELLA QUERY: $strQ"); 
		}
	}
	function rawExec ($strQ)							{ return mysqli_query ($this->conn, $strQ); }
	
	//*****************************
	function beginTransactionW()
	{
		mysqli_begin_transaction($this->conn, MYSQLI_TRANS_START_READ_WRITE);
		$this->bTransactionInProgress = 1;
	}
	
	function commit()
	{
		mysqli_commit($this->conn);
		$this->bTransactionInProgress = 0;
	}
	
	function rollback ()
	{
		mysqli_rollback ($this->conn);
		$this->bTransactionInProgress = 0;
	}	
}
?>
