const express = require("express");
const app = express();
const port = 3000; // Specify the desired port number

app.use(express.static("public"));

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
