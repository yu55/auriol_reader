<html>
<pre>
<?php

$file = popen("tac /home/pi/auriol_pluviometer_reader/output-auriol.log",'r');

while ($line = fgets($file)) {
  echo $line;
}

?>
<pre>
</html>
