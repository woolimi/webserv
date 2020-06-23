<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Accept-Language</title>
</head>
<body>

<?php
	$accept_language = false;
	if (array_key_exists("HTTP_ACCEPT_LANGUAGE", $_SERVER))
		$accept_language = $_SERVER["HTTP_ACCEPT_LANGUAGE"];
?>

<div style="background:#ffeb3b61">
	REQUEST : <?=$_SERVER["REQUEST_METHOD"]?> <?=$_SERVER["PATH_INFO"]?> <?=$_SERVER["SERVER_PROTOCOL"]?>
	<ul>
		<li>Accept-Language : <?=$accept_language?></li>
	</ul>
</div>

<div>
	<p>Content</p>
	<div>
	<?php
		$en = strpos($accept_language, 'en');
		$fr = strpos($accept_language, 'fr');
		if (is_bool($en))
			$en = 100;
		if (is_bool($fr))
			$fr = 100;
		if ($en < $fr) {
	?>
		I speak english.
	<?php
		} else if ($fr < $en) {
	?>
		Je parle français.
	<?php
		} else if ($en === $fr){
	?>
		I don't know which language I should speak to you.
	<?php
		}
	?>
	</div>
</div>

</body>
</html>



