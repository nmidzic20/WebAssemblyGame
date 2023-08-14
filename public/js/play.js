Module = {};
playing = false;

let usernameInput = document.getElementById("username-input");
let submitButton = document.getElementById("submit-button");
let startButton = document.getElementById("start-button");
let restartButton = document.getElementById("restart-button");
let avatarContainer = document.getElementById("avatar-container");

submitButton.addEventListener("click", function () {
  let username = usernameInput.value;
  let score = Module.playerScore;

  fetch("/save_player", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      username: username,
      score: score,
    }),
  })
    .then((response) => response.json())
    .then((data) => {
      console.log(data.message);
      usernameInput.value = "";
      Module.gameDataStored = true;
      usernameInput.style.display = "none";
      submitButton.style.display = "none";
    })
    .catch((error) => {
      console.error("Error:", error);
    });
});

usernameInput.addEventListener("keydown", function (event) {
  if (event.key == "Backspace")
    usernameInput.value = usernameInput.value.slice(0, -1);
  else if (event.key.length === 1) usernameInput.value += event.key;
});

startButton.addEventListener("click", function () {
  Module.ccall("start_game", "void", ["number"], [Module.context]);
  document.getElementById("start-container").style.display = "none";
  playing = true;
});

restartButton.addEventListener("click", function () {
  Module.ccall("restart_game", "void", ["number"], [Module.context]);
  document.getElementById("username-container").style.display = "none";
});

if (!Module.canvas) {
  Module.canvas = document.getElementById("canvas");
}
loadIndexJS();

function loadIndexJS() {
  let existingScript = document.querySelector("script[src='/index.js']");
  if (existingScript) {
    existingScript.remove();
  }

  let script = document.createElement("script");
  script.src = "/index.js";
  document.head.appendChild(script);
}

Module.gameDataStored = false;

Module.onGameLoaded = function (width, height) {
  let startContainer = document.getElementById("start-container");
  let usernameContainer = document.getElementById("username-container");
  startContainer.style.width = width + "px";
  startContainer.style.height = height + "px";
  usernameContainer.style.width = width + "px";
  usernameContainer.style.height = height + "px";

  startButton.style.display = "block";
  avatarContainer.style.display = "flex";
};
