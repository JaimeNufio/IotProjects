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
      VALUES ($1, CURRENT_TIMESTAMP)
      RETURNING *;
    `;

    await client.query(insertQuery, values);
    client.release();

    res.send(`${Math.floor(Date.now() / 1000)}`);

  } catch (err) {
    console.error('Error occurred:', err);
  }
}

async function recallInteraction(res,name){
  const client = await pool.connect();

  console.log('connected via recall rt')

  try {
    values = [name]
    const query = `
      SELECT EXTRACT(EPOCH FROM "time" AT TIME ZONE 'UTC') AS unix_timestamp
      FROM timekeep
      WHERE NAME = '${values[0]}'
      ORDER BY id DESC
      LIMIT 1;
    `;

    const result = await client.query(query)


    if (result.rows.length > 0) {
      const row = result.rows[0]
      console.log(row.unix_timestamp)
      res.send(result.unix_timestamp);
    } else {
      console.log(0);
      res.send(0)
    }
  } catch (error) {
    console.error('Error executing query:', error);
  } finally {
    client.release();
  }
};





// button pressed, record the interaction and return back the time stamp
app.get('/interact', (req, res) => {
  recordInteraction(res, req.query.name)
});

// device powered on, fetch last time stored.
app.get('/recall', (req, res) => {

  recallInteraction(res, req.query.name)
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});