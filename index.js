// index.js
const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// Handle client connections via Socket.io
io.on('connection', (socket) => {
  console.log('A user connected');

  // Handle disconnection
  socket.on('disconnect', () => {
    console.log('A user disconnected');
  });

  // Receive answer from client
  socket.on('submit-answer', (data) => {
    console.log('Received answer:', data);
    // Validate answer logic here

    // Example Question: "Where does pizza come from?"
    if (data.question === "Where does pizza come from?") {
      if (data.answer.toLowerCase() === "italy") {
        // Correct answer
        socket.emit('answer-result', { correct: true });
        // Command ESP32 to move to Italy statue (assuming statue 1)
        io.emit('move-train', { target: 1 });
      } else {
        // Incorrect answer
        socket.emit('answer-result', { correct: false });
        // Option to retry or move back (auto move back for simplicity)
        io.emit('move-train', { target: 'start' });
      }
    }

    // Add more question validations as needed
  });

  // Handle joystick commands from client
  socket.on('joystick-command', (data) => {
    console.log('Received joystick command:', data);
    // Forward joystick commands to ESP32 master
    io.emit('joystick-command', data);
  });
});

// Start the server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
