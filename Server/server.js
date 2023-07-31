const express = require('express');
const { Pool } = require('pg'); // Import the Pool class from 'pg'

const app = express();
const port = 3000;

// Configure the connection to your PostgreSQL server
const pool = new Pool({
  user: 'postgres',
  host: '192.168.1.156',
  database: 'IoT',
  password: 'postgres',
  port: 9856,
});

// Function to connect to the PostgreSQL server on startup
async function connectToPGServer() {
  try {
    await pool.connect();
    console.log('Connected to PostgreSQL server!');
  } catch (error) {
    console.error('Error connecting to PostgreSQL server:', error);
  }
}

// Call the connectToPGServer function on server startup
connectToPGServer();

// Define the route for the root endpoint
app.get('/', (req, res) => {
  res.send('Hello World');
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});