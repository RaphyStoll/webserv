#!/usr/bin/env bash

set -u

prompt_default() {
  local message="$1"
  local default_value="$2"
  local input
  read -r -p "$message [$default_value]: " input
  if [[ -z "$input" ]]; then
    echo "$default_value"
  else
    echo "$input"
  fi
}

echo "=== Webserv tmp body test (2 clients) ==="

base_url="$(prompt_default "Base URL (host+port)" "http://127.0.0.1:2224")"
upload_route="$(prompt_default "Route upload" "/upload")"
rate="$(prompt_default "Debit limite par client (curl --limit-rate)" "40k")"
size_mb="$(prompt_default "Taille des fichiers en MB" "2")"
watch="$(prompt_default "Ouvrir un watcher /tmp dans une 2e fenetre Terminal ? (y/n)" "y")"

if ! [[ "$size_mb" =~ ^[0-9]+$ ]] || [[ "$size_mb" -lt 1 ]]; then
  echo "Erreur: la taille doit etre un entier >= 1"
  exit 1
fi

timestamp="$(date +%s)"
src1="/tmp/body1_${timestamp}.bin"
src2="/tmp/body2_${timestamp}.bin"
name1="client1_${timestamp}.bin"
name2="client2_${timestamp}.bin"

url1="${base_url}${upload_route}/${name1}"
url2="${base_url}${upload_route}/${name2}"

log1="/tmp/curl1_${timestamp}.log"
log2="/tmp/curl2_${timestamp}.log"

echo
echo "[1/5] Generation des payloads..."
dd if=/dev/urandom of="$src1" bs=1m count="$size_mb" status=none || exit 1
dd if=/dev/urandom of="$src2" bs=1m count="$size_mb" status=none || exit 1
echo "Payloads: $src1, $src2"

if [[ "$watch" == "y" || "$watch" == "Y" ]]; then
  echo "[2/5] Lancement watcher tmp dans une 2e fenetre..."
  if command -v osascript >/dev/null 2>&1; then
    osascript <<EOF >/dev/null 2>&1
tell application "Terminal"
  activate
  do script "bash -lc 'while true; do printf \"\\033[2J\\033[H\"; date \"+%H:%M:%S\"; find /tmp -maxdepth 1 -name \"webserv_body_*\" -exec ls -lh {} \\; 2>/dev/null; sleep 0.2; done'"
end tell
EOF
    echo "Watcher ouvert."
  else
    echo "osascript indisponible: watcher non lance automatiquement."
  fi
fi

echo "[3/5] Envoi de 2 clients en parallele..."
curl --http1.1 -sS -v --limit-rate "$rate" -X POST --data-binary @"$src1" \
  -H "Expect:" \
  "$url1" >| "$log1" 2>&1 &
pid1=$!

curl --http1.1 -sS -v --limit-rate "$rate" -X POST --data-binary @"$src2" \
  -H "Expect:" \
  "$url2" >| "$log2" 2>&1 &
pid2=$!

wait "$pid1"
ec1=$?
wait "$pid2"
ec2=$?

echo "curl1 exit=$ec1 log=$log1"
echo "curl2 exit=$ec2 log=$log2"

echo "[4/5] Verification des uploads..."
dst1="www/upload/$name1"
dst2="www/upload/$name2"

if [[ ! -f "$dst1" || ! -f "$dst2" ]]; then
  echo "Echec: fichiers upload absents."
  echo "- $dst1"
  echo "- $dst2"
  echo
  echo "Dernieres lignes logs curl:"
  tail -n 40 "$log1" 2>/dev/null || true
  tail -n 40 "$log2" 2>/dev/null || true
  exit 1
fi

echo "Fichiers presents:"
ls -lh "$dst1" "$dst2"

hash_src1="$(shasum "$src1" | awk '{print $1}')"
hash_dst1="$(shasum "$dst1" | awk '{print $1}')"
hash_src2="$(shasum "$src2" | awk '{print $1}')"
hash_dst2="$(shasum "$dst2" | awk '{print $1}')"

echo
echo "Hash 1 src=$hash_src1 dst=$hash_dst1"
echo "Hash 2 src=$hash_src2 dst=$hash_dst2"

echo "[5/5] Resultat"
if [[ "$hash_src1" == "$hash_dst1" && "$hash_src2" == "$hash_dst2" ]]; then
  echo "OK: uploads valides. Tmp body utilise si seuil depasse."
  echo "Astuce: regarde les logs curl ($log1, $log2) et la fenetre watcher."
  exit 0
fi

echo "KO: mismatch hash."
exit 1
