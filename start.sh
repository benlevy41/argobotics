#!/bin/bash
#run this in the nautilus2 directory

cleanup() {
	echo "stopping running processes..."
	kill 0
	exit 0
}

trap cleanup EXIT

#start python scripts
python3 -m http.server 8000 > http.log 2>&1 &
python3 websocket_server.py > ws.log 2>&1 &

#start rpicam-vid raw video stream to localhost port 8555
rpicam-vid -t 0 --bitrate 2000000 --profile baseline --width 1920 --height 1080 --inline --listen -o tcp://127.0.0.1:8555 &
echo "rpicam-vid started"
sleep 1

#start mediamtx to serve stream over all protocols
"$HOME/nautilus2/mediamtx/mediamtx"  "$HOME/nautilus2/mediamtx/mediamtx.yml" &
echo "mediamtx started"

#wait for mediamtx to open port
while ! echo > /dev/tcp/127.0.0.1/8554 2>/dev/null; do
	echo "waiting for mediamtx..." >> debug.log
	sleep 1
done

#start ffmpeg to wrap raw video into rstp and redirect to port 8554/cam
ffmpeg -i tcp://127.0.0.1:8555 -c:v copy -f rtsp -an -rtsp_transport tcp rtsp://127.0.0.1:8554/cam > ffmpg.log 2>&1
