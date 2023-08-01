const express = require('express');
const { Pool } = require('pg');

const app = express();
const port = 3000;

const pool = new Pool({
  user: 'postgres',
  host: '192.168.1.156',
  database: 'IoT',
  password: 'postgres',
  port: 9856,
});

async function connectToPGServer() {
  try {
    await pool.connect();
    console.log('Connected to PostgreSQL server!');
  } catch (error) {
    console.error('Error connecting to PostgreSQL server:', error);
  }
}

connectToPGServer();

async function recordInteraction(res,name) {
  console.log("adding timestamp from device",name)
  try {

    const client = await pool.connect();
    const values = [name];
    const insertQuery = `
      INSERT INTO timekeep (name,time)
      VALUES ($1, CURRENT_TIME)
      RETURNING *;
    `;

    await client.query(insertQuery, values);
    client.release();

    res.send("created Interaction for "+name);

  } catch (err) {
    console.error('Error occurred:', err);
    // res.status(400).send("Error during interaction for device, "+name);
  }
}

app.get('/interact', (req, res) => {
  recordInteraction(res, req.query.name)
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});