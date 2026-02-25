#!/opt/homebrew/bin/php
<?php
// Script CGI pour récupérer les X dernières lignes de logs
echo "Content-Type: text/plain\r\n\r\n";

$logFile = realpath(__DIR__ . '/../../log/general.log');

if (!$logFile || !file_exists($logFile)) {
    echo "[SYSTEM] Aucune trace de log trouvée dans /log/general.log\n";
    exit;
}

// On lit les 20 dernières lignes du fichier de logs (simili `tail -n 20`)
$lines = file($logFile);
if ($lines === false) {
    echo "[ERROR] Impossible de lire le fichier de logs.\n";
    exit;
}

$last_lines = array_slice($lines, -20);
foreach ($last_lines as $line) {
    echo $line;
}
?>