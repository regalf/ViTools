#!/bin/bash

# ViStudio IDE - Development Launcher

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Kill any existing instances
killall -9 node 2>/dev/null
killall -9 vite 2>/dev/null
killall -9 electron 2>/dev/null
sleep 2

echo "🚀 Starting ViStudio IDE..."

# Start in background
nohup bash -c '
  export VITE_DEV_SERVER_URL=http://localhost:5173
  npx vite --host 127.0.0.1 --port 5173 > /dev/null 2>&1 &
  sleep 4
  npx electron .
' > /tmp/vistudio.log 2>&1 &

echo "⏳ Starting..."
sleep 8

# Show status
if ps aux | grep "electron/dist/electron \." | grep -v grep > /dev/null; then
  echo "✅ ViStudio is running!"
  echo "📝 Logs: tail -f /tmp/vistudio.log"
  echo "🛑 Stop: killall electron vite"
else
  echo "❌ Failed to start. Check logs: cat /tmp/vistudio.log"
fi
