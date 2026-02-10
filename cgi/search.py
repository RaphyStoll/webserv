#!/usr/bin/env python3
import os

query = os.environ.get("QUERY_STRING", "")

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Résultat de recherche</h1>")
print(f"<p>Tu as cherché : {query}</p>")
print("</body></html>")
