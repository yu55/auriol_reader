<html>
    <head>
        <title>Sportowa8 Meteo</title>
        <meta charset="utf-8">
        <style type='text/css'>
            body {
                font-family: Arial, sans-serif;
                font-size: 100%;
            }
        </style>
    </head>
<body>
<h2>Sportowa8 Meteo (stacja pogody AURIOL H13726)</h2>
<?php
$db = new SQLite3('/var/local/auriol-db.sl3');

echo "<table>";
echo "<col width=\"425\">";
echo "<col width=\"300\">";

$results = $db->query("SELECT created, temperature FROM temperature ORDER BY created DESC LIMIT 1;");
$created = " ";
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $temperature = number_format($row['temperature'], 1);
    $created = $row['created'];
    echo "<tr>";
    echo "<td>Temperatura powietrza</td><td> <b>{$temperature} &#8451;</b></td>";
    echo "</tr>";
}
$result = $results->finalize();

$results = $db->query("SELECT min(temperature) AS tmin, max(temperature) AS tmax FROM temperature WHERE created > datetime('now','localtime','-1 day');");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $temperature = number_format($row['tmin'], 1);
    echo "<tr>";
    echo "<td>Minimalna temperatura powietrza za ostatnie 24 godziny</td><td> <b>{$temperature} &#8451;</b></td>";
    echo "</tr>";

    $temperature = number_format($row['tmax'], 1);
    echo "<tr>";
    echo "<td>Maksymalna temperatura powietrza za ostatnie 24 godziny</td><td> <b>{$temperature} &#8451;</b></td>";
    echo "</tr>";
}
$result = $results->finalize();

echo "<tr>";
echo "<td>Ostatnia aktualizacja bazy danych </td><td> <b>{$created}</b></td>";
echo "<tr>";

echo "</table>";

echo "<br>";

echo "<table>";
echo "<col width=\"425\">";
echo "<col width=\"300\">";

$results = $db->query("SELECT (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-118 minute') ORDER BY created DESC LIMIT 1) - (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-118 minute') ORDER BY created ASC LIMIT 1) AS rain1h;");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $rain1h = number_format($row['rain1h'], 2);
    echo "<tr>";
    echo "<td>Opad atmosferyczny za ostatnią godzinę</td><td> <b>{$rain1h} mm</b></td>";
    echo "</tr>";
}
$result = $results->finalize();

$results = $db->query("SELECT (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 day') ORDER BY created DESC LIMIT 1) - (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 day') ORDER BY created ASC LIMIT 1) AS rain24h;");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $rain24h = number_format($row['rain24h'], 2);
    echo "<tr>";
    echo "<td>Opad atmosferyczny za ostatnie 24 godziny</td><td> <b>{$rain24h} mm</b></td> ";
    echo "</tr>";
}
$result = $results->finalize();

$results = $db->query("SELECT (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 day') ORDER BY created DESC LIMIT 1) - (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-7 day') ORDER BY created ASC LIMIT 1) AS rain7d;");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $rain7d = number_format($row['rain7d'], 2);
    echo "<tr>";
    echo "<td>Opad atmosferyczny za ostatnie 7 dni</td><td> <b>{$rain7d} mm</b></td> ";
    echo "</tr>";
}
$result = $results->finalize();

$results = $db->query("SELECT (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 month') ORDER BY created DESC LIMIT 1) - (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 month') ORDER BY created ASC LIMIT 1) AS rain30days;");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $rain30days = number_format($row['rain30days'], 2);
    echo "<tr>";
    echo "<td>Opad atmosferyczny za ostatnie 30 dni</td><td> <b>{$rain30days} mm</b></td> ";
    echo "</tr>";
}
$result = $results->finalize();

$results = $db->query("SELECT created FROM pluviometer ORDER BY created DESC LIMIT 1;");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    echo "<tr>";
    echo "<td>Ostatnia aktualizacja bazy danych</td><td> <b>{$row['created']}</b></td>";
    echo "<tr>";
}
$result = $results->finalize();

echo "</table>";

exec('gnuplot temperature24h.plt');

exec('gnuplot rain30d.plt');
?>
<br>
<img src="tmp/temperature24h.png" alt="temperature24h">
<br>
<br>
<img src="tmp/rain30d.png" alt="rain30d">
</body>
</html>




