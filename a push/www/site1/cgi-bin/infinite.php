#!/usr/bin/env php
<?php
// Trick for PHP CLI to parse QUERY_STRING manually
if (empty($_GET) && getenv('QUERY_STRING')) {
    parse_str(getenv('QUERY_STRING'), $_GET);
}

$isInfinite = true;
$durationSeconds = 0;

// Parse time parameter (e.g., 10s, 2m, 1h or just 15)
if (isset($_GET['time'])) {
    $timeParam = strtolower(trim($_GET['time']));
    if (preg_match('/^(\d+)(s|m|h)$/', $timeParam, $matches)) {
        $isInfinite = false;
        $val = (int)$matches[1];
        $unit = $matches[2];
        if ($unit === 'h') {
            $durationSeconds = $val * 3600;
        } elseif ($unit === 'm') {
            $durationSeconds = $val * 60;
        } else {
            $durationSeconds = $val; // 's'
        }
    } elseif (is_numeric($timeParam)) {
        $isInfinite = false;
        $durationSeconds = (int)$timeParam; // Defaults to seconds if no unit is provided
    }
}

// CGI Headers
echo "Content-Type: text/plain; charset=utf-8\r\n";
echo "X-Content-Type-Options: nosniff\r\n";
echo "Cache-Control: no-cache\r\n\r\n";

// Ensure PHP streams output immediately
@ob_end_flush();
ob_implicit_flush(true);

echo "--- CGI NON-BLOCKING DEMONSTRATION ---\n";
echo "This script demonstrates that the WebServer handles CGIs asynchronously.\n";
if ($isInfinite) {
    echo "[INFO] Running infinitely! (You can add ?time=10s, ?time=2m or ?time=1h to limit it)\n";
} else {
    echo "[INFO] Running for $durationSeconds seconds.\n";
}
echo "Open another tab on your server to verify it is still completely responsive!\n";
echo str_repeat("-", 60) . "\n\n";

$i = 1;
while ($isInfinite || $i <= $durationSeconds) {
    $msg = "Second $i : Script is actively running in the background...\n";
    echo str_pad($msg, 1024, " "); // Chrome buffers ~1024 bytes before initial render
    flush(); 
    sleep(1);
    $i++;
}

echo "\n[SUCCESS] Script execution completed gracefully!\n";
?>
