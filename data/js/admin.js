// Admin panel JavaScript
document.addEventListener('DOMContentLoaded', function() {
    // Check if user is logged in and has admin access
    checkAdminAuth();
    
    // Setup event listeners
    document.getElementById('logout-btn').addEventListener('click', logout);
    
    // Setup tab navigation
    setupTabs();
    
    // Setup modal functionality
    setupModal();
    
    // Get system status
    getSystemStatus();
    
    // Get users
    getUsers();
    
    // Get devices
    getDevices();
    
    // Refresh status every 30 seconds
    setInterval(getSystemStatus, 30000);
});

// Check if user is authenticated and has admin access
function checkAdminAuth() {
    fetch('/api/users')
        .then(response => {
            if (response.status === 401) {
                // Redirect to login page if not authenticated
                window.location.href = '/login';
                return;
            }
            if (response.status === 403) {
                // Redirect to home page if not admin
                window.location.href = '/';
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
                        break;
                    }
                }
            }
        })
        .catch(error => {
            console.error('Error checking authentication:', error);
        });
}

// Setup tab navigation
function setupTabs() {
    const tabButtons = document.querySelectorAll('.tab-btn');
    const tabPanes = document.querySelectorAll('.tab-pane');
    
    tabButtons.forEach(button => {
        button.addEventListener('click', () => {
            // Remove active class from all buttons and panes
            tabButtons.forEach(btn => btn.classList.remove('active'));
            tabPanes.forEach(pane => pane.classList.remove('active'));
            
            // Add active class to clicked button and corresponding pane
            button.classList.add('active');
            const tabId = button.dataset.tab;
            document.getElementById(`${tabId}-tab`).classList.add('active');
        });
    });
}

// Setup modal functionality
function setupModal() {
    const modal = document.getElementById('user-modal');
    const closeBtn = document.querySelector('.close');
    const cancelBtn = document.getElementById('cancel-user-btn');
    const addUserBtn = document.getElementById('add-user-btn');
    const userForm = document.getElementById('user-form');
    
    // Open modal when add user button is clicked
    addUserBtn.addEventListener('click', () => {
        document.getElementById('modal-title').textContent = 'Add User';
        document.getElementById('edit-mode').value = 'add';
        document.getElementById('user-username').disabled = false;
        document.getElementById('user-password').required = true;
        document.getElementById('password-hint').style.display = 'none';
        userForm.reset();
        populateDeviceCheckboxes();
        modal.style.display = 'block';
    });
    
    // Close modal when close button is clicked
    closeBtn.addEventListener('click', () => {
        modal.style.display = 'none';
    });
    
    // Close modal when cancel button is clicked
    cancelBtn.addEventListener('click', () => {
        modal.style.display = 'none';
    });
    
    // Close modal when clicking outside of it
    window.addEventListener('click', (event) => {
        if (event.target === modal) {
            modal.style.display = 'none';
        }
    });
    
    // Handle form submission
    userForm.addEventListener('submit', (event) => {
        event.preventDefault();
        
        const editMode = document.getElementById('edit-mode').value;
        const username = document.getElementById('user-username').value;
        const password = document.getElementById('user-password').value;
        const role = parseInt(document.getElementById('user-role').value);
        
        // Get allowed devices
        const allowedDevices = [];
        const deviceCheckboxes = document.querySelectorAll('#allowed-devices-checkboxes input[type="checkbox"]:checked');
        deviceCheckboxes.forEach(checkbox => {
            allowedDevices.push(parseInt(checkbox.value));
        });
        
        // Show/hide allowed devices based on role
        updateAllowedDevicesVisibility(role);
        
        // Create request body
        const requestBody = {
            username: username,
            role: role,
            allowedDevices: allowedDevices
        };
        
        // Add password if provided
        if (password) {
            requestBody.password = password;
        }
        
        // Send request
        const endpoint = editMode === 'add' ? '/api/users/add' : '/api/users/update';
        fetch(endpoint, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(requestBody)
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('Failed to ' + (editMode === 'add' ? 'add' : 'update') + ' user');
            }
            return response.json();
        })
        .then(data => {
            if (data.success) {
                // Close modal
                modal.style.display = 'none';
                
                // Refresh users list
                getUsers();
                
                // Show success message
                alert((editMode === 'add' ? 'User added' : 'User updated') + ' successfully');
            } else {
                document.getElementById('user-form-message').textContent = data.message;
                document.getElementById('user-form-message').className = 'form-message error';
            }
        })
        .catch(error => {
            document.getElementById('user-form-message').textContent = error.message;
            document.getElementById('user-form-message').className = 'form-message error';
        });
    });
    
    // Update allowed devices visibility when role changes
    document.getElementById('user-role').addEventListener('change', function() {
        updateAllowedDevicesVisibility(parseInt(this.value));
    });
}

// Update allowed devices visibility based on role
function updateAllowedDevicesVisibility(role) {
    const allowedDevicesGroup = document.getElementById('allowed-devices-group');
    
    // Show allowed devices only for operator role
    if (role === 1) { // Operator
        allowedDevicesGroup.style.display = 'block';
    } else {
        allowedDevicesGroup.style.display = 'none';
    }
}

// Populate device checkboxes in user form
function populateDeviceCheckboxes() {
    const checkboxesContainer = document.getElementById('allowed-devices-checkboxes');
    checkboxesContainer.innerHTML = '';
    
    fetch('/api/devices')
        .then(response => response.json())
        .then(data => {
            data.devices.forEach(device => {
                const checkbox = document.createElement('div');
                checkbox.className = 'device-checkbox';
                
                const input = document.createElement('input');
                input.type = 'checkbox';
                input.id = `device-checkbox-${device.channel}`;
                input.value = device.channel;
                
                const label = document.createElement('label');
                label.htmlFor = `device-checkbox-${device.channel}`;
                label.textContent = device.name.replace(/_/g, ' ');
                
                checkbox.appendChild(input);
                checkbox.appendChild(label);
                checkboxesContainer.appendChild(checkbox);
            });
        })
        .catch(error => {
            console.error('Error getting devices for checkboxes:', error);
        });
}

// Get system status
function getSystemStatus() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            document.getElementById('wifi-ssid').textContent = data.wifi.ssid;
            document.getElementById('wifi-signal').textContent = data.wifi.rssi + ' dBm';
            document.getElementById('ip-address-admin').textContent = data.wifi.ip;
            
            // Format uptime
            const uptime = formatUptime(data.uptime);
            document.getElementById('uptime-admin').textContent = uptime;
            
            // Format memory
            const memory = formatMemory(data.freeHeap);
            document.getElementById('free-memory').textContent = memory;
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

// Format memory in KB or MB
function formatMemory(bytes) {
    if (bytes < 1024) {
        return bytes + ' B';
    } else if (bytes < 1048576) {
        return (bytes / 1024).toFixed(2) + ' KB';
    } else {
        return (bytes / 1048576).toFixed(2) + ' MB';
    }
}

// Get users
function getUsers() {
    fetch('/api/users')
        .then(response => response.json())
        .then(data => {
            const usersTableBody = document.getElementById('users-table-body');
            usersTableBody.innerHTML = '';
            
            data.users.forEach(user => {
                const row = document.createElement('tr');
                
                // Username
                const usernameCell = document.createElement('td');
                usernameCell.textContent = user.username;
                row.appendChild(usernameCell);
                
                // Role
                const roleCell = document.createElement('td');
                roleCell.textContent = getRoleName(user.role);
                row.appendChild(roleCell);
                
                // Allowed devices
                const devicesCell = document.createElement('td');
                if (user.role === 1) { // Operator
                    if (user.allowedDevices.length > 0) {
                        devicesCell.textContent = user.allowedDevices.join(', ');
                    } else {
                        devicesCell.textContent = 'None';
                    }
                } else {
                    devicesCell.textContent = user.role === 0 ? 'All' : 'None';
                }
                row.appendChild(devicesCell);
                
                // Actions
                const actionsCell = document.createElement('td');
                
                // Edit button
                const editButton = document.createElement('button');
                editButton.className = 'btn btn-primary';
                editButton.textContent = 'Edit';
                editButton.addEventListener('click', () => editUser(user));
                actionsCell.appendChild(editButton);
                
                // Delete button
                const deleteButton = document.createElement('button');
                deleteButton.className = 'btn btn-danger';
                deleteButton.textContent = 'Delete';
                deleteButton.style.marginLeft = '5px';
                deleteButton.addEventListener('click', () => deleteUser(user.username));
                actionsCell.appendChild(deleteButton);
                
                row.appendChild(actionsCell);
                
                usersTableBody.appendChild(row);
            });
        })
        .catch(error => {
            console.error('Error getting users:', error);
        });
}

// Get role name from role ID
function getRoleName(roleId) {
    switch (roleId) {
        case 0: return 'Admin';
        case 1: return 'Operator';
        case 2: return 'Viewer';
        default: return 'Unknown';
    }
}

// Edit user
function editUser(user) {
    const modal = document.getElementById('user-modal');
    document.getElementById('modal-title').textContent = 'Edit User';
    document.getElementById('edit-mode').value = 'edit';
    document.getElementById('user-username').value = user.username;
    document.getElementById('user-username').disabled = true;
    document.getElementById('user-password').value = '';
    document.getElementById('user-password').required = false;
    document.getElementById('password-hint').style.display = 'block';
    document.getElementById('user-role').value = user.role;
    
    // Update allowed devices visibility
    updateAllowedDevicesVisibility(user.role);
    
    // Populate device checkboxes
    populateDeviceCheckboxes();
    
    // Wait for checkboxes to be populated
    setTimeout(() => {
        // Check allowed devices
        user.allowedDevices.forEach(deviceChannel => {
            const checkbox = document.getElementById(`device-checkbox-${deviceChannel}`);
            if (checkbox) {
                checkbox.checked = true;
            }
        });
        
        // Show modal
        modal.style.display = 'block';
    }, 500);
}

// Delete user
function deleteUser(username) {
    if (confirm(`Are you sure you want to delete user "${username}"?`)) {
        fetch('/api/users/delete', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                username: username
            })
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('Failed to delete user');
            }
            return response.json();
        })
        .then(data => {
            if (data.success) {
                // Refresh users list
                getUsers();
                
                // Show success message
                alert('User deleted successfully');
            } else {
                alert(data.message || 'Failed to delete user');
            }
        })
        .catch(error => {
            alert(error.message);
        });
    }
}

// Get devices
function getDevices() {
    fetch('/api/devices')
        .then(response => response.json())
        .then(data => {
            const devicesTableBody = document.getElementById('devices-table-body');
            devicesTableBody.innerHTML = '';
            
            data.devices.forEach(device => {
                const row = document.createElement('tr');
                
                // Channel
                const channelCell = document.createElement('td');
                channelCell.textContent = device.channel;
                row.appendChild(channelCell);
                
                // Name
                const nameCell = document.createElement('td');
                nameCell.textContent = device.name.replace(/_/g, ' ');
                row.appendChild(nameCell);
                
                // State
                const stateCell = document.createElement('td');
                const stateSwitch = document.createElement('label');
                stateSwitch.className = 'switch';
                
                const stateInput = document.createElement('input');
                stateInput.type = 'checkbox';
                stateInput.checked = device.state;
                stateInput.addEventListener('change', () => toggleDevice(device.channel, stateInput.checked));
                
                const stateSlider = document.createElement('span');
                stateSlider.className = 'slider';
                
                stateSwitch.appendChild(stateInput);
                stateSwitch.appendChild(stateSlider);
                stateCell.appendChild(stateSwitch);
                row.appendChild(stateCell);
                
                // Alexa Enabled
                const alexaCell = document.createElement('td');
                const alexaSwitch = document.createElement('label');
                alexaSwitch.className = 'switch';
                
                const alexaInput = document.createElement('input');
                alexaInput.type = 'checkbox';
                alexaInput.checked = device.alexaEnabled;
                alexaInput.addEventListener('change', () => toggleAlexaEnabled(device.channel, alexaInput.checked));
                
                const alexaSlider = document.createElement('span');
                alexaSlider.className = 'slider';
                
                alexaSwitch.appendChild(alexaInput);
                alexaSwitch.appendChild(alexaSlider);
                alexaCell.appendChild(alexaSwitch);
                row.appendChild(alexaCell);
                
                // Actions
                const actionsCell = document.createElement('td');
                
                // Edit button
                const editButton = document.createElement('button');
                editButton.className = 'btn btn-primary';
                editButton.textContent = 'Edit';
                editButton.addEventListener('click', () => editDevice(device));
                actionsCell.appendChild(editButton);
                
                row.appendChild(actionsCell);
                
                devicesTableBody.appendChild(row);
            });
        })
        .catch(error => {
            console.error('Error getting devices:', error);
        });
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
        const switchInput = document.querySelector(`#devices-table-body tr:nth-child(${channel + 1}) input[type="checkbox"]`);
        if (switchInput) {
            switchInput.checked = !state;
        }
    });
}

// Toggle Alexa enabled
function toggleAlexaEnabled(channel, enabled) {
    // This would be implemented in a real system
    console.log(`Toggle Alexa enabled for device ${channel}: ${enabled}`);
}

// Edit device
function editDevice(device) {
    // This would be implemented in a real system
    console.log('Edit device:', device);
    alert('Device editing not implemented in this demo');
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
