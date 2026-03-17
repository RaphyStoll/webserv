#!/usr/bin/env php
<?php
// Script CGI infini pour démontrer le blocage du serveur !
echo "Content-Type: text/plain\r\n\r\n";

echo "--- DÉBUT DE LA DÉMO DU GOULET D'ÉTRANGLEMENT (CGI SYNCHRONE) ---\n";
echo "Je suis un script qui tourne à l'infini et fait 'sleep' chaque seconde.\n";
echo "Essayez d'ouvrir un autre onglet sur http://localhost:2224/ pour tester le non blockant\n\n";

// Vider le buffer (flush) pour forcer l'envoi au pipe
@ob_flush();
flush();

$i = 1;
while (true) {
    echo "Seconde $i : Je tourne toujours, et je bloque le poll() !\n";
    @ob_flush();
    flush();
    sleep(1);
    $i++;
}
