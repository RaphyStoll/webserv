#!/usr/bin/env php
<?php
// Script CGI pour lister les fichiers du dossier /upload
echo "Content-Type: application/json\r\n";
echo "X-Powered-By: Webserv-CGI\r\n";
echo "\r\n";

$uploadDir = realpath(__DIR__ . '/../upload');
if (!$uploadDir || !is_dir($uploadDir)) {
    echo json_encode(["error" => "Upload directory not found"]);
    exit;
}

$result = [];

// Utilisation de FilesystemIterator, bien plus performant que scandir()
// Il ignore automatiquement '.' et '..'
$iterator = new FilesystemIterator($uploadDir, FilesystemIterator::SKIP_DOTS);

foreach ($iterator as $fileInfo) {
    if ($fileInfo->isFile()) {
        $filename = $fileInfo->getFilename();
        
        // Ignorer exclusivement les fichiers gitkeep / getkeep
        if ($filename === '.gitkeep' || $filename === '.getkeep') {
            continue;
        }

        $result[] = [
            "name" => $filename,
            "path" => "/upload/" . $filename,
            "size" => $fileInfo->getSize()
        ];
    }
}

echo json_encode($result);
?>