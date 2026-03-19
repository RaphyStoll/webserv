#!/usr/bin/env php
<?php
// Simple PHP CGI Script for Demo
// Ensure this script is executable (chmod +x hello.php)

// Astuce pour PHP CLI: Parser la QUERY_STRING manuellement
if (empty($_GET) && getenv('QUERY_STRING')) {
    parse_str(getenv('QUERY_STRING'), $_GET);
}

$name = "World"; // Default value
if (isset($_GET['name']) && !empty($_GET['name'])) {
    $name = stripslashes(htmlspecialchars($_GET['name']));
}

$serverSoft = getenv('SERVER_SOFTWARE') ? htmlspecialchars(getenv('SERVER_SOFTWARE')) : 'Webserv/1.0';

// CGI Response headers
echo "Content-Type: text/html\r\n";
echo "X-Powered-By: Webserv-CGI\r\n";
echo "\r\n"; // End of headers

// Body (HTML)
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hello CGI</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            padding: 3rem;
            border-radius: 15px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
            text-align: center;
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: transform 0.3s ease;
        }
        .container:hover {
            transform: translateY(-5px);
        }
        h1 {
            font-size: 3rem;
            margin-bottom: 5px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.2);
        }
        h1 span {
            color: #ffde59;
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0% { text-shadow: 0 0 10px #ffde59; }
            50% { text-shadow: 0 0 20px #ffde59, 0 0 30px #ffde59; }
            100% { text-shadow: 0 0 10px #ffde59; }
        }
        p {
            font-size: 1.2rem;
            opacity: 0.9;
            margin-bottom: 30px;
        }
        .info {
            font-size: 0.85rem;
            padding-top: 1.5rem;
            border-top: 1px solid rgba(255, 255, 255, 0.2);
            color: rgba(255, 255, 255, 0.7);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Hello <span><?php echo $name; ?></span>!</h1>
        <p>Welcome to the Webserv CGI.<br>use IP:PORT?name=XXX</p>

        <div class="info">
            Server Software: <b><?php echo $serverSoft; ?></b><br>
            Current Time: <b><?php echo date('Y-m-d H:i:s'); ?></b>
        </div>
    </div>
</body>
</html>