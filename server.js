const express = require("express");
const sqlite3 = require("sqlite3");
const fs = require("fs/promises");

const app = express();
const port = 3000;

app.use(express.static("public"));
app.use(express.urlencoded({ extended: true }));
app.use(express.json());

const db = new sqlite3.Database("game_data.db");

db.run(`
  CREATE TABLE IF NOT EXISTS players (
    id INTEGER PRIMARY KEY,
    username TEXT,
    score INTEGER
  )
`);

app.get("/", (req, res) => {
  res.redirect("/play");
});

app.get("/play", async (req, res) => {
  const playHtml = await loadPage("play");
  res.send(playHtml);
});

app.get("/leaderboard", async (req, res) => {
  const scores = await getScoresFromDatabase();
  const scoresTable = generateScoresTable(scores);

  let leaderboardHtml = await loadPage("leaderboard");
  leaderboardHtml = leaderboardHtml.replace("#content#", scoresTable);

  res.send(leaderboardHtml);
});

app.post("/save_player", (req, res) => {
  const { username, score } = req.body;

  db.run(
    "INSERT INTO players (username, score) VALUES (?, ?)",
    [username, score],
    function (err) {
      if (err) {
        console.error("Error saving player:", err);
        res.status(500).json({ message: "Error saving player" });
      } else {
        console.log("Player saved successfully");
        res.json({ message: "Player saved successfully" });
      }
    }
  );
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});

async function loadPage(pageTitle) {
  let pages = [loadHTML(pageTitle), loadHTML("nav")];
  let [page, nav] = await Promise.all(pages);
  page = page.replace("#nav#", nav);
  return page;
}

function loadHTML(path) {
  return fs.readFile(__dirname + "/public/html/" + path + ".html", "UTF-8");
}

async function getScoresFromDatabase() {
  return new Promise((resolve, reject) => {
    db.all("SELECT * FROM players", (err, rows) => {
      if (err) {
        reject(err);
      } else {
        resolve(rows);
      }
    });
  });
}

function generateScoresTable(scores) {
  let tableHtml = "<table><tr><th>ID</th><th>Username</th><th>Score</th></tr>";

  for (const score of scores) {
    tableHtml += `<tr><td>${score.id}</td><td>${score.username}</td><td>${score.score}</td></tr>`;
  }

  tableHtml += "</table>";
  return tableHtml;
}
