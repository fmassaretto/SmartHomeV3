// Login page JavaScript
document.addEventListener('DOMContentLoaded', function() {
    // Setup event listeners
    document.getElementById('login-form').addEventListener('submit', handleLogin);
});

// Handle login form submission
function handleLogin(event) {
    event.preventDefault();
    
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const messageElement = document.getElementById('login-message');
    
    // Clear previous messages
    messageElement.textContent = '';
    messageElement.className = 'form-message';
    
    // Validate input
    if (!username || !password) {
        messageElement.textContent = 'Username and password are required';
        messageElement.className = 'form-message error';
        return;
    }
    
    // Send login request
    fetch('/api/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            username: username,
            password: password
        })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Invalid username or password');
        }
        return response.json();
    })
    .then(data => {
        if (data.success) {
            // Store username in cookie for display purposes
            document.cookie = `username=${encodeURIComponent(username)}; path=/; max-age=3600`;
            
            // Redirect to home page
            window.location.href = '/';
        } else {
            messageElement.textContent = data.message || 'Login failed';
            messageElement.className = 'form-message error';
        }
    })
    .catch(error => {
        messageElement.textContent = error.message;
        messageElement.className = 'form-message error';
    });
}
