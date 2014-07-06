<?php

$conn = mysql_connect("mysql.cs.ccu.edu.tw","lym101m","mike610") or die('connect error');
mysql_query("SET NAMES 'utf8'");
mysql_select_db("lym101m_nupedia");

echo "connected!";

$sql ="LOAD DATA LOCAL INFILE './relatedCorpus_forDB' INTO TABLE relatedCorpus;";

$result = mysql_query($sql) or die('Mysql query error');

mysql_close($conn);

echo 'done!';
?>
