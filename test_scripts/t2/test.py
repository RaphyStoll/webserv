import http.client
import sys

def run_test(name, method, path, expected_status, port=8080):
    try:
        conn = http.client.HTTPConnection("localhost", port, timeout=5)
        conn.request(method, path)
        response = conn.getresponse()
        
        status = response.status
        content = response.read()
        
        if status == expected_status:
            print(f"✅ {name}: OK (Status {status})")
        else:
            print(f"❌ {name}: FAILED (Expected {expected_status}, got {status})")
        conn.close()
    except Exception as e:
        print(f"💥 {name}: ERROR - {e}")

if __name__ == "__main__":
    print("🚀 Démarrage des tests Webserv sur localhost:8080...\n")
    
    # --- TESTS DE BASE ---
    run_test("GET Simple", "GET", "/", 200)
    run_test("GET Page inexistante", "GET", "/page_qui_n_existe_pas", 404)
    
    # --- TESTS DE MÉTHODES ---
    run_test("DELETE (si non autorisé/existant)", "DELETE", "/private.php", 405) # Ou 404/204 selon votre config
    
    # --- TESTS DE SÉCURITÉ / SYNTAXE ---
    run_test("Requête malformée", "INVALID_METHOD", "/", 400) # Ou 405

    print("\n💡 Si les tests échouent, vérifiez que votre serveur écoute sur le port 8080.")
    print("👉 Ouvrez : http://localhost:8080 dans votre navigateur pour un test manuel.")