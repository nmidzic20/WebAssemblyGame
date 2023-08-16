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
  const players = await getPlayersPaginated(1, 10);
  const playersTable = generatePlayersTable(players);

  let leaderboardHtml = await loadPage("leaderboard");
  leaderboardHtml = leaderboardHtml.replace("#content#", playersTable);

  res.send(leaderboardHtml);
});

app.get("/api/players", async (req, res) => {
  let players;
  if (req.query.itemsPerPage && req.query.page)
    players = await getPlayersPaginated(req.query.page, req.query.itemsPerPage);
  else players = await getPlayers();
  res.send(players);
});

app.post("/api/players", (req, res) => {
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

/*app.post("/table", async (req, res) => {
  const page = parseInt(req.body.page) || 1;
  const itemsPerPage = parseInt(req.body.itemsPerPage) || 10;
  const data = await getPlayersPaginated(page, itemsPerPage);
  res.send(data);
});*/

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

async function getPlayers() {
  return new Promise((resolve, reject) => {
    db.all("SELECT * FROM players", [], (err, rows) => {
      if (err) {
        reject(err);
      } else {
        resolve(rows);
      }
    });
  });
}

async function getPlayersPaginated(page, itemsPerPage) {
  const offset = (page - 1) * itemsPerPage;
  return new Promise((resolve, reject) => {
    db.all(
      "SELECT * FROM players LIMIT ? OFFSET ?",
      [itemsPerPage, offset],
      (err, rows) => {
        if (err) {
          reject(err);
        } else {
          resolve(rows);
        }
      }
    );
  });
}

function generatePlayersTable(players) {
  const selectString = generateSelectWithOptions();

  let tableOptions =
    "<div id='table-options'>" +
    selectString +
    "<div id='pagination'></div></div>";

  let tableHtml =
    tableOptions +
    "<div id='table-wrapper'><table><tr><th>ID</th><th>Username</th><th>Score</th></tr>";

  for (const player of players) {
    tableHtml += `<tr><td>${player.id}</td><td>${player.username}</td><td>${player.score}</td></tr>`;
  }

  tableHtml += "</table></div>";
  return tableHtml;
}

function generateSelectWithOptions() {
  const options = [5, 10, 20];
  let selectHTML = '<select id="itemsPerPageSelect">';

  for (const option of options) {
    selectHTML += `<option value="${option}">${option}</option>`;
  }

  selectHTML += "</select>";
  return selectHTML;
}
