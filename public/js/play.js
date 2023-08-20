Module = {};
playing = false;

let startContainer = document.getElementById("start-container");
let gameOverContainer = document.getElementById("game-over-container");
let usernameContainer = document.getElementById("username-container");
let usernameInput = document.getElementById("username-input");
let submitButton = document.getElementById("submit-button");
let startButton = document.getElementById("start-button");
let restartButton = document.getElementById("restart-button");
let avatarContainer = document.getElementById("threejs_canvas_wrapper");

submitButton.addEventListener("click", function () {
  let username = usernameInput.value;
  let score = Module.playerScore;

  fetch("/api/players", {
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
      usernameInput.value = "";
      Module.gameDataStored = true;
      usernameInput.style.display = "none";
      submitButton.style.display = "none";
      const message = usernameContainer.querySelector("p");
      message.textContent = data.message;
      message.style.color = "orange";
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
  gameOverContainer.style.display = "none";
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
  startContainer.style.width = width + "px";
  startContainer.style.height = height + "px";
  gameOverContainer.style.width = width + "px";
  gameOverContainer.style.height = height + "px";

  startButton.style.display = "block";
  avatarContainer.style.display = "flex";
};

Module.onGameStarted = function () {
  if (playing === false) {
    playing = true;
  }
};

Module.onGameEnded = function () {
  if (playing === true) {
    playing = false;
  }
};

// Fullscreen
const fullscreenIconGame = document.getElementById("fullscreen-game");
const fullscreenIconThreeJS = document.getElementById("fullscreen-threejs");

function toggleFullscreen(element) {
  if (document.fullscreenElement) {
    const exitFullscreen =
      document.exitFullscreen ||
      document.mozCancelFullScreen ||
      document.webkitExitFullscreen ||
      document.msExitFullscreen;
    if (exitFullscreen) {
      exitFullscreen.call(document);
    }
  } else {
    const requestFullscreen =
      element.requestFullscreen ||
      element.mozRequestFullScreen ||
      element.webkitRequestFullscreen ||
      element.msRequestFullscreen;
    if (requestFullscreen) {
      requestFullscreen.call(element);
    }
  }
}

function makeGameFullscreen() {
  if (
    startContainer.style.display !== "none" ||
    gameOverContainer.style.display !== "none"
  )
    return;

  toggleFullscreen(canvas);
  /*if (
    startContainer.style.display != "none" ||
    gameOverContainer.style.display != "none"
  )
    return;

  if (canvas.requestFullscreen) {
    canvas.requestFullscreen();
  } else if (canvas.mozRequestFullScreen) {
    canvas.mozRequestFullScreen();
  } else if (canvas.webkitRequestFullscreen) {
    canvas.webkitRequestFullscreen();
  } else if (canvas.msRequestFullscreen) {
    canvas.msRequestFullscreen();
  }*/
}

function makeThreeJSFullscreen() {
  toggleFullscreen(avatarContainer);
  /*if (document.fullscreenElement) {
    if (document.exitFullscreen) {
      document.exitFullscreen();
    } else if (document.mozCancelFullScreen) {
      document.mozCancelFullScreen();
    } else if (document.webkitExitFullscreen) {
      document.webkitExitFullscreen();
    } else if (document.msExitFullscreen) {
      document.msExitFullscreen();
    }
  } else {
    if (avatarContainer.requestFullscreen) {
      avatarContainer.requestFullscreen();
    } else if (avatarContainer.mozRequestFullScreen) {
      avatarContainer.mozRequestFullScreen();
    } else if (avatarContainer.webkitRequestFullscreen) {
      avatarContainer.webkitRequestFullscreen();
    } else if (avatarContainer.msRequestFullscreen) {
      avatarContainer.msRequestFullscreen();
    }
  }*/
}

fullscreenIconGame.addEventListener("click", makeGameFullscreen);
fullscreenIconThreeJS.addEventListener("click", makeThreeJSFullscreen);
