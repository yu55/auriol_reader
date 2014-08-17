<html>
    <head>
        <title>Sportowa8 Meteo</title>
        <meta charset="utf-8">
    </head>

<?php
$db = new SQLite3('database.sl3');

echo "<table>";
echo "<col width=\"500\">";
echo "<col width=\"300\">";

$results = $db->query("SELECT created, temperature FROM anemometer ORDER BY created DESC LIMIT 1;");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $temperature = number_format($row['temperature'], 2);
    echo "<tr>";
    echo "<td>Temperatura</td><td> <b>{$temperature} &#8451;</b></td>";
    echo "</tr>";

    echo "Ostatni zapis w bazie o temperaturze wykonano: <b>{$row['created']}</b>. Zapisywanie do bazy odbywa się nie rzadziej niż co godzinę.";

}
$result = $results->finalize();

$results = $db->query("SELECT min(temperature) AS temperature FROM anemometer WHERE created > datetime('now','localtime','-1 day');");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $temperature = number_format($row['temperature'], 2);
    echo "<tr>";
    echo "<td>Temperatura minimalna za ostatnie 24 godziny</td><td> <b>{$temperature} &#8451;</b></td>";
    echo "</tr>";
}
$result = $results->finalize();

$results = $db->query("SELECT max(temperature) AS temperature FROM anemometer WHERE created > datetime('now','localtime','-1 day');");
while ($row = $results->fetchArray(SQLITE3_ASSOC)) {

    $temperature = number_format($row['temperature'], 2);
    echo "<tr>";
    echo "<td>Temperatura maksymalna za ostatnie 24 godziny</td><td> <b>{$temperature} &#8451;</b></td>";
    echo "</tr>";
}
$result = $results->finalize();


echo "</table>";

echo "<br>";

echo "<table>";
echo "<col width=\"500\">";
echo "<col width=\"300\">";

$results = $db->query("SELECT (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 hour') ORDER BY created DESC LIMIT 1) - (SELECT amount FROM pluviometer WHERE created > datetime('now','localtime','-1 hour') ORDER BY created ASC LIMIT 1) AS rain1h;");
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

    echo "Ostatni zapis w bazie o opadach wykonano: <b>{$row['created']}</b>. Zapisywanie do bazy odbywa się nie rzadziej niż co godzinę.";
}
$result = $results->finalize();

echo "</table>";
?>

</html>




