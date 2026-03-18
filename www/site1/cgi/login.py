#!/usr/bin/env python3
import sys
import os
import urllib.parse
import html

# --- 1) Parsing of incoming Data ---
method = os.environ.get('REQUEST_METHOD', 'GET')
data = ""

# Si c'est un POST, le body arrive via stdin
if method == 'POST':
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        if content_length > 0:
            data = sys.stdin.read(content_length)
    except ValueError:
        data = ""

# Parse les données encodées (ex: username=Bob&password=123)
parsed_data = urllib.parse.parse_qs(data)

# Extract username and password with safe fallbacks
username = html.escape(parsed_data.get('username', [''])[0])
password = html.escape(parsed_data.get('password', [''])[0])

# Vérification (Mock d'une base de donnée très simple)
is_submitted = method == 'POST'
is_success = False

if is_submitted:
    if username == "admin" and password == "admin":
        is_success = True

# --- 2) HTML Response Structure ---
print("Content-Type: text/html\r\n", end="")
print("X-Powered-By: Webserv-Python\r\n", end="")
print("\r\n", end="") # End of headers

# Theme colors
bg_color = "#f3f4f6"
card_bg = "#ffffff"
text_color = "#1f2937"
primary_color = "#4f46e5"
error_color = "#ef4444"
success_color = "#10b981"

print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Webserv Login CGI</title>
    <style>
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: {bg_color};
            color: {text_color};
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }}
        .login-card {{
            background: {card_bg};
            padding: 2.5rem;
            border-radius: 12px;
            box-shadow: 0 10px 25px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 380px;
        }}
        h2 {{
            margin-top: 0;
            color: {primary_color};
            text-align: center;
            margin-bottom: 30px;
            font-size: 1.8rem;
        }}
        .form-group {{
            margin-bottom: 20px;
        }}
        label {{
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            font-size: 0.9rem;
        }}
        input[type="text"], input[type="password"] {{
            width: 100%;
            padding: 10px 12px;
            border: 1px solid #d1d5db;
            border-radius: 6px;
            box-sizing: border-box;
            font-size: 1rem;
            transition: border-color 0.3s;
        }}
        input:focus {{
            outline: none;
            border-color: {primary_color};
            box-shadow: 0 0 0 3px rgba(79, 70, 229, 0.2);
        }}
        button {{
            width: 100%;
            background-color: {primary_color};
            color: white;
            border: none;
            padding: 12px;
            border-radius: 6px;
            font-size: 1rem;
            font-weight: bold;
            cursor: pointer;
            transition: background-color 0.3s;
            margin-top: 10px;
        }}
        button:hover {{
            background-color: #4338ca;
        }}
        .alert {{
            padding: 12px;
            border-radius: 6px;
            margin-bottom: 20px;
            text-align: center;
            font-weight: 500;
        }}
        .alert-error {{ background: #fef2f2; color: {error_color}; border: 1px solid #fecaca; }}
        .alert-success {{ background: #ecfdf5; color: {success_color}; border: 1px solid #a7f3d0; }}
        .footer {{
            margin-top: 30px;
            text-align: center;
            font-size: 0.8rem;
            color: #6b7280;
        }}
    </style>
</head>
<body>
    <div class="login-card">
        <h2>Webserv Login</h2>
""")

# --- 3) Conditionnal rendering based on Submission ---
if is_submitted:
    if is_success:
        print(f'<div class="alert alert-success">Welcome Back, {username}! Authentication Successful.</div>')
    else:
        print('<div class="alert alert-error">Invalid Username or Password!</div>')

if not is_success:
    print("""
        <form method="POST" action="">
            <div class="form-group">
                <label for="username">Username</label>
                <input type="text" id="username" name="username" placeholder="Enter 'admin'" required autocomplete="off">
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" placeholder="Enter 'admin'" required>
            </div>
            <button type="submit">Sign In</button>
        </form>
    """)
else:
    print("""
        <button onclick="window.location.href='?'" style="background:#10b981;">Log Out</button>
    """)

print(f"""
        <div class="footer">
            Raw POST Data received: <code style="background:#f3f4f6; padding:2px 4px;">{html.escape(data) if data else 'None'}</code>
        </div>
    </div>
</body>
</html>
""")
