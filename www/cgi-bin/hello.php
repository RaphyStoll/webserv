#!/usr/bin/php
<?php
// Simple PHP CGI Script for Demo
// Ensure this script is executable (chmod +x hello.php)

$name = "Guest";
if (isset($_GET['name']) && !empty($_GET['name'])) {
    $name = htmlspecialchars($_GET['name']);
}

// CGI Response headers
echo "Content-Type: text/plain\r\n";
echo "X-Powered-By: Webserv-CGI\r\n";
echo "\r\n"; // End of headers

// Body
echo "Hello, " . $name . "!\n";
echo "Welcome to the Webserv CGI Executable Environment.\n";
echo "Server Software: " . $_SERVER['SERVER_SOFTWARE'] . "\n";
echo "Current Time: " . date('Y-m-d H:i:s') . "\n";
?>
