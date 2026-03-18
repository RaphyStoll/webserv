#!/usr/bin/env php
<?php
echo "Content-Type: text/plain\r\n\r\n";

echo "[INFO] You can add ?lines=X at the end of the URL to display the last X lines (e.g., /get_logs.php?lines=50)\n";
echo str_repeat("-", 80) . "\n\n";

$numLines = 20;

if (empty($_GET) && getenv('QUERY_STRING')) {
    parse_str(getenv('QUERY_STRING'), $_GET);
}

if (isset($_GET['lines']) && is_numeric($_GET['lines'])) {
    $numLines = (int)$_GET['lines'];
    if ($numLines < 1) $numLines = 1;
    if ($numLines > 5000) $numLines = 5000;
}

$logFile = realpath(__DIR__ . '/../../../log/general.log');

if (!$logFile || !file_exists($logFile)) {
    echo "[SYSTEM] No log trace found in /log/general.log\n";
    exit;
}
$command = "tail -n " . escapeshellarg($numLines) . " " . escapeshellarg($logFile) . " 2>&1";
$output = shell_exec($command);

if ($output === null) {
    echo "[ERROR] Unable to read the log file.\n";
} else {
    echo $output;
}
?>