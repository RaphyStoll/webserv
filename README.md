*This project has been created as part of the 42 curriculum by raphalme, sydubois, scuthber.*

<div align="center">
  <h1>🌐 Webserv</h1>
  <p><i>Un serveur HTTP écrit en C++98, inspiré de NGINX.</i></p>
  
  <p>
    <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++" />
    <img src="https://img.shields.io/badge/Makefile-8A2BE2?style=for-the-badge&logo=gnu-bash&logoColor=white" alt="Makefile" />
    <img src="https://img.shields.io/badge/42-000000?style=for-the-badge&logo=42&logoColor=white" alt="42" />
  </p>
</div>

## 📝 Description

Ce projet a pour but de développer un serveur HTTP en C++ (norme C++98) depuis zéro, en s'inspirant du fonctionnement de serveurs web tels que **NGINX**. L'objectif principal est de maîtriser le protocole HTTP, la programmation de sockets réseau (TCP) et le multiplexage d'entrées/sorties non bloquantes (I/O multiplexing). 

Le serveur permet de :
- ⚡ Gérer de multiples connexions simultanées
- 🔄 Traiter différentes méthodes HTTP (`GET`, `POST`, `DELETE`)
- ⚙️ Parser des fichiers de configuration complets
- 📜 Exécuter des scripts CGI

### ✨ Fonctionnalités (Features)
- **Multiplexage d'E/S non-bloquant** via `poll()`, permettant de maintenir de très nombreuses connexions réseau simultanément sur un seul thread.
- **Support des requêtes Chunked** (`Transfer-Encoding: chunked`).
- **Gestion optimisée de la mémoire** : les requêtes avec de gros *body* sont sauvegardées dans des fichiers temporaires pour empêcher la surcharge de la mémoire (RAM) du serveur.
- Gestion complète des méthodes `GET`, `POST` et `DELETE`.
- Routage complet des requêtes selon l'URI, détermination du bon dossier racine (`root`), vérification des droits (`Allowed Methods`), et indexation.
- Fichiers de configurations avancés inspirés de NGINX (ex: blocs `server` et `location`, upload directory, etc.).
- Support de multiples ports et multiples sites sur le même port (Virtual Hosting) basé sur le header `Host`.
- Exécution de scripts CGI (PHP, Python, etc.) via l'environnement adéquat et récupération de leurs sorties de façon asynchrone pour ne pas bloquer l'`EventLoop`.
- Gestion d'upload (enregistrement sécurisé) et de download de fichiers.
- Génération d'auto-index (pages HTML listant automatiquement le contenu d'un répertoire).
- Gestion des pages d'erreurs par défaut et personnalisées (404, 403, 500, etc.).
- Timeout management : Fermeture automatique des connexions clientes inactives pour éviter la saturation réseau.

### 📊 Comparatif : Outils de Multiplexage d'E/S (I/O)

Bien que `epoll` (Linux) et `kqueue` (MacOS/BSD) soient les standards de l'industrie pour les serveurs modernes, ce projet a été développé en utilisant `poll()` pour des raisons de compatibilité transversale (Posix).

| Outil | Complexité | Efficacité / Scalabilité | Compatibilité OS | Remarques |
|---|---|---|---|---|
| **`select`** | Faible | Décroît fortement avec le nb de FD : O(n) | Très élevée (POSIX) | Historique, complexe à scaler (limité généralement à 1024 FD). |
| **`poll`** 📍 | Faible | Décroît avec le nb de FD : O(n) | Très élevée (POSIX) | **Utilisé dans ce projet !** Plus flexible que `select` (pas de limite stricte sur le nombre de FD fixée par l'OS). |
| **`epoll`** | Modérée | Excellente : O(1) (événements actifs) | Uniquement Linux | NGINX et la plupart des serveurs modernes l'utilisent sous Linux. |
| **`kqueue`** | Élevée | Excellente : O(1) (événements actifs) | MacOS & BSD | Équivalent de `epoll` sur les systèmes BSD. Permet de monitorer bien d'autres choses que les sockets. |

### 👥 L'Équipe & Rôles
Ce projet a été réalisé en groupe de 3, avec la répartition suivante :
- **[scuthber] :** Parsing complet des requêtes HTTP, support des body *chunked*, mise en place d'une gestion de mémoire via fichiers temporaires (pour palier aux requêtes trop importantes) et création du système d'auto-indexation des dossiers.
- **[sydubois] :** Architecture des données de configuration, Parsing du fichier `.conf` et version 1 de l'implémentation des CGI.
- **[raphalme] :** Architecture globale, cœur réseau, multiplexing I/O, gestion de l'EventLoop, réponses HTTP, intégration de la `LIBFTPP`, attachement complet des *fichiers temporaires* inter-processus via le Client, et refactorisation/finalisation asynchrone des CGI.

### 📦 LIBFTPP
Ce projet repose sur notre propre bibliothèque statique, **LIBFTPP** (située dans le dossier `lib/LIBFTPP/`). À la manière de la *libft* du tronc commun de 42, il s'agit d'une collection d'utilitaires et de fonctions de base développée par **raphalme**, adaptée pour le C++98, afin de centraliser nos outils utiles (manipulations de string, classes utilitaires, etc.).

---

## 🚀 Instructions

### 🛠 Compilation

Pour compiler le projet, placez-vous à la racine du dépôt GitHub et exécutez la commande `make`. Cela va générer l'exécutable `Webserv`.

```bash
make
```

**Cibles générales :**
- `make` (ou `make all`) : Compile la `LIBFTPP` puis le serveur `Webserv`.
- `make clean` : Supprime les fichiers objets (`.o`, `.d`) du projet et de la librairie.
- `make fclean` : `clean` + supprime l'exécutable final et les binaires de test.
- `make re` : Ré-exécute les commandes `fclean` puis `make`.
- `make help` / `make help_fr` : Affiche l'aide détaillée de toutes les commandes disponibles.

### 💻 Exécution

Lancez le serveur en spécifiant votre fichier de configuration en argument.

```bash
./Webserv [chemin/vers/le/fichier.conf]
```

**Exemple d'utilisation :**
```bash
./Webserv config/config.conf
```

### 🧪 Tests et Débogage
Ces cibles permettent de valider le comportement du serveur et de vérifier les fuites mémoire :
- `make test` : Compile le binaire pour les tests unitaires (`test_request`).
- `make test-run` : Compile et lance automatiquement les tests unitaires.
- `make test-clean` : Supprime les fichiers générés par la compilation des tests.
- `make valgrind` : Lance le serveur avec Valgrind (les résultats sont générés dans `valgrind.log`).

### 📈 Tests de Charge (Siege)
Pour éprouver la robustesse du serveur HTTP :
- `make siege` : Lance un test standard (200 répétitions, 25 connexions concurrentes).
- `make siege_stress` : Test un benchmark intense pendant 3 minutes sans délai.
- `make siege_charge` : Test de charge réaliste (mode internet, avec des délais simulant les utilisateurs).

### 🐳 Docker (Environnement)
Des scripts pour lancer un environnement conteneurisé dédié aux tests :
- `make up` : Démarre les conteneurs Docker en arrière-plan (intègre une vérification pour `colima`).
- `make down` : Arrête proprement les conteneurs Docker.
- `make join` : Ouvre un terminal interactif à l'intérieur du conteneur `webserv_tester`.
- `make logs` : Affiche le flux des logs en direct.
- `make re-docker` : Redémarre complètement l'environnement.

### 🔄 Gestion de LIBFTPP (Git Subtree)
Gère le lien avec notre librairie externe *(réservé aux créateurs)* :
- `make pull-libftpp` : Met à jour la LIBFTPP locale avec les derniers changements de son dépôt distant.
- `make push-libftpp` : Pousse vos modifications locales sur la librairie directement sur la branche `main` du dépôt distant.
- `make pr-libftpp` : Demande un nom de branche, puis pousse vos modifications locales dessus pour pouvoir facilement ouvrir une Pull Request sur le dépôt distant.

---

## 📚 Ressources

### 📖 Ouvrages et documentations de référence :
- 📄 [RFC 9110 : HTTP Semantics](https://datatracker.ietf.org/doc/html/rfc9110)
- 📄 [RFC 9112 : HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc9112)
- 📄 [RFC 3875 : CGI Version 1.1](https://datatracker.ietf.org/doc/html/rfc3875)
- 🌐 [MDN Web Docs : HTTP Status Codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)
- 🖥️ [Documentation Officielle NGINX](https://nginx.org/en/docs/)

### 🤖 Utilisation de l'Intelligence Artificielle :
L'intelligence artificielle a été utilisée comme outil d'assistance et de support tout au long du projet :
- **Documentation et Rédaction :** Génération et mise en forme de ce fichier `README.md`.
- **Développement Web :** Création du frontend (HTML/CSS) pour nos différents sites de tests.
- **Fonctionnalités spécifiques :** Aide et pistes d'implémentation pour la logique d'exécution des scripts CGI.
- **Base de connaissances :** Support de réponses et d'explications sur diverses questions théoriques liées au C++98, aux protocoles réseau ou à des points d'architecture.
