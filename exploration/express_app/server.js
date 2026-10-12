// Express test app for the Sprint 1 exploration.
//   npm install && node server.js      (listens on 8002)
const express = require("express");
const app = express();

app.get("/hello", (req, res) => res.send("hello from express\n"));

// CPU-bound: busy-loop on the single event-loop thread
app.get("/cpu", (req, res) => {
  const ms = parseInt(req.query.ms || "20", 10);
  const end = Date.now() + ms;
  let x = 0;
  while (Date.now() < end) x++;
  res.send(`cpu ${ms}ms\n`);
});

// I/O-bound: non-blocking wait, the event loop is free meanwhile
app.get("/io", (req, res) => {
  const ms = parseInt(req.query.ms || "20", 10);
  setTimeout(() => res.send(`io ${ms}ms\n`), ms);
});

app.listen(8002, () => console.log("express listening on 8002"));
