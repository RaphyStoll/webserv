import socket

def test_raw(name, payload):
    print(f"Testing {name:25}", end=" ")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect(("localhost", 8080))
        s.sendall(payload.encode())
        res = s.recv(1024)
        s.close()
        print("✅ Répondu")
    except Exception as e:
        print(f"❌ CRASH ou TIMEOUT ({e})")

# 1. La requête vide (Juste une connexion qui se ferme)
test_raw("Empty Connection", "")

# 2. La requête avec des espaces n'importe où
test_raw("Spaces only", "      \r\n\r\n")

# 3. La méthode ultra longue (Buffer Overflow check)
test_raw("Huge Method", "A" * 10000 + " / HTTP/1.1\r\nHost: localhost\r\n\r\n")

# 4. Le chemin avec des caractères spéciaux
test_raw("Special Chars", "GET /../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n")

# 5. Header sans valeur
test_raw("Header no value", "GET / HTTP/1.1\r\nHost:\r\n\r\n")