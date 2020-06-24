<?php
if (!isset($_SERVER['HTTP_AUTHORIZATION'])) {
    header('WWW-Authenticate: Basic realm="My Realm"');
	header("Status: 401 Unauthorized", false, 401);
    echo 'You canceled access !';
    exit;
} else if ($_SERVER['PHP_AUTH_USER'] !== "admin" && $_SERVER['PHP_AUTH_PW'] != "1234")
{
    header('WWW-Authenticate: Basic realm="My Realm"');
	header("Status: 401 Unauthorized", false, 401);
    echo 'You cannot access';
    exit;
} else {
    echo "<p>Hello \"{$_SERVER['PHP_AUTH_USER']}\".</p>";
    echo "<p>You entered \"{$_SERVER['PHP_AUTH_PW']}\" as your password.</p>";
}
?>