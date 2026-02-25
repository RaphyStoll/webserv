#!/opt/homebrew/bin/php
<?php
// Script CGI pour lister les fichiers du dossier /upload
echo "Content-Type: application/json\r\n\r\n";

$uploadDir = realpath(__DIR__ . '/../upload');
if (!$uploadDir || !is_dir($uploadDir)) {
    echo json_encode(["error" => "Upload directory not found"]);
    exit;
}

$files = array_diff(scandir($uploadDir), array('.', '..'));
$result = [];

foreach ($files as $file) {
    if (is_file($uploadDir . '/' . $file)) {
        $result[] = [
            "name" => $file,
            "path" => "/upload/" . $file,
            "size" => filesize($uploadDir . '/' . $file)
        ];
    }
}

echo json_encode($result);
?>