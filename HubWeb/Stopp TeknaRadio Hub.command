#!/bin/bash
# Stopper hub-serveren som kjører i bakgrunnen (startet via app-ikonet).
cd "$(dirname "$0")"
if [ -f ".hub.pid" ]; then
  PID="$(cat .hub.pid)"
  if kill "$PID" 2>/dev/null; then
    echo "Stoppet hub-server (pid $PID)."
  else
    echo "Fant ingen kjørende prosess (pid $PID). Rydder opp."
  fi
  rm -f .hub.pid
else
  # Fallback: finn server.py i denne mappa
  pkill -f "$(pwd)/server.py" 2>/dev/null && echo "Stoppet." || echo "Ingen kjørende server funnet."
fi
sleep 1
