<?php

function setenable($db, $id, $val)
{
    global $table;
    $query = "UPDATE $table SET enabled=$val WHERE id=$id";
    mysqli_query($db, $query);
}

function adduser($db, $name, $pubkey, $rights, $enabled)
{
    global $table;
    $query = "INSERT INTO $table (name, pubkey, rights, enabled) VALUES(\"$name\", \"$pubkey\", \"$rights\", $enabled)";
    mysqli_query($db, $query);
}

function deleteuser($db, $id)
{
    global $table;
    $query = "DELETE FROM $table WHERE id=$id";
    mysqli_query($db, $query);
}

function deleteuserconfirm($id)
{
    echo("<html>
<head>
<title>Confirm delete</title>
</head>
<body>
Confirm delete:<br>
[<a href=\"?action=delete&id=$id&confirm=1\">Yes, delete</a>] [<a href=\"".$_SERVER["DOCUMENT_URI"]."\">No, cancel</a>]
</body></html>");
}

?>
