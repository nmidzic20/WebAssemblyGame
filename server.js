const express = require("express");
const sqlite3 = require("sqlite3");

const app = express();
const port = 3000;

app.use(express.static("public"));
app.use(express.json());

const db = new sqlite3.Database("game_data.db");

db.run(`
  CREATE TABLE IF NOT EXISTS scores (
    id INTEGER PRIMARY KEY,
    score INTEGER
  )
`);

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
