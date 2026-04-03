function last() {
	var request = new XMLHttpRequest();
	request.open("POST", "/api/audio/last");
	request.send("");
}

function next() {
	var request = new XMLHttpRequest();
	request.open("POST", "/api/audio/next");
	request.send("");
}

function pauseResume() {
	var request = new XMLHttpRequest();
	request.open("POST", "/api/audio/pauseResume");
	request.send("");
}

function suffle() {
	var request = new XMLHttpRequest();
	request.open("POST", "/api/audio/suffle");
	request.send("");
}

document.getElementById("last").addEventListener("click", last);
document.getElementById("next").addEventListener("click", next);
document.getElementById("pauseResume").addEventListener("click", pauseResume);
document.getElementById("suffle").addEventListener("click", suffle);
console.log("\"audio.js\" loaded");
