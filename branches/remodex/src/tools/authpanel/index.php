<?php

include_once("functions.php");

// Page title
$title = "Remod authkey control page";

// Database connection
$host = "localhost";
$user = "remod";
$password = "remod";
$database = "remod";
$table = 'auth';

// Set this if you want set password for control page
$protected = 1;
$adminname = "admin";
$adminpass = "";

// Default login and password warning
$warnings = array();
if($adminname == "admin" && $adminpass == "") $warnings[] = 'You are using default login and password. Please change $adminname and $adminpass in index.php to something else.';

// Authorization
if($protected)
	if(!isset($_SERVER['PHP_AUTH_USER']))
	{
 	  	header('WWW-Authenticate: Basic realm="'.$title.'"');
 	  	header('HTTP/1.0 401 Unauthorized');
 	  	echo "Authorization Required";
 	  	exit;
	}
	else
	{
		// Check for valid login and password
		if(strcmp($_SERVER['PHP_AUTH_USER'], $adminname) !== 0 || strcmp($_SERVER['PHP_AUTH_PW'], $adminpass) !== 0)
		{
			header('WWW-Authenticate: Basic realm="'.$title.'"');
			header('HTTP/1.0 401 Unauthorized');
 	  		die("You are not right, skinny");
 	  		exit;
		}
	}

$mysqli = mysqli_connect($host, $user, $password, $database);

if ($mysqli->connect_errno)
{
    echo "Failed to connect to MySQL: (".$mysqli->connect_errno .") ".$mysqli->connect_error;
    exit;
}

switch($_GET["action"])
{
    case "setenable":
        $id     = $_GET["id"];
        $val    = $_GET["val"];        
        setenable($mysqli, $id, $val);
        header("Location: ./");
        exit;
        
    case "add":
        $name   = $_POST["name"];
        $pubkey = $_POST["pubkey"];
        $rights = $_POST["rights"];
        $enabled = ($_POST["enabled"] == "1");
        
        // Check for errors
		$adderrors = false;
		if(strlen($name) == 0) { $warnings[] = "User name shouldn't be empty."; $adderrors = true; }
        if(strlen($pubkey) != 49) { $warnings[] = "Public key must be 49 characters."; $adderrors = true; }
   		if(!in_array($rights, array("m", "M", "a", "A"))) { $warnings[] = "Rights must be 'm' or 'M' or 'a' or 'A' but not '$rights'."; $adderrors = true; }
		        
        if(!$adderrors)
        {
        	adduser($mysqli, $name, $pubkey, $rights, $enabled);
        	header("Location: ./");
        	exit;
        }
   		break;
        
    case "delete":
        $id     = $_GET["id"];
        $confirm = $_GET["confirm"];
        
        if($confirm == "1")
        {
            deleteuser($mysqli, $id);
            header("Location: ./");
        }
        else
        {
            deleteuserconfirm($id);
        }
        exit;
}

$res = mysqli_query($mysqli, "SELECT * FROM $table ORDER BY rights, enabled DESC");
?>

<!DOCTYPE HTML>
<html>
<head>
<title><?php echo($title); ?></title>
<style type="text/css">
.warning {
	background-color: #ffeeee;
	color: #ff0000;
	padding: 20px;
	margin: 20px;
	border-radius: 10px;
	}

.note {
	background-color: #ffffbb;
	padding: 20px;
	margin: 20px;
	border-radius: 10px;
	}

.container {
	float: left; }

.caption {
    text-align: center; }

.authkeys-table {
	border-collapse:collapse;
	text-align: center;
	width: 100%;
	margin: 0 20px; }
	
.authkeys-table thead {
	background-color: #eeeeee;
	font-weight: bold; }
	
.authkeys-table tr td {
	padding: 5px 10px 5px 10px;
	border-bottom: solid 2px #fff;
	border-left: solid 1px #fff;
	border-right: solid 1px #fff; }
	
.enabled {
	background-color: #eeffee; }
	
.disabled {
	background-color: #ffeeee;
	color: #777; }
	
.link-enable {
	text-decoration: underline;
	color: #5b5; }
	
.link-disable, .delete {
	text-decoration: underline;
	color: #b55; }
	
.link-disable:hover, .delete:hover {
	color: #f00; }
	
.link-enable:hover {
	color: #0f0; }
	
.addform {
	margin: 20px; }

.addform .title {
	margin-left: 15px;
	padding: 3px;
	background-color: #fff; }

.formtable {
	margin-top: -8px;
	padding: 10px;
	border: solid 1px #eee;
	background-color: #efe;
	border-radius: 10px;
     }

.formtable thead {
	text-align: center;
	font-weight: bold; }

</style>
</head>

<body>
	<div class="container">
<?php
	// Show warnings
	if(count($warnings) > 0)
	{
		echo("<div class=\"warning\"><ul><h3>Warnings</h3>");
		foreach ($warnings as $warn)
    		echo("<li>$warn</li>");
    	echo("</ul></div>");
   	}
	
?>
    <h1 class="caption"><?php echo($title); ?></h1>
	<table class="authkeys-table">
		<thead><tr><td>Name</td><td>Public key</td><td>Rights</td><td>Status</td><td>Action</td></tr></thead>
        
<?php
    while ($row = mysqli_fetch_assoc($res))
    {
        $id = $row["id"];
        
        if($row["enabled"] == 1)
        {
            $enabled = 0;
            $enabledname = "Active";
            $enabledclass = "enabled";
            $linkclass = "link-disable";
            $linktext = "disable";
        }
        else
        {
            $enabled = 1;            
         	$enabledname = "Disabled";
            $enabledclass = "disabled";
            $linkclass = "link-enable";
            $linktext = "enable";
        }
        
        echo("<tr class=\"$enabledclass\">\n");
        echo("<td>".$row["name"]."</td>\n");
        echo("<td>".$row["pubkey"]."</td>\n");
		switch($row["rights"])
		{
    		case "m":
    		case "M":
        	$rightsname = "Master";
        	break;
        	
    		case "a":
    		case "A":
        	$rightsname = "Admin";
        	break;
		}        
        echo("<td>$rightsname</td>\n");
        echo("<td>$enabledname</td>\n");
        echo("<td>[<a href=\"?action=setenable&id=$id&val=$enabled\" class=\"$linkclass\">$linktext</a>] [<a class=\"delete\" href=\"?action=delete&id=$id\">remove</a>]</td></tr>\n");
    }
?>

</table>
<form class="addform" action="?action=add" method="post">
		<div>
		<span class="title">Add new authkey</span>
			<table class="formtable">
				<thead><td>Name</td><td>Public key</td><td>Rights</td><td>&nbsp;</td></thead>
				<tr>
					<td><input name="name"></td>
					<td><input name="pubkey" size="49" maxlength="49"></td>
					<td>
						<input type="radio" name="rights" value="m" checked>Master<br>
						<input type="radio" name="rights" value="a">Admin
					</td>
					<td>
						<input type="radio" name="enabled" value="1" checked>Enabled<br>
						<input type="radio" name="enabled" value="0">Disabled
					</td>
				</tr>
				<tr>
					<td colspan="3">&nbsp;</td>
					<td><input type="submit" value="Add user"></td>
				</tr>
			</table>
			
		</div>
	</form>
	<div class="note">Note: do not forgot to reload authkeys on server with #syncauth</div>
	</div>
</body>
</html>
