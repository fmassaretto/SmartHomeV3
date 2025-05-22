// Main JavaScript for the device control interface
document.addEventListener('DOMContentLoaded', function() {
    // Check if user is logged in
    checkAuth();
    
    // Setup event listeners
    document.getElementById('logout-btn').addEventListener('click', logout);
    
    // Setup event source for real-time updates
    setupEventSource();
    
    // Get system status
    getSystemStatus();
    
    // Get devices
    getDevices();
    
    // Refresh status every 30 seconds
    setInterval(getSystemStatus, 30000);
});

// Check if user is authenticated
function checkAuth() {
    fetch('/api/status')
        .then(response => {
            if (response.status === 401) {
                // Redirect to login page if not authenticated
                window.location.href = '/login';
                return;
            }
            return response.json();
        })
        .then(data => {
            if (data) {
                // Get username from session
                const cookies = document.cookie.split(';');
                for (let cookie of cookies) {
                    const [name, value] = cookie.trim().split('=');
                    if (name === 'username') {
                        document.getElementById('username').textContent = decodeURIComponent(value);
                        
                        // Show admin link if user is admin
                        checkAdminAccess();
                        break;
                    }
                }
            }
        })
        .catch(error => {
            console.error('Error checking authentication:', error);
        });
}

// Check if user has admin access
function checkAdminAccess() {
    fetch('/api/users')
        .then(response => {
            if (response.status === 200) {
                // User is admin, show admin link
                document.getElementById('admin-link').style.display = 'inline-block';
            }
        })
        .catch(error => {
            console.error('Error checking admin access:', error);
        });
}

// Get system status
function getSystemStatus() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            document.getElementById('wifi-status').textContent = data.wifi.connected ? 'Connected' : 'Disconnected';
            document.getElementById('wifi-status').className = 'status-value ' + (data.wifi.connected ? 'connected' : 'disconnected');
            document.getElementById('ip-address').textContent = data.wifi.ip;
            
            // Format uptime
            const uptime = formatUptime(data.uptime);
            document.getElementById('uptime').textContent = uptime;
        })
        .catch(error => {
            console.error('Error getting system status:', error);
        });
}

// Format uptime in days, hours, minutes, seconds
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    seconds %= 86400;
    const hours = Math.floor(seconds / 3600);
    seconds %= 3600;
    const minutes = Math.floor(seconds / 60);
    seconds %= 60;
    
    let result = '';
    if (days > 0) result += `${days}d `;
    if (hours > 0 || days > 0) result += `${hours}h `;
    if (minutes > 0 || hours > 0 || days > 0) result += `${minutes}m `;
    result += `${seconds}s`;
    
    return result;
}

// Get devices
function getDevices() {
    fetch('/api/devices')
        .then(response => response.json())
        .then(data => {
            const devicesList = document.getElementById('devices-list');
            devicesList.innerHTML = '';
            
            data.devices.forEach(device => {
                const deviceCard = createDeviceCard(device);
                devicesList.appendChild(deviceCard);
            });
        })
        .catch(error => {
            console.error('Error getting devices:', error);
        });
}

// Create device card
function createDeviceCard(device) {
    const deviceCard = document.createElement('div');
    deviceCard.className = 'device-card';
    deviceCard.id = `device-${device.channel}`;
    
    const deviceName = document.createElement('div');
    deviceName.className = 'device-name';
    deviceName.textContent = device.name.replace(/_/g, ' ');
    
    const deviceStatus = document.createElement('div');
    deviceStatus.className = 'device-status';
    
    const statusText = document.createElement('span');
    statusText.className = 'status-text';
    statusText.textContent = device.state ? 'ON' : 'OFF';
    
    const switchLabel = document.createElement('label');
    switchLabel.className = 'switch';
    
    const switchInput = document.createElement('input');
    switchInput.type = 'checkbox';
    switchInput.checked = device.state;
    switchInput.disabled = !device.canControl;
    switchInput.addEventListener('change', () => toggleDevice(device.channel, switchInput.checked));
    
    const switchSlider = document.createElement('span');
    switchSlider.className = 'slider';
    
    switchLabel.appendChild(switchInput);
    switchLabel.appendChild(switchSlider);
    
    deviceStatus.appendChild(statusText);
    deviceStatus.appendChild(switchLabel);
    
    deviceCard.appendChild(deviceName);
    deviceCard.appendChild(deviceStatus);
    
    return deviceCard;
}

// Toggle device state
function toggleDevice(channel, state) {
    fetch('/api/devices/toggle', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            channel: channel,
            state: state
        })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Failed to toggle device');
        }
        return response.json();
    })
    .then(data => {
        console.log('Device toggled:', data);
    })
    .catch(error => {
        console.error('Error toggling device:', error);
        // Revert switch state on error
        const switchInput = document.querySelector(`#device-${channel} input[type="checkbox"]`);
        if (switchInput) {
            switchInput.checked = !state;
        }
    });
}

// Setup event source for real-time updates
function setupEventSource() {
    if (!!window.EventSource) {
        const eventSource = new EventSource('/events');
        
        eventSource.addEventListener('state', function(e) {
            const data = JSON.parse(e.data);
            updateDeviceState(data.channel, data.state);
        }, false);
        
        eventSource.addEventListener('error', function(e) {
            if (e.target.readyState === EventSource.CLOSED) {
                console.log('Event source closed');
            } else if (e.target.readyState === EventSource.CONNECTING) {
                console.log('Connecting to event source...');
            }
        }, false);
    } else {
        console.error('EventSource not supported');
    }
}

// Update device state in UI
function updateDeviceState(channel, state) {
    const deviceCard = document.getElementById(`device-${channel}`);
    if (deviceCard) {
        const statusText = deviceCard.querySelector('.status-text');
        const switchInput = deviceCard.querySelector('input[type="checkbox"]');
        
        if (statusText) {
            statusText.textContent = state ? 'ON' : 'OFF';
        }
        
        if (switchInput && !switchInput.disabled) {
            switchInput.checked = state;
        }
    }
}

// Logout function
function logout() {
    fetch('/api/logout', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            window.location.href = '/login';
        }
    })
    .catch(error => {
        console.error('Error logging out:', error);
    });
}
