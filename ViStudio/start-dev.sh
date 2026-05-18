#!/bin/bash
export VITE_DEV_SERVER_URL="http://localhost:5173"
vite &
wait-on http://localhost:5173
electron . --no-sandbox --disable-gpu
