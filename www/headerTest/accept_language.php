<?php
	$accept_language = false;
	if (isset($_SERVER["HTTP_ACCEPT_LANGUAGE"]))
		$accept_language = $_SERVER["HTTP_ACCEPT_LANGUAGE"];
	$en = strpos($accept_language, 'en');
	$fr = strpos($accept_language, 'fr');
	if (is_bool($en))
		$en = 100;
	if (is_bool($fr))
		$fr = 100;

	$print_language;
	if ($en < $fr) {
		$print_language = "en";
		header("Content-Language: en");
	}
	else if ($fr < $en) {
		$print_language = "fr";
		header("Content-Language: fr");
	}
	else {
		$print_language = "en";
		header("Content-Language: en");
	}
?>


<!DOCTYPE html>
<html lang=<?=$print_language?>>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Accept-Language</title>
</head>
<body>

<div style="background:#ffeb3b61">
	REQUEST : <?=$_SERVER["REQUEST_METHOD"]?> <?=$_SERVER["PATH_INFO"]?> <?=$_SERVER["SERVER_PROTOCOL"]?>
	<ul>
	<?php
	if (isset($_SERVER["HTTP_ACCEPT_LANGUAGE"])) {
	?>
		<li>Accept-Language : <?=$_SERVER["HTTP_ACCEPT_LANGUAGE"]?></li>
	<?php
	}
	?>
		<li>Content-Language : <?=$print_language?></li>
	</ul>
</div>

<div>
	<p>Content</p>
	<div>
	<?php
	if ($print_language == "en") {
	?>
		I speak english.
	<?php
	} else if ($print_language == "fr") {
	?>
		Je parle français.
	<?php
	} 
	?>
	</div>
</div>

</body>
</html>



