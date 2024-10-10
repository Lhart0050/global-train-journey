// Set an initial timeout duration, e.g., 10 seconds
const TIMEOUT_DURATION = 10000;
let espTimeout;

// Function to update status to "Disconnected" when the ESP doesn't respond in time
function setDisconnectedStatus() {
    const statusText = document.getElementById('status-text');
    const statusIcon = document.getElementById('status-icon');
    
    statusText.textContent = "Disconnected";
    statusText.classList.remove('text-success');
    statusText.classList.add('text-danger');
    
    statusIcon.classList.remove('text-success');
    statusIcon.classList.add('text-danger');
}

// Socket.io logic to update status and reset timeout on receiving data from ESP32
const socket = io();

socket.on('esp-status', function(data) {
    const statusText = document.getElementById('status-text');
    const statusIcon = document.getElementById('status-icon');
    
    if (data.connected) {
        statusText.textContent = "Connected";
        statusText.classList.remove('text-danger');
        statusText.classList.add('text-success');
        
        statusIcon.classList.remove('text-danger');
        statusIcon.classList.add('text-success');
        
        // Clear the previous timeout and start a new one
        clearTimeout(espTimeout);
        espTimeout = setTimeout(setDisconnectedStatus, TIMEOUT_DURATION);
    } else {
        setDisconnectedStatus();
    }
});

// Optionally, handle manual disconnection (e.g., when the user refreshes the page)
socket.on('disconnect', function() {
    setDisconnectedStatus();
});
