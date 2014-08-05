<html>
<pre>
<?php

$file = popen("tac /home/pi/workspace/temperature.txt",'r');

while ($line = fgets($file)) {
  echo $line;
}

?>
<pre>
</html>
