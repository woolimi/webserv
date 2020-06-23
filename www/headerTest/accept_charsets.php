<?php
	$accept_charset = false;
	if (array_key_exists("HTTP_ACCEPT_CHARSETS", $_SERVER))
		$accept_charset = $_SERVER["HTTP_ACCEPT_CHARSETS"];

	if ($accept_charset === "EUR-KR") {
		header("Status: 301 Moved Permanently", false, 301);
		header("Location: /euc_kr_page.html");
		die();
	} else if ($accept_charset === "UTF-8") {
		header("Status: 301 Moved Permanently", false, 301);
		header('Location: /utf_8_page.html');
		die();
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
		Please set Accept-Charset UTF-8 or EUR-KR
	</div>
</div>

</body>
</html>



