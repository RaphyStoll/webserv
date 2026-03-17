🔍 Pourquoi ton port est occupé si webserv n’est pas là ?

Très fréquent sur les VPS :
Apache, Nginx ou un autre service écoute déjà sur 9001
Un ancien test serveur (pas nommé webserv)
Docker / container
Un autre étudiant avant toi 😄
Pour voir tout ce qui écoute sur tous les ports, fais :
sudo ss -lntup
Cherche la ligne avec 9001 dedans.
puis : kill -9 PID

🔧 Côté code (pour éviter ce problème)

Dans ton serveur, AVANT bind(), mets :
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
Ça t’évitera ce blocage à chaque redémarrage.