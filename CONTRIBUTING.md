# Guide de Contribution pour les Assistants IA (Projet Webserv)

Ce document contient les règles strictes et le contexte du projet `webserv` (serveur HTTP en C++98). **En tant qu'assistant IA, tu dois impérativement lire et respecter ces consignes avant de proposer ou de modifier du code.**

## 1. Contraintes Globales et Langage
- **Standard strict :** Le projet doit compiler et fonctionner exclusivement en **C++98**.
  - ❌ INTERDIT : `auto`, `nullptr`, `std::unique_ptr`, `std::shared_ptr`, `std::thread`, `std::mutex`, les boucles *range-based* (`for (auto& x : y)`), les *lambdas*, etc.
  - ✅ AUTORISÉ : Itérateurs classiques, pointeurs bruts, `NULL`, foncteurs.
- **Compilation :** `c++ -Wall -Wextra -Werror -std=c++98`. Le code ne doit générer aucun warning.
- **Stabilité absolue :** Le programme ne doit **jamais** crasher (segfault, bus error, exception non catchée, out of memory).
- **Bibliothèques externes :** Totalement interdites (pas de Boost, pas de lib externe).
- **C vs C++ :** Préférer les versions C++ des bibliothèques C (ex: `<cstring>` au lieu de `<string.h>`).

## 2. Fonctions Autorisées (Liste Exhaustive)
Tu ne dois utiliser **que** les fonctions suivantes pour les appels système et réseau. Toute autre fonction est éliminatoire :
`execve`, `pipe`, `strerror`, `gai_strerror`, `errno`, `dup`, `dup2`, `fork`, `socketpair`, `htons`, `htonl`, `ntohs`, `ntohl`, `select`, `poll`, `epoll` (`epoll_create`, `epoll_ctl`, `epoll_wait`), `kqueue` (`kqueue`, `kevent`), `socket`, `accept`, `listen`, `send`, `recv`, `chdir`, `bind`, `connect`, `getaddrinfo`, `freeaddrinfo`, `setsockopt`, `getsockname`, `getprotobyname`, `fcntl`, `close`, `read`, `write`, `waitpid`, `kill`, `signal`, `access`, `stat`, `open`, `opendir`, `readdir`, `closedir`.

## 3. Architecture Réseau et I/O (Le Cœur du Sujet)
- **Multiplexage unique :** Le serveur doit utiliser **un seul** appel à `poll()` (ou équivalent comme `epoll`, `kqueue`, `select`) pour gérer **toutes** les opérations I/O entre les clients et le serveur (y compris le `listen`).
- **Non-bloquant :** Tous les sockets et I/O réseau doivent être non-bloquants.
- **Règle d'or des I/O :** Ne **jamais** faire de `read`/`recv` ou `write`/`send` sans que le multiplexeur (`poll`/`epoll`/`kqueue`) n'ait notifié que le descripteur est prêt.
- **Fichiers réguliers :** Les fichiers sur le disque n'ont pas besoin de passer par `poll()`.
- **Spécificité macOS (`fcntl`) :** L'utilisation de `fcntl()` est restreinte aux flags suivants : `F_SETFL`, `O_NONBLOCK`, et `FD_CLOEXEC`. Tout autre flag est interdit.
- **Interdiction de `fork` :** `fork` ne doit être utilisé **que** pour l'exécution des CGI. Interdit de forker le serveur lui-même.
- codant sur mac m3 mais le projet etant destiner a ubuntu 22 garantir une conpatibiliter entre ces 2 os

## 4. Fonctionnalités Requises
- **Méthodes HTTP :** Implémenter au minimum `GET`, `POST`, et `DELETE`.
- **Fichier de configuration :** Inspiré de NGINX. Doit gérer :
  - Les couples `interface:port`.
  - Les pages d'erreur par défaut.
  - La taille maximale du corps de la requête client.
  - Les routes (méthodes acceptées, redirections, répertoire racine, listing de dossier, fichier par défaut, upload de fichiers, exécution de CGI par extension).
- **CGI (Common Gateway Interface) :**
  - L'environnement complet de la requête doit être passé au CGI.
  - Les requêtes *chunked* doivent être dé-chunkées avant d'être envoyées au CGI (le CGI s'attend à un EOF).
  - Si le CGI ne renvoie pas de `Content-Length`, la fin des données est marquée par un EOF.
  - Le CGI doit s'exécuter dans le bon répertoire (pour les chemins relatifs).
- **Upload :** Les clients doivent pouvoir uploader des fichiers.

## 5. Instructions Spécifiques pour l'IA lors de la génération de code
1. **Vérifie la norme :** Avant de proposer un bout de code, assure-toi qu'il est 100% compatible C++98.
2. **Pas de fuites mémoire :** Gère rigoureusement les allocations dynamiques (`new`/`delete`).
3. **Gestion des erreurs :** Ne vérifie pas `errno` pour ajuster le comportement *après* un `read` ou `write` (c'est explicitement interdit par le sujet). Gère les retours de fonctions correctement.
4. **Robustesse :** Ajoute des blocs `try/catch` appropriés pour éviter que le serveur ne s'arrête inopinément.
5. **Contexte du projet :** Le projet est structuré avec des dossiers `src/`, `include/`, `config/`, `www/`, etc. Respecte cette architecture lors de la création de nouveaux fichiers.
