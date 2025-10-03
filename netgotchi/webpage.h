// ============================================================================
// NETGOTCHI WEB INTERFACE
// ============================================================================
// Matrix-style cyberpunk web interface for Netgotchi
// Separated from main code for better organization and easier editing

#ifndef WEBPAGE_H
#define WEBPAGE_H

static const char PROGMEM pagehtml[] = R"rawliteral( 
<!DOCTYPE html>
<html>
<head>
    <title>Netgotchi Matrix Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            background: linear-gradient(135deg, #0a0e27 0%, #1a1f3a 100%);
            color: #00ff41;
            font-family: 'Courier New', monospace;
            padding: 20px;
            min-height: 100vh;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
        }

        h1 {
            text-align: center;
            font-size: 3em;
            color: #00ff41;
            text-shadow: 0 0 10px #00ff41, 0 0 20px #00ff41, 0 0 30px #00ff41;
            margin-bottom: 10px;
            animation: glow 2s ease-in-out infinite alternate;
        }

        @keyframes glow {
            from { text-shadow: 0 0 10px #00ff41, 0 0 20px #00ff41, 0 0 30px #00ff41; }
            to { text-shadow: 0 0 20px #00ff41, 0 0 30px #00ff41, 0 0 40px #00ff41; }
        }

        .subtitle {
            text-align: center;
            color: #0f0;
            font-size: 1.2em;
            margin-bottom: 30px;
            opacity: 0.8;
        }

        .card {
            background: rgba(0, 20, 40, 0.8);
            border: 2px solid #00ff41;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 0 20px rgba(0, 255, 65, 0.3);
            backdrop-filter: blur(10px);
        }

        .card h2 {
            color: #00ff41;
            margin-bottom: 15px;
            font-size: 1.5em;
            border-bottom: 1px solid #00ff41;
            padding-bottom: 10px;
        }

        canvas {
            image-rendering: pixelated;
            border: 2px solid #00ff41;
            border-radius: 5px;
            box-shadow: 0 0 15px rgba(0, 255, 65, 0.5);
            background: #000;
            display: block;
            margin: 0 auto;
            transform: scale(2);
            transform-origin: center;
        }

        .canvas-container {
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 40px 20px;
            background: #000;
            border-radius: 5px;
            margin: 15px 0;
        }

        button {
            background: linear-gradient(135deg, #003300 0%, #006600 100%);
            color: #00ff41;
            border: 2px solid #00ff41;
            padding: 12px 24px;
            margin: 5px;
            border-radius: 5px;
            cursor: pointer;
            font-family: 'Courier New', monospace;
            font-size: 14px;
            font-weight: bold;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
            box-shadow: 0 0 10px rgba(0, 255, 65, 0.3);
        }

        button:hover {
            background: linear-gradient(135deg, #006600 0%, #00aa00 100%);
            box-shadow: 0 0 20px rgba(0, 255, 65, 0.6);
            transform: translateY(-2px);
        }

        button:active {
            transform: translateY(0);
            box-shadow: 0 0 10px rgba(0, 255, 65, 0.3);
        }

        .button-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 10px;
            margin: 15px 0;
        }

        .switch-container {
            display: flex;
            align-items: center;
            justify-content: space-between;
            background: rgba(0, 50, 0, 0.3);
            padding: 15px;
            border-radius: 5px;
            border: 1px solid #00ff41;
            margin: 10px 0;
        }

        .switch-label {
            color: #00ff41;
            font-size: 16px;
            font-weight: bold;
        }

        input[type="checkbox"] {
            appearance: none;
            width: 60px;
            height: 30px;
            background: #333;
            border: 2px solid #00ff41;
            border-radius: 15px;
            position: relative;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        input[type="checkbox"]::before {
            content: '';
            position: absolute;
            width: 22px;
            height: 22px;
            border-radius: 50%;
            background: #666;
            top: 2px;
            left: 2px;
            transition: all 0.3s ease;
        }

        input[type="checkbox"]:checked {
            background: #00aa00;
            box-shadow: 0 0 15px rgba(0, 255, 65, 0.5);
        }

        input[type="checkbox"]:checked::before {
            background: #00ff41;
            left: 32px;
            box-shadow: 0 0 10px #00ff41;
        }

        .status-badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 14px;
            font-weight: bold;
            text-transform: uppercase;
        }

        .status-enabled {
            background: rgba(0, 255, 0, 0.2);
            color: #0f0;
            border: 1px solid #0f0;
            box-shadow: 0 0 10px rgba(0, 255, 0, 0.3);
        }

        .status-disabled {
            background: rgba(255, 0, 0, 0.2);
            color: #f00;
            border: 1px solid #f00;
            box-shadow: 0 0 10px rgba(255, 0, 0, 0.3);
        }

        #headless {
            background: #000;
            color: #0f0;
            padding: 20px;
            border-radius: 5px;
            font-size: 1.2em;
            border: 1px solid #00ff41;
            line-height: 1.6;
            word-wrap: break-word;
            box-shadow: inset 0 0 20px rgba(0, 255, 65, 0.1);
        }

        #hosts {
            background: rgba(0, 0, 0, 0.5);
            padding: 15px;
            border-radius: 5px;
            border: 1px solid #00ff41;
            margin-top: 15px;
            color: #0f0;
            font-size: 14px;
            max-height: 400px;
            overflow-y: auto;
        }

        #hosts::-webkit-scrollbar {
            width: 10px;
        }

        #hosts::-webkit-scrollbar-track {
            background: #111;
            border-radius: 5px;
        }

        #hosts::-webkit-scrollbar-thumb {
            background: #00ff41;
            border-radius: 5px;
        }

        .grid-2col {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }

        @media (max-width: 768px) {
            h1 {
                font-size: 2em;
            }
            .button-grid {
                grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
            }
        }

        .blink {
            animation: blink-animation 1s steps(2, start) infinite;
        }

        @keyframes blink-animation {
            to { visibility: hidden; }
        }

        footer {
            text-align: center;
            margin-top: 30px;
            padding: 20px;
            border-top: 1px solid #00ff41;
            opacity: 0.7;
        }

        footer p {
            margin: 5px 0;
            color: #00ff41;
            font-size: 14px;
        }

        footer a {
            color: #00ff41;
            text-decoration: none;
            transition: all 0.3s ease;
            font-weight: bold;
        }

        footer a:hover {
            color: #00aa00;
            text-shadow: 0 0 10px #00ff41;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>NETGOTCHI</h1>
    <div class="subtitle">[ NETWORK SECURITY MATRIX ]</div>

    <div class="card">
        <h2>STATUS MONITOR</h2>
        <div id="headless">Initializing...</div>
    </div>

    <div class="grid-2col">
        <div class="card">
            <h2>DISPLAY OUTPUT</h2>
            <div class="canvas-container">
                <canvas id="canvas" width="128" height="64"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>CONTROLS</h2>
            <div class="button-grid">
                <button onclick="sendCommand('left')">LEFT</button>
                <button onclick="sendCommand('right')">RIGHT</button>
                <button onclick="sendCommand('A')">A</button>
                <button onclick="sendCommand('B')">B</button>
                <button onclick="sendCommand('ON')">PIN ON</button>
                <button onclick="sendCommand('OFF')">PIN OFF</button>
                <button onclick="sendCommand('TIMEPLUS')">TIME +</button>
                <button onclick="sendCommand('TIMEMINUS')">TIME -</button>
            </div>
        </div>
    </div>

    <div class="card">
        <h2>SETTINGS</h2>
        <div class="switch-container">
            <span class="switch-label">Evil Twin Detection</span>
            <div style="display: flex; align-items: center; gap: 15px;">
                <input type="checkbox" id="evilTwinToggle" onchange="toggleEvilTwin()">
                <span id="evilTwinStatus" class="status-badge">---</span>
            </div>
        </div>
        <div class="switch-container">
            <span class="switch-label">External Pin (D0)</span>
            <div style="display: flex; align-items: center; gap: 15px;">
                <span id="pinStatus" class="status-badge">---</span>
            </div>
        </div>
    </div>

    <div class="card">
        <h2>NETWORK HOSTS</h2>
        <button onclick="getHosts()" style="width: 100%;">SCAN HOSTS</button>
        <div id="hosts">Click 'SCAN HOSTS' to retrieve network data...</div>
    </div>

    <footer>
        <p>Netgotchi by MXZZ</p>
        <p>
            <a href="https://github.com/MXZZ/Netgotchi/" target="_blank">
                https://github.com/MXZZ/Netgotchi/
            </a>
        </p>
    </footer>
</div>
    <script>
        function updateCanvas() {
            fetch('/matrix')
                .then(response => response.json())
                .then(matrix => {
                    const canvas = document.getElementById('canvas');
                    const ctx = canvas.getContext('2d');
                    ctx.fillStyle = 'black';
                    ctx.fillRect(0, 0, canvas.width, canvas.height);
                    ctx.fillStyle = 'white';
                    for (let y = 0; y < matrix.length; y++) {
                        for (let x = 0; x < matrix[y].length; x++) {
                            if (matrix[y][x] === 1) {
                                ctx.fillRect(x, y, 1, 1);
                            }
                        }
                    }
                });
        }

        function sendCommand(command) {
            fetch('/command/' + command)
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                    // Update pin status immediately after ON/OFF commands
                    if (command === 'ON' || command === 'OFF') {
                        setTimeout(getPinStatus, 100); // Small delay to ensure pin state is set
                    }
                })
                .catch(error => console.error('Error:', error));
        }

        function getHosts() {
            fetch('/hosts' )
                .then(response => response.text())
                .then(response => document.getElementById('hosts').innerHTML= response)
                .catch(error => console.error('Error:', error));
        }
         function getHeadlessStatus() {
            fetch('/headless' )
                .then(response => response.text())
                .then(response => document.getElementById('headless').innerHTML= response)
                .catch(error => console.error('Error:', error));
        }

        function getEvilTwinStatus() {
            fetch('/eviltwin/status')
                .then(response => response.json())
                .then(data => {
                    const checkbox = document.getElementById('evilTwinToggle');
                    const statusText = document.getElementById('evilTwinStatus');
                    checkbox.checked = data.enabled;
                    statusText.textContent = data.enabled ? 'ENABLED' : 'DISABLED';
                    statusText.className = data.enabled ? 'status-badge status-enabled' : 'status-badge status-disabled';
                })
                .catch(error => console.error('Error:', error));
        }

        function toggleEvilTwin() {
            const checkbox = document.getElementById('evilTwinToggle');
            const newState = checkbox.checked ? 'enable' : 'disable';
            
            fetch('/eviltwin/' + newState)
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                    getEvilTwinStatus(); // Update status display
                })
                .catch(error => console.error('Error:', error));
        }

        function getPinStatus() {
            fetch('/pin/status')
                .then(response => response.json())
                .then(data => {
                    const statusText = document.getElementById('pinStatus');
                    const isHigh = data.state === 'HIGH';
                    statusText.textContent = isHigh ? 'HIGH (ON)' : 'LOW (OFF)';
                    statusText.className = isHigh ? 'status-badge status-enabled' : 'status-badge status-disabled';
                })
                .catch(error => console.error('Error:', error));
        }

        // Update the canvas every 2 second
        setInterval(updateCanvas, 2000);

        setInterval(getHeadlessStatus, 2000);
        
        // Update Evil Twin status every 5 seconds
        setInterval(getEvilTwinStatus, 5000);
        
        // Update Pin status every 3 seconds
        setInterval(getPinStatus, 3000);

        // Initial updates
        updateCanvas();
        getEvilTwinStatus();
        getPinStatus();
    </script>
</body></html>
)rawliteral";

#endif // WEBPAGE_H

