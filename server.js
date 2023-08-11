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
  CREATE TABLE IF NOT EXISTS scores (
    id INTEGER PRIMARY KEY,
    score INTEGER
  )
`);

app.get("/", (req, res) => {
  res.redirect("/game");
});

app.get("/game", async (req, res) => {
  const gameHtml = await loadPage("game");
  res.send(gameHtml);
});

app.get("/leaderboard", async (req, res) => {
  const leaderboardHtml = await loadPage("leaderboard");
  res.send(leaderboardHtml);
});

app.post("/save_score", (req, res) => {
  const { score } = req.body;

  db.run("INSERT INTO scores (score) VALUES (?)", [score], function (err) {
    if (err) {
      console.error("Error saving score:", err);
      res.status(500).json({ message: "Error saving score" });
    } else {
      console.log("Score saved successfully");
      res.json({ message: "Score saved successfully" });
    }
  });
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
