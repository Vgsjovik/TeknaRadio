#!/bin/bash
# Dobbeltklikk for å starte hub-nettsiden. Lukk vinduet eller Ctrl-C for å stoppe.
cd "$(dirname "$0")"

PY="$(command -v python3 || true)"
if [ -z "$PY" ]; then
  osascript -e 'display alert "Python 3 mangler" message "Installer python3 (f.eks. brew install python) og prøv igjen."'
  exit 1
fi

# Drep eventuell gammel server-prosess så vi alltid starter fersk
pkill -f "$(pwd)/server.py" 2>/dev/null || true
sleep 0.5

# Engangsoppsett: virtuelt miljø + pyserial
if [ ! -d ".venv" ]; then
  echo "Setter opp (engangs) – installerer pyserial …"
  "$PY" -m venv .venv
  ./.venv/bin/pip install --quiet --upgrade pip pyserial
fi

echo "Starter TeknaRadio hub-nettside på http://localhost:8765 …"
( sleep 1.5; open "http://localhost:8765" ) &
exec ./.venv/bin/python server.py "$@"
