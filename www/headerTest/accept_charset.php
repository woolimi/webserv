<?php
	$accept_charset = false;
	if (isset($_SERVER["HTTP_ACCEPT_CHARSET"]))
		$accept_charset = $_SERVER["HTTP_ACCEPT_CHARSET"];
	if (strpos($accept_charset, '*') === false && $res = strpos($accept_charset, 'UTF-8') === false)
	{
		header("Status: 406 Not Acceptable", false, 406);
		echo "406 Charset Not Acceptable\n";
		echo "<a href=\"https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Accept-Charset\">reference</a>\n";
		exit();
	}
?>

<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Accept-Charset</title>
</head>
<body>

<div style="background:#ffeb3b61">
	REQUEST : <?=$_SERVER["REQUEST_METHOD"]?> <?=$_SERVER["PATH_INFO"]?> <?=$_SERVER["SERVER_PROTOCOL"]?>
</div>

<div>
	<p>Content</p>
	<div>
		Accept-Charset UTF-8
	</div>
</div>

</body>
</html>



