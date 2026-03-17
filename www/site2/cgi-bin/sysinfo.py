#!/usr/bin/env python3
import sys
import os
import platform
from datetime import datetime

# Headers
print("Content-Type: text/html\r\n\r\n", end="")

# Body HTML
html = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Python CGI Bonus</title>
    <style>
        body {{ font-family: sans-serif; background: #0d1117; color: #c9d1d9; padding: 20px; }}
        .box {{ border: 1px solid #30363d; background: #161b22; padding: 20px; border-radius: 8px; }}
        h1 {{ color: #58a6ff; }}
        span {{ color: #2ea043; font-weight: bold; }}
    </style>
</head>
<body>
    <div class="box">
        <h1>🐍 Python CGI Executed</h1>
        <p>This page was generated dynamically by a Python script through the Webserv CGI protocol.</p>
        <hr style="border-color:#30363d;">
        <ul>
            <li><strong>Current Time:</strong> <span>{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</span></li>
            <li><strong>Python Version:</strong> <span>{platform.python_version()}</span></li>
            <li><strong>Operating System:</strong> <span>{platform.system()} {platform.release()}</span></li>
            <li><strong>Machine Arch:</strong> <span>{platform.machine()}</span></li>
        </ul>
        <p style="margin-top:20px; color:#8b949e; font-size:12px;">Query String received: {os.environ.get('QUERY_STRING', 'None')}</p>
    </div>
</body>
</html>
"""

print(html)
