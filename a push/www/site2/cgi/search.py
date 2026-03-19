#!/usr/bin/env python3
import os
import urllib.parse
import html

# --- RECUPERATION DES VARIABLES D'ENV ---
method = os.environ.get("REQUEST_METHOD", "Unknown")
query_raw = os.environ.get("QUERY_STRING", "")
path_info = os.environ.get("PATH_INFO", "")
server = os.environ.get("SERVER_NAME", "Unknown")
port = os.environ.get("SERVER_PORT", "Unknown")
contentType = os.environ.get("CONTENT_TYPE", "")
contentLength = os.environ.get("CONTENT_LENGTH", "0") # Correction coquille Length
script = os.environ.get("SCRIPT_NAME", "Unknown")
protocol = os.environ.get("SERVER_PROTOCOL", "Unknown")
software = os.environ.get("SERVER_SOFTWARE", "Unknown")
interface = os.environ.get("GATEWAY_INTERFACE", "Unknown")

# Parsing propre de la query_string pour extraire le terme cherché
parsed_query = urllib.parse.parse_qs(query_raw)
search_term = parsed_query.get('q', [''])[0]

# --- EN-TETES HTTP ---
print("Content-Type: text/html\r\n", end="")
print("X-Powered-By: Webserv-Python\r\n", end="")
print("\r\n", end="")

# --- DESIGN & COULEURS ---
primary_color = "#3b82f6"
bg_color = "#e5e7eb"
card_bg = "#ffffff"

print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Webserv Search Engine</title>
    <style>
        body {{
            font-family: 'Inter', system-ui, -apple-system, sans-serif;
            background-color: {bg_color};
            margin: 0;
            padding: 40px 20px;
            color: #374151;
            display: flex;
            flex-direction: column;
            align-items: center;
        }}
        .header {{
            text-align: center;
            margin-bottom: 40px;
        }}
        .header h1 {{
            font-size: 2.5rem;
            color: #111827;
            margin: 0;
        }}
        .search-box {{
            width: 100%;
            max-width: 600px;
            display: flex;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
            border-radius: 9999px;
            overflow: hidden;
            margin-bottom: 40px;
        }}
        .search-box input {{
            flex: 1;
            padding: 15px 25px;
            font-size: 1.1rem;
            border: none;
            outline: none;
        }}
        .search-box button {{
            background-color: {primary_color};
            color: white;
            border: none;
            padding: 0 30px;
            font-size: 1.1rem;
            font-weight: 600;
            cursor: pointer;
            transition: background-color 0.2s;
        }}
        .search-box button:hover {{
            background-color: #2563eb;
        }}
        .results-container {{
            width: 100%;
            max-width: 800px;
            background: {card_bg};
            border-radius: 12px;
            padding: 30px;
            box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
        }}
        .results-header {{
            font-size: 1.2rem;
            border-bottom: 2px solid #f3f4f6;
            padding-bottom: 15px;
            margin-bottom: 20px;
            margin-top: 0;
        }}
        .results-header span {{
            color: {primary_color};
            font-weight: bold;
        }}
        .env-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 15px;
        }}
        .env-item {{
            background: #f9fafb;
            padding: 12px;
            border-radius: 8px;
            border-left: 4px solid {primary_color};
        }}
        .env-key {{
            font-size: 0.75rem;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            color: #6b7280;
            margin-bottom: 4px;
        }}
        .env-value {{
            font-size: 1rem;
            font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
            word-break: break-all;
            color: #111827;
        }}
        .empty-state {{
            text-align: center;
            color: #6b7280;
            padding: 40px 0;
            font-style: italic;
        }}
    </style>
</head>
<body>

    <div class="header">
        <h1>🔍 Webserv Search</h1>
        <p>A Python CGI Script Demonstration</p>
    </div>

    <form class="search-box" method="GET" action="">
        <input type="text" name="q" placeholder="Rechercher quelque chose..." value="{html.escape(search_term)}" autocomplete="off" autofocus>
        <button type="submit">Search</button>
    </form>
""")

if search_term:
    search_dir = "." # Racine du repo
    matched_files = []
    matched_dirs = []
    
    # Recherche récursive dans l'arborescence (sur les noms)
    if os.path.exists(search_dir):
        for root, dirs, files in os.walk(search_dir):
            # On ignore les dossiers .git et obj pour ne pas polluer les résultats
            if '.git' in dirs:
                dirs.remove('.git')
            if 'obj' in dirs:
                dirs.remove('obj')

            # Recherche dans les dossiers
            for d in dirs:
                if search_term.lower() in d.lower():
                    matched_dirs.append(os.path.join(root, d))
                    
            # Recherche dans les fichiers
            for f in files:
                if search_term.lower() in f.lower():
                    matched_files.append(os.path.join(root, f))
    
    total_found = len(matched_files) + len(matched_dirs)
    
    # Choix de la couleur selon le résultat
    if total_found > 0:
        banner_color = "#10b981"  # Vert
        banner_title = f"{total_found} élément(s) trouvé(s) ! ({len(matched_files)} fichier(s), {len(matched_dirs)} dossier(s))"
    else:
        banner_color = "#ef4444"  # Rouge
        banner_title = "Aucun élément trouvé..."

    print(f"""
    <div class="results-container" style="margin-bottom: 30px; border-left: 5px solid {banner_color};">
        <h2 class="results-header" style="color: {banner_color}; border-bottom-color: {banner_color}40;">
            {banner_title}
        </h2>
        <div style="padding: 10px 0;">
            <h3 style="margin:0 0 10px 0; color:#1f2937;">Résultats pour le nom "<span>{html.escape(search_term)}</span>" :</h3>
    """)
    
    if total_found > 0:
        print("<ul style='color:#4b5563; line-height: 1.6; padding-left: 20px; word-break: break-all;'>")
        # Affichage des dossiers en premier
        for dpath in matched_dirs:
            print(f"<li>📁 <strong>{html.escape(dpath)}</strong></li>")
        # Puis les fichiers
        for fpath in matched_files:
            print(f"<li>📄 <strong>{html.escape(fpath)}</strong></li>")
        print("</ul>")
    else:
        print(f"<p style='margin:0; color:#6b7280;'>Aucun fichier ni dossier ne contient '{html.escape(search_term)}' dans son nom dans le répertoire racine.</p>")

    print("""
        </div>
    </div>
    """)

print("""
    <div class="results-container">
        <h2 class="results-header">🔧 CGI Environment Variables (Debug)</h2>
        <div class="env-grid">
""")

# Liste des variables à afficher avec leurs données
variables = [
    ("REQUEST_METHOD", method),
    ("QUERY_STRING", query_raw if query_raw else "[Empty]"),
    ("PATH_INFO", path_info if path_info else "[Empty]"),
    ("SCRIPT_NAME", script),
    ("SERVER_NAME", server),
    ("SERVER_PORT", port),
    ("SERVER_PROTOCOL", protocol),
    ("SERVER_SOFTWARE", software),
    ("GATEWAY_INTERFACE", interface),
    ("CONTENT_TYPE", contentType if contentType else "[Empty]"),
    ("CONTENT_LENGTH", contentLength if contentLength else "0")
]

for key, val in variables:
    print(f"""
            <div class="env-item">
                <div class="env-key">{key}</div>
                <div class="env-value">{html.escape(str(val))}</div>
            </div>
    """)

print("""
        </div>
    </div>

    <script>
        console.log("CGI Search Executed.");
        console.log("Method:", "{method}");
        console.log("Search term:", "{html.escape(search_term)}");
    </script>
</body>
</html>
""")
