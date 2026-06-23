const ws = new WebSocket(`ws://${window.location.hostname}:8765`);

ws.onopen = () => {
	console.log("opened websocket");
};

ws.onmessage = (event) => {
	console.log("received: ", event.data);
};

ws.onerror = (err) => {
	console.error("websocket error: ", err);
};


document.addEventListener("keydown", (event) => {
	const key = event.key.toLowerCase();
	console.log("Key pressed: ", key);
	ws.send(key);
});

const pc = new RTCPeerConnection();

const video = document.getElementById("stream");
pc.ontrack=(event)=> {
	video.srcObject = event.streams[0];
};

fetch("http://nautilus2:8889/cam/", {
	method: "POST",
	body: JSON.stringify({
		"type": "request"
	})
})

.then(res => res.json())
.then(async (data) => {
	const offer = new RTCSessionDescription(data.offer);
	await pc.setRemoteDescription(offer);

	const answer = await pc.createAnswer();
	await pc.setLocalDescription(answer);

	fetch("http://nautilus2:8889/cam/", {
		method: "POST",
		body: JSON.stringify({ "answer": pc.localDescription })
	});
});
