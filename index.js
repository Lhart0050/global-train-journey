const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');
const bodyParser = require('body-parser'); // To parse JSON bodies

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Middleware to parse JSON
app.use(bodyParser.json());

// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// Variable to track ESP32 status
let espStatus = false; // false = disconnected, true = connected
let espTimeout;

// Define the timeout duration (10 seconds)
const TIMEOUT_DURATION = 10000;

// Function to set ESP as disconnected after timeout
function setDisconnected() {
  espStatus = false;
  console.log('ESP32 disconnected due to timeout.');
  io.emit('esp-status', { connected: false });
}

// Handle client connections via Socket.io
io.on('connection', (socket) => {
  console.log('A user connected');

  // Emit current ESP status to newly connected client
  socket.emit('esp-status', { connected: espStatus });

  // Handle disconnection
  socket.on('disconnect', () => {
    console.log('A user disconnected');
  });

  // Receive answer from client (if applicable)
  socket.on('submit-answer', (data) => {
    console.log('Received answer:', data);
    // Example Question: "Where does pizza come from?"
    if (data.question === "Where does pizza come from?") {
      if (data.answer.toLowerCase() === "italy") {
        // Correct answer
        socket.emit('answer-result', { correct: true });
        io.emit('move-train', { target: 1 });
      } else {
        // Incorrect answer
        socket.emit('answer-result', { correct: false });
        io.emit('move-train', { target: 'start' });
      }
    }
  });
});

// Endpoint to receive ESP32 status updates
app.post('/esp-status', (req, res) => {
  const { connected } = req.body;

  if (typeof connected !== 'boolean') {
    return res.status(400).json({ message: 'Invalid status format. Expected boolean.' });
  }

  // Update ESP status
  espStatus = connected;
  console.log(`ESP32 is now ${connected ? 'connected' : 'disconnected'}.`);

  // Broadcast the new status to all connected website clients
  io.emit('esp-status', { connected });

  // Clear previous timeout and start a new one
  clearTimeout(espTimeout);
  if (connected) {
    espTimeout = setTimeout(setDisconnected, TIMEOUT_DURATION); // Timeout after 10 seconds
  }

  res.status(200).json({ message: 'Status updated successfully.' });
});

// Start the server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});

// const express = require('express');
// const http = require('http');
// const socketIo = require('socket.io');
// const path = require('path');
// const bodyParser = require('body-parser'); // To parse JSON bodies

// const app = express();
// const server = http.createServer(app);
// const io = socketIo(server);

// // Middleware to parse JSON
// app.use(bodyParser.json());

// // Serve static files from the 'public' directory
// app.use(express.static(path.join(__dirname, 'public')));

// // Variable to track ESP32 status
// let espStatus = false; // false = disconnected, true = connected
// let espTimeout;

// // Define the timeout duration (10 seconds)
// const TIMEOUT_DURATION = 10000;

// // Function to set ESP as disconnected after timeout
// function setDisconnected() {
//   espStatus = false;
//   console.log('ESP32 disconnected due to timeout.');
//   io.emit('esp-status', { connected: false });
// }

// // Dummy data: 10 sets of questions
// const questions = [
//   {
//     question: "This food is called Pizza. Where does it come from?",
//     answer: "Italy",
//     image: "pizza.jpg",
//   },
//   {
//     question: "What is the capital city of France?",
//     answer: "Paris",
//     image: "paris.jpg",
//   },
//   {
//     question: "In which country is the Great Wall located?",
//     answer: "China",
//     image: "great_wall.jpg",
//   },
//   {
//     question: "Which country is famous for the pyramids?",
//     answer: "Egypt",
//     image: "pyramids.jpg",
//   },
//   {
//     question: "What is the official language of Brazil?",
//     answer: "Portuguese",
//     image: "brazil.jpg",
//   },
//   {
//     question: "Which country is known as the Land of the Rising Sun?",
//     answer: "Japan",
//     image: "japan.jpg",
//   },
//   {
//     question: "Which city is known as the Big Apple?",
//     answer: "New York",
//     image: "new_york.jpg",
//   },
//   {
//     question: "In which country would you find the Sydney Opera House?",
//     answer: "Australia",
//     image: "sydney_opera_house.jpg",
//   },
//   {
//     question: "Which river runs through London?",
//     answer: "Thames",
//     image: "london.jpg",
//   },
//   {
//     question: "Mount Everest is located on the border of Nepal and which country?",
//     answer: "China",
//     image: "everest.jpg",
//   },
// ];

// // Endpoint to serve questions
// app.get('/questions', (req, res) => {
//   // Shuffle the questions
//   const shuffledQuestions = questions.sort(() => Math.random() - 0.5);
//   res.json(shuffledQuestions);
// });

// // Handle client connections via Socket.io
// io.on('connection', (socket) => {
//   console.log('A user connected');

//   // Emit current ESP status to newly connected client
//   socket.emit('esp-status', { connected: espStatus });

//   // Handle disconnection
//   socket.on('disconnect', () => {
//     console.log('A user disconnected');
//   });

//   // Receive answer from client
//   socket.on('submit-answer', (data) => {
//     console.log('Received answer:', data);
//     const { question, answer } = data;
//     // Find the question in the questions array
//     const q = questions.find((q) => q.question === question);
//     if (q) {
//       if (answer.trim().toLowerCase() === q.answer.toLowerCase()) {
//         // Correct answer
//         socket.emit('answer-result', { correct: true });
//         // Additional logic can be added here
//         io.emit('move-train', { target: 'next' });
//       } else {
//         // Incorrect answer
//         socket.emit('answer-result', { correct: false });
//       }
//     } else {
//       console.log('Question not found');
//       socket.emit('answer-result', { correct: false, message: 'Question not found' });
//     }
//   });
// });

// // Endpoint to receive ESP32 status updates
// app.post('/esp-status', (req, res) => {
//   const { connected } = req.body;

//   if (typeof connected !== 'boolean') {
//     return res.status(400).json({ message: 'Invalid status format. Expected boolean.' });
//   }

//   // Update ESP status
//   espStatus = connected;
//   console.log(`ESP32 is now ${connected ? 'connected' : 'disconnected'}.`);

//   // Broadcast the new status to all connected website clients
//   io.emit('esp-status', { connected });

//   // Clear previous timeout and start a new one
//   clearTimeout(espTimeout);
//   if (connected) {
//     espTimeout = setTimeout(setDisconnected, TIMEOUT_DURATION); // Timeout after 10 seconds
//   }

//   res.status(200).json({ message: 'Status updated successfully.' });
// });

// // Start the server
// const PORT = process.env.PORT || 3000;
// server.listen(PORT, () => {
//   console.log(`Server is running on port ${PORT}`);
// });
