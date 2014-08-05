<html>
<pre>
<?php

$file = popen("tac /home/pi/repositories/auriol_reader/output-auriol.log",'r');

while ($line = fgets($file)) {
  echo $line;
}

?>
<pre>
</html>
