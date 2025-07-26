#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <DHT.h>

// DHT sensor setup (optional - for real-time data)
#define DHT_PIN 4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Replace with your network credentials
const char* ssid = "BLESSED";
const char* password = "12345678B";

WebServer server(80);

// Helper function to determine content type
String getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".svg")) return "image/svg+xml";
    else if (filename.endsWith(".json")) return "application/json";
    return "text/plain";
}

// Store dynamic content
String portfolioData = "{}";
float temperature = 0;
float humidity = 0;
unsigned long lastSensorRead = 0;

// Main portfolio HTML
const char* portfolioHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My Portfolio - ESP32</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-container">
            <h2>My Portfolio</h2>
            <ul class="nav-menu">
                <li><a href="/">Home</a></li>
                <li><a href="/about">About</a></li>
                <li><a href="/projects">Projects</a></li>
                <li><a href="/sensor-data">Live Data</a></li>
                <li><a href="/admin">Admin</a></li>
            </ul>
        </div>
    </nav>

    <div class="container">
        <div class="card header">
            <img src="/profile.jpg" alt="Profile" class="profile-img" onerror="this.style.display='none'">
            <h1 id="name">Your Name</h1>
            <p class="subtitle" id="title">Full Stack Developer | IoT Enthusiast</p>
            <p id="description">Passionate about creating innovative solutions with modern technologies.</p>
        </div>
        
        <div class="card">
            <div class="section">
                <h2>Live System Status</h2>
                <div class="status-grid">
                    <div class="status-item">
                        <h3>Temperature</h3>
                        <p id="temperature">Loading...</p>
                    </div>
                    <div class="status-item">
                        <h3>Humidity</h3>
                        <p id="humidity">Loading...</p>
                    </div>
                    <div class="status-item">
                        <h3>Uptime</h3>
                        <p id="uptime">Loading...</p>
                    </div>
                    <div class="status-item">
                        <h3>Free Memory</h3>
                        <p id="memory">Loading...</p>
                    </div>
                </div>
            </div>
        </div>
        
        <div class="card">
            <div class="section">
                <h2>Skills</h2>
                <div class="skills" id="skills">
                    <span class="skill-tag">JavaScript</span>
                    <span class="skill-tag">Python</span>
                    <span class="skill-tag">C++</span>
                    <span class="skill-tag">ESP32</span>
                    <span class="skill-tag">Arduino</span>
                    <span class="skill-tag">HTML/CSS</span>
                    <span class="skill-tag">React</span>
                    <span class="skill-tag">Node.js</span>
                    <span class="skill-tag">IoT</span>
                </div>
            </div>
        </div>
        
        <div class="card">
            <div class="section">
                <h2>Featured Projects</h2>
                <div id="projects">
                    <div class="project">
                        <h3>ESP32 Advanced Portfolio</h3>
                        <p>A feature-rich portfolio with file upload, real-time sensor data, and dynamic content management.</p>
                        <p><strong>Technologies:</strong> ESP32, SPIFFS, DHT22, WebServer, JSON</p>
                    </div>
                </div>
            </div>
        </div>
    </div>
    
    <div class="esp32-badge">
        Powered by ESP32
    </div>
    
    <script src="/app.js"></script>
</body>
</html>
)rawliteral";

// About page HTML
const char* aboutHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>About - My Portfolio</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-container">
            <h2>My Portfolio</h2>
            <ul class="nav-menu">
                <li><a href="/">Home</a></li>
                <li><a href="/about" class="active">About</a></li>
                <li><a href="/projects">Projects</a></li>
                <li><a href="/sensor-data">Live Data</a></li>
                <li><a href="/admin">Admin</a></li>
            </ul>
        </div>
    </nav>

    <div class="container">
        <div class="card">
            <h1>About Me</h1>
            <div class="about-content">
                <p>I'm a passionate developer with extensive experience in embedded systems, web development, and IoT solutions. My journey began with curiosity about how things work, and has evolved into a career focused on creating innovative solutions that bridge the physical and digital worlds.</p>
                
                <h2>Experience</h2>
                <div class="experience-item">
                    <h3>IoT Developer</h3>
                    <p class="company">Tech Solutions Inc. | 2022 - Present</p>
                    <p>Developing connected devices and smart systems using ESP32, Arduino, and cloud platforms.</p>
                </div>
                
                <h2>Education</h2>
                <div class="education-item">
                    <h3>Computer Science Degree</h3>
                    <p class="institution">University Name | 2018 - 2022</p>
                </div>
                
                <h2>Certifications</h2>
                <ul>
                    <li>ESP32 Advanced Programming</li>
                    <li>Full Stack Web Development</li>
                    <li>IoT System Design</li>
                </ul>
            </div>
        </div>
    </div>
</body>
</html>
)rawliteral";

// Projects page HTML
const char* projectsHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Projects - My Portfolio</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-container">
            <h2>My Portfolio</h2>
            <ul class="nav-menu">
                <li><a href="/">Home</a></li>
                <li><a href="/about">About</a></li>
                <li><a href="/projects" class="active">Projects</a></li>
                <li><a href="/sensor-data">Live Data</a></li>
                <li><a href="/admin">Admin</a></li>
            </ul>
        </div>
    </nav>

    <div class="container">
        <div class="card">
            <h1>My Projects</h1>
            
            <div class="project-grid">
                <div class="project-card">
                    <img src="/project1.jpg" alt="Project 1" onerror="this.style.display='none'">
                    <h3>Smart Home Automation</h3>
                    <p>Complete home automation system with voice control, mobile app, and web interface.</p>
                    <div class="project-tech">
                        <span>ESP32</span>
                        <span>React</span>
                        <span>MQTT</span>
                    </div>
                    <a href="#" class="project-link">View Details</a>
                </div>
                
                <div class="project-card">
                    <img src="/project2.jpg" alt="Project 2" onerror="this.style.display='none'">
                    <h3>Weather Station Network</h3>
                    <p>Distributed weather monitoring system with real-time data visualization.</p>
                    <div class="project-tech">
                        <span>ESP32</span>
                        <span>Sensors</span>
                        <span>Cloud</span>
                    </div>
                    <a href="#" class="project-link">View Details</a>
                </div>
                
                <div class="project-card">
                    <img src="/project3.jpg" alt="Project 3" onerror="this.style.display='none'">
                    <h3>IoT Security System</h3>
                    <p>Advanced security system with facial recognition and mobile alerts.</p>
                    <div class="project-tech">
                        <span>ESP32-CAM</span>
                        <span>AI/ML</span>
                        <span>Mobile</span>
                    </div>
                    <a href="#" class="project-link">View Details</a>
                </div>
            </div>
        </div>
    </div>
</body>
</html>
)rawliteral";

// Admin panel HTML
const char* adminHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Admin - Portfolio Management</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-container">
            <h2>Admin Panel</h2>
            <ul class="nav-menu">
                <li><a href="/">Home</a></li>
                <li><a href="/about">About</a></li>
                <li><a href="/projects">Projects</a></li>
                <li><a href="/sensor-data">Live Data</a></li>
                <li><a href="/admin" class="active">Admin</a></li>
            </ul>
        </div>
    </nav>

    <div class="container">
        <div class="card">
            <h1>Portfolio Management</h1>
            
            <div class="admin-section">
                <h2>Upload Files</h2>
                <form id="uploadForm" enctype="multipart/form-data">
                    <div class="form-group">
                        <label for="fileInput">Select File:</label>
                        <input type="file" id="fileInput" name="file" accept="image/*,.css,.js,.html">
                    </div>
                    <button type="submit">Upload File</button>
                </form>
                <div id="uploadStatus"></div>
            </div>
            
            <div class="admin-section">
                <h2>Update Profile Information</h2>
                <form id="profileForm">
                    <div class="form-group">
                        <label for="name">Name:</label>
                        <input type="text" id="name" name="name" placeholder="Your Name">
                    </div>
                    <div class="form-group">
                        <label for="title">Title:</label>
                        <input type="text" id="title" name="title" placeholder="Your Professional Title">
                    </div>
                    <div class="form-group">
                        <label for="description">Description:</label>
                        <textarea id="description" name="description" placeholder="Brief description about yourself"></textarea>
                    </div>
                    <button type="submit">Update Profile</button>
                </form>
            </div>
            
            <div class="admin-section">
                <h2>System Information</h2>
                <div class="system-info">
                    <p><strong>Free Heap:</strong> <span id="freeHeap">Loading...</span></p>
                    <p><strong>Total Files:</strong> <span id="totalFiles">Loading...</span></p>
                    <p><strong>Used Space:</strong> <span id="usedSpace">Loading...</span></p>
                </div>
            </div>
        </div>
    </div>
    
    <script>
        // File upload functionality
        document.getElementById('uploadForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData();
            const fileInput = document.getElementById('fileInput');
            
            if (fileInput.files.length === 0) {
                alert('Please select a file');
                return;
            }
            
            formData.append('file', fileInput.files[0]);
            
            try {
                const response = await fetch('/upload', {
                    method: 'POST',
                    body: formData
                });
                
                const result = await response.text();
                document.getElementById('uploadStatus').innerHTML = `<p class="success">${result}</p>`;
                loadSystemInfo();
            } catch (error) {
                document.getElementById('uploadStatus').innerHTML = `<p class="error">Upload failed: ${error}</p>`;
            }
        });
        
        // Profile update functionality
        document.getElementById('profileForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData(e.target);
            const data = Object.fromEntries(formData);
            
            try {
                const response = await fetch('/update-profile', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(data)
                });
                
                if (response.ok) {
                    alert('Profile updated successfully!');
                }
            } catch (error) {
                alert('Update failed: ' + error);
            }
        });
        
        // Load system information
        function loadSystemInfo() {
            fetch('/api/system-info')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('freeHeap').textContent = data.freeHeap + ' bytes';
                    document.getElementById('totalFiles').textContent = data.totalFiles;
                    document.getElementById('usedSpace').textContent = data.usedSpace + ' bytes';
                });
        }
        
        loadSystemInfo();
        setInterval(loadSystemInfo, 5000);
    </script>
</body>
</html>
)rawliteral";

// CSS styles
const char* cssStyles = R"rawliteral(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Arial', sans-serif;
    line-height: 1.6;
    color: #333;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
}

.navbar {
    background: rgba(255, 255, 255, 0.95);
    backdrop-filter: blur(10px);
    padding: 1rem 0;
    position: sticky;
    top: 0;
    z-index: 100;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}

.nav-container {
    max-width: 1000px;
    margin: 0 auto;
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0 20px;
}

.nav-menu {
    display: flex;
    list-style: none;
    gap: 30px;
}

.nav-menu a {
    text-decoration: none;
    color: #333;
    font-weight: 500;
    transition: color 0.3s;
}

.nav-menu a:hover,
.nav-menu a.active {
    color: #667eea;
}

.container {
    max-width: 1000px;
    margin: 0 auto;
    padding: 20px;
}

.card {
    background: rgba(255, 255, 255, 0.95);
    padding: 30px;
    border-radius: 15px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.1);
    margin-bottom: 20px;
    backdrop-filter: blur(10px);
}

.header {
    text-align: center;
    margin-bottom: 40px;
}

.profile-img {
    width: 150px;
    height: 150px;
    border-radius: 50%;
    border: 5px solid #667eea;
    margin: 0 auto 20px;
    display: block;
    object-fit: cover;
}

h1 {
    color: #667eea;
    font-size: 2.5em;
    margin-bottom: 10px;
}

.subtitle {
    color: #666;
    font-size: 1.2em;
    margin-bottom: 20px;
}

.section {
    margin-bottom: 30px;
}

.section h2 {
    color: #667eea;
    border-bottom: 2px solid #667eea;
    padding-bottom: 10px;
    margin-bottom: 20px;
}

.status-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 20px;
    margin-top: 20px;
}

.status-item {
    background: #f8f9fa;
    padding: 20px;
    border-radius: 10px;
    text-align: center;
    border-left: 4px solid #667eea;
}

.status-item h3 {
    color: #667eea;
    margin-bottom: 10px;
}

.status-item p {
    font-size: 1.5em;
    font-weight: bold;
    color: #333;
}

.skills {
    display: flex;
    flex-wrap: wrap;
    gap: 10px;
}

.skill-tag {
    background: #667eea;
    color: white;
    padding: 8px 15px;
    border-radius: 20px;
    font-size: 0.9em;
    transition: transform 0.2s;
}

.skill-tag:hover {
    transform: translateY(-2px);
}

.project-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 20px;
}

.project-card {
    background: #f8f9fa;
    border-radius: 10px;
    overflow: hidden;
    transition: transform 0.3s;
}

.project-card:hover {
    transform: translateY(-5px);
}

.project-card img {
    width: 100%;
    height: 200px;
    object-fit: cover;
}

.project-card h3 {
    color: #667eea;
    padding: 15px;
    margin-bottom: 0;
}

.project-card p {
    padding: 0 15px;
    margin-bottom: 15px;
}

.project-tech {
    padding: 0 15px;
    display: flex;
    gap: 5px;
    flex-wrap: wrap;
}

.project-tech span {
    background: #667eea;
    color: white;
    padding: 4px 8px;
    border-radius: 12px;
    font-size: 0.8em;
}

.project-link {
    display: block;
    background: #667eea;
    color: white;
    text-align: center;
    padding: 10px;
    text-decoration: none;
    margin-top: 15px;
    transition: background 0.3s;
}

.project-link:hover {
    background: #5a6fd8;
}

.admin-section {
    margin-bottom: 40px;
    padding-bottom: 30px;
    border-bottom: 1px solid #eee;
}

.form-group {
    margin-bottom: 20px;
}

.form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
    color: #333;
}

.form-group input,
.form-group textarea {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
    font-size: 1em;
}

.form-group textarea {
    height: 100px;
    resize: vertical;
}

button {
    background: #667eea;
    color: white;
    padding: 12px 25px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    font-size: 1em;
    transition: background 0.3s;
}

button:hover {
    background: #5a6fd8;
}

.system-info {
    background: #f8f9fa;
    padding: 20px;
    border-radius: 10px;
}

.system-info p {
    margin-bottom: 10px;
}

.success {
    color: #28a745;
    font-weight: bold;
}

.error {
    color: #dc3545;
    font-weight: bold;
}

.esp32-badge {
    position: fixed;
    bottom: 20px;
    right: 20px;
    background: #ff6b6b;
    color: white;
    padding: 10px 15px;
    border-radius: 25px;
    font-size: 0.8em;
    font-weight: bold;
    z-index: 1000;
}

@media (max-width: 768px) {
    .nav-container {
        flex-direction: column;
        gap: 15px;
    }
    
    .nav-menu {
        gap: 15px;
    }
    
    .container {
        padding: 10px;
    }
    
    .card {
        padding: 20px;
    }
    
    h1 {
        font-size: 2em;
    }
    
    .status-grid {
        grid-template-columns: 1fr;
    }
    
    .project-grid {
        grid-template-columns: 1fr;
    }
}
)rawliteral";

// JavaScript for dynamic content
const char* jsApp = R"rawliteral(
// Load dynamic content and real-time data
document.addEventListener('DOMContentLoaded', function() {
    loadPortfolioData();
    loadSensorData();
    
    // Update sensor data every 5 seconds
    setInterval(loadSensorData, 5000);
    
    // Animate cards on load
    const cards = document.querySelectorAll('.card');
    cards.forEach((card, index) => {
        card.style.animationDelay = `${index * 0.1}s`;
        card.style.animation = 'fadeInUp 0.6s ease forwards';
    });
});

function loadPortfolioData() {
    fetch('/api/portfolio-data')
        .then(response => response.json())
        .then(data => {
            if (data.name) document.getElementById('name').textContent = data.name;
            if (data.title) document.getElementById('title').textContent = data.title;
            if (data.description) document.getElementById('description').textContent = data.description;
        })
        .catch(error => console.log('Portfolio data not available'));
}

function loadSensorData() {
    fetch('/api/sensor-data')
        .then(response => response.json())
        .then(data => {
            document.getElementById('temperature').textContent = data.temperature + '°C';
            document.getElementById('humidity').textContent = data.humidity + '%';
            document.getElementById('uptime').textContent = formatUptime(data.uptime);
            document.getElementById('memory').textContent = formatBytes(data.freeMemory);
        })
        .catch(error => {
            document.getElementById('temperature').textContent = 'N/A';
            document.getElementById('humidity').textContent = 'N/A';
        });
}

function formatUptime(milliseconds) {
    const seconds = Math.floor(milliseconds / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);
    
    if (days > 0) return `${days}d ${hours % 24}h`;
    if (hours > 0) return `${hours}h ${minutes % 60}m`;
    return `${minutes}m ${seconds % 60}s`;
}

function formatBytes(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1048576) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / 1048576).toFixed(1) + ' MB';
}

// Add CSS animation
const style = document.createElement('style');
style.textContent = `
    @keyframes fadeInUp {
        from {
            opacity: 0;
            transform: translateY(30px);
        }
        to {
            opacity: 1;
            transform: translateY(0);
        }
    }
    
    .card {
        opacity: 0;
    }
`;
document.head.appendChild(style);
)rawliteral";

void setup() {
    Serial.begin(115200);
    
    // Initialize DHT sensor
    dht.begin();
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    
    // Create default portfolio data file if it doesn't exist
    if (!SPIFFS.exists("/portfolio.json")) {
        File file = SPIFFS.open("/portfolio.json", "w");
        if (file) {
            file.println("{\"name\":\"Your Name\",\"title\":\"Full Stack Developer | IoT Enthusiast\",\"description\":\"Passionate about creating innovative solutions with modern technologies.\"}");
            file.close();
        }
    }
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Define web server routes
    
    // Main pages
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", portfolioHTML);
    });
    
    server.on("/about", HTTP_GET, []() {
        server.send(200, "text/html", aboutHTML);
    });
    
    server.on("/projects", HTTP_GET, []() {
        server.send(200, "text/html", projectsHTML);
    });
    
    server.on("/admin", HTTP_GET, []() {
        server.send(200, "text/html", adminHTML);
    });
    
    server.on("/sensor-data", HTTP_GET, []() {
        String html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Live Sensor Data</title>
            <link rel="stylesheet" href="/style.css">
            <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
        </head>
        <body>
            <nav class="navbar">
                <div class="nav-container">
                    <h2>Live Sensor Data</h2>
                    <ul class="nav-menu">
                        <li><a href="/">Home</a></li>
                        <li><a href="/about">About</a></li>
                        <li><a href="/projects">Projects</a></li>
                        <li><a href="/sensor-data" class="active">Live Data</a></li>
                        <li><a href="/admin">Admin</a></li>
                    </ul>
                </div>
            </nav>
            <div class="container">
                <div class="card">
                    <h1>Real-time Environmental Data</h1>
                    <canvas id="sensorChart" width="400" height="200"></canvas>
                </div>
            </div>
            <script>
                const ctx = document.getElementById('sensorChart').getContext('2d');
                const chart = new Chart(ctx, {
                    type: 'line',
                    data: {
                        labels: [],
                        datasets: [{
                            label: 'Temperature (°C)',
                            data: [],
                            borderColor: 'rgb(75, 192, 192)',
                            tension: 0.1
                        }, {
                            label: 'Humidity (%)',
                            data: [],
                            borderColor: 'rgb(255, 99, 132)',
                            tension: 0.1
                        }]
                    },
                    options: {
                        responsive: true,
                        scales: {
                            y: {
                                beginAtZero: true
                            }
                        }
                    }
                });
                
                function updateChart() {
                    fetch('/api/sensor-data')
                        .then(response => response.json())
                        .then(data => {
                            const now = new Date().toLocaleTimeString();
                            chart.data.labels.push(now);
                            chart.data.datasets[0].data.push(data.temperature);
                            chart.data.datasets[1].data.push(data.humidity);
                            
                            if (chart.data.labels.length > 20) {
                                chart.data.labels.shift();
                                chart.data.datasets[0].data.shift();
                                chart.data.datasets[1].data.shift();
                            }
                            
                            chart.update();
                        });
                }
                
                updateChart();
                setInterval(updateChart, 2000);
            </script>
        </body>
        </html>
        )rawliteral";
        server.send(200, "text/html", html);
    });
    
    // Static files
    server.on("/style.css", HTTP_GET, []() {
        server.send(200, "text/css", cssStyles);
    });
    
    server.on("/app.js", HTTP_GET, []() {
        server.send(200, "application/javascript", jsApp);
    });
    
    // API endpoints
    server.on("/api/sensor-data", HTTP_GET, []() {
        DynamicJsonDocument doc(200);
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["uptime"] = millis();
        doc["freeMemory"] = ESP.getFreeHeap();
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    
    server.on("/api/portfolio-data", HTTP_GET, []() {
        File file = SPIFFS.open("/portfolio.json", "r");
        if (file) {
            String content = file.readString();
            file.close();
            server.send(200, "application/json", content);
        } else {
            server.send(404, "application/json", "{\"error\":\"Portfolio data not found\"}");
        }
    });
    
    server.on("/api/system-info", HTTP_GET, []() {
        DynamicJsonDocument doc(300);
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["usedSpace"] = SPIFFS.usedBytes();
        doc["totalSpace"] = SPIFFS.totalBytes();
        
        // Count files
        int fileCount = 0;
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file) {
            fileCount++;
            file = root.openNextFile();
        }
        doc["totalFiles"] = fileCount;
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    
    // File upload endpoint
    server.on("/upload", HTTP_POST, []() {
        server.send(200, "text/plain", "Upload complete");
    }, []() {
        HTTPUpload& upload = server.upload();
        static File uploadFile;
        
        if (upload.status == UPLOAD_FILE_START) {
            String filename = "/" + upload.filename;
            uploadFile = SPIFFS.open(filename, "w");
            if (!uploadFile) {
                Serial.println("Failed to open file for writing");
                return;
            }
            Serial.println("Upload started: " + filename);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (uploadFile) {
                uploadFile.write(upload.buf, upload.currentSize);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (uploadFile) {
                uploadFile.close();
                Serial.println("Upload completed: " + String(upload.totalSize) + " bytes");
            }
        }
    });
    
    // Profile update endpoint
    server.on("/update-profile", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            String body = server.arg("plain");
            File file = SPIFFS.open("/portfolio.json", "w");
            if (file) {
                file.print(body);
                file.close();
                server.send(200, "text/plain", "Profile updated successfully");
            } else {
                server.send(500, "text/plain", "Failed to save profile");
            }
        } else {
            server.send(400, "text/plain", "No data received");
        }
    });
    
    // Serve uploaded files
    server.onNotFound([]() {
        String path = server.uri();
        
        // Check if file exists in SPIFFS
        if (SPIFFS.exists(path)) {
            File file = SPIFFS.open(path, "r");
            if (file) {
                String contentType = getContentType(path);
                server.streamFile(file, contentType);
                file.close();
                return;
            }
        }
        
        server.send(404, "text/plain", "File not found");
    });
    
    // Start server
    server.begin();
    Serial.println("HTTP server started");
    Serial.println("Access your portfolio at: http://" + WiFi.localIP().toString());
    Serial.println("Admin panel at: http://" + WiFi.localIP().toString() + "/admin");
}

void loop() {
    server.handleClient();
    
    // Read sensor data every 2 seconds
    if (millis() - lastSensorRead > 2000) {
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        
        // Check if readings are valid
        if (isnan(temperature)) temperature = 0.0;
        if (isnan(humidity)) humidity = 0.0;
        
        lastSensorRead = millis();
    }
    
    delay(2);
}