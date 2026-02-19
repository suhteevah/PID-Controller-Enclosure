// ESP-Nail v2 Web Dashboard Application
(function() {
    'use strict';

    const API = {
        get:  (url) => fetch(url).then(r => r.json()),
        post: (url, data) => fetch(url, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(data)
        }).then(r => r.json())
    };

    let ws = null;
    let numChannels = 1;
    let channelData = [];
    let currentPage = 'dashboard';

    // --- WebSocket ---
    function connectWS() {
        const proto = location.protocol === 'https:' ? 'wss' : 'ws';
        ws = new WebSocket(proto + '://' + location.host + '/ws');
        ws.onopen = () => document.getElementById('wifi-icon').style.opacity = '1';
        ws.onclose = () => {
            document.getElementById('wifi-icon').style.opacity = '0.3';
            setTimeout(connectWS, 3000);
        };
        ws.onmessage = (e) => {
            try {
                const msg = JSON.parse(e.data);
                if (msg.type === 'temps') updateChannels(msg.channels);
            } catch(err) { /* ignore malformed */ }
        };
    }

    // --- Channel Rendering ---
    function initChannels(n) {
        numChannels = n;
        const container = document.getElementById('channels');
        container.innerHTML = '';
        for (let i = 0; i < n; i++) {
            channelData[i] = { temp: 0, target: 710, state: 'OFF', output: 0 };
            container.innerHTML += createChannelCard(i);
        }
        bindChannelEvents();
    }

    function createChannelCard(i) {
        return `
        <div class="channel-card" id="ch-${i}">
            <div class="ch-header">
                <span class="ch-label">Channel ${i + 1}</span>
                <span class="ch-state state-off" id="ch-${i}-state">OFF</span>
            </div>
            <div class="ch-temp">
                <span class="temp-current" id="ch-${i}-temp">---</span>
                <span class="temp-unit">&deg;F</span>
                <span class="temp-target" id="ch-${i}-target-label">/ 710&deg;F</span>
            </div>
            <div class="temp-bar">
                <div class="temp-bar-fill" id="ch-${i}-bar" style="width:0%"></div>
            </div>
            <div class="temp-slider-row">
                <button class="btn btn-sm" onclick="adjustTemp(${i},-5)">-</button>
                <input type="range" class="temp-slider" id="ch-${i}-slider"
                    min="0" max="999" value="710" oninput="sliderChange(${i},this.value)">
                <button class="btn btn-sm" onclick="adjustTemp(${i},5)">+</button>
                <span class="temp-display" id="ch-${i}-slider-val">710</span>
            </div>
            <div class="ch-controls">
                <button class="btn btn-toggle btn-primary" id="ch-${i}-toggle"
                    onclick="toggleChannel(${i})">Start</button>
            </div>
        </div>`;
    }

    function updateChannels(channels) {
        channels.forEach((ch, i) => {
            if (i >= numChannels) return;
            channelData[i] = ch;
            const card = document.getElementById('ch-' + i);
            const stateEl = document.getElementById('ch-' + i + '-state');
            const tempEl = document.getElementById('ch-' + i + '-temp');
            const targetLabel = document.getElementById('ch-' + i + '-target-label');
            const bar = document.getElementById('ch-' + i + '-bar');
            const toggle = document.getElementById('ch-' + i + '-toggle');

            // Temperature
            tempEl.textContent = ch.temp > 0 ? Math.round(ch.temp) : '---';

            // State
            const stateLower = ch.state.toLowerCase();
            stateEl.textContent = ch.state;
            stateEl.className = 'ch-state state-' + stateLower;

            // Card styling
            card.className = 'channel-card';
            if (stateLower === 'heating' || stateLower === 'holding') card.classList.add('active');
            if (stateLower === 'fault') card.classList.add('fault');

            // Target label
            targetLabel.textContent = '/ ' + Math.round(ch.target) + '\u00B0F';

            // Bar
            const pct = Math.min(100, Math.max(0, (ch.temp / ch.target) * 100));
            bar.style.width = pct + '%';
            if (pct < 80) bar.style.background = 'var(--warn)';
            else if (pct < 105) bar.style.background = 'var(--success)';
            else bar.style.background = 'var(--danger)';

            // Toggle button
            if (stateLower === 'off') {
                toggle.textContent = 'Start';
                toggle.className = 'btn btn-toggle btn-primary';
            } else {
                toggle.textContent = 'Stop';
                toggle.className = 'btn btn-toggle btn-danger';
            }
        });
    }

    function bindChannelEvents() {
        // Event listeners bound via inline onclick for simplicity
    }

    // --- Global Functions ---
    window.toggleChannel = function(ch) {
        const isOff = channelData[ch].state === 'OFF';
        API.post('/api/channel/' + ch + '/state', { state: isOff ? 'ON' : 'OFF' });
    };

    window.adjustTemp = function(ch, delta) {
        const slider = document.getElementById('ch-' + ch + '-slider');
        let val = parseInt(slider.value) + delta;
        val = Math.max(0, Math.min(999, val));
        slider.value = val;
        window.sliderChange(ch, val);
    };

    window.sliderChange = function(ch, val) {
        document.getElementById('ch-' + ch + '-slider-val').textContent = val;
        document.getElementById('ch-' + ch + '-target-label').textContent = '/ ' + val + '\u00B0F';
        // Debounced send
        clearTimeout(window['_sliderTimer' + ch]);
        window['_sliderTimer' + ch] = setTimeout(() => {
            API.post('/api/channel/' + ch + '/target', { temp: parseInt(val) });
        }, 300);
    };

    // --- Navigation ---
    document.querySelectorAll('.nav-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const page = btn.dataset.page;
            switchPage(page);
        });
    });

    function switchPage(page) {
        currentPage = page;
        document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
        document.querySelector('[data-page="' + page + '"]').classList.add('active');

        // Show/hide
        document.getElementById('channels').classList.toggle('hidden', page !== 'dashboard');
        ['profiles', 'settings', 'logs'].forEach(p => {
            document.getElementById('page-' + p).classList.toggle('hidden', page !== p);
        });

        if (page === 'profiles') loadProfiles();
        if (page === 'logs') loadLogs();
        if (page === 'settings') loadSettings();
    }

    // --- Profiles ---
    async function loadProfiles() {
        try {
            const data = await API.get('/api/profiles');
            const list = document.getElementById('profile-list');
            list.innerHTML = '';
            data.profiles.forEach(p => {
                list.innerHTML += `
                <div class="profile-item">
                    <span class="profile-name">${escapeHtml(p.name)}</span>
                    <span class="profile-temp">${p.temp}&deg;F</span>
                </div>`;
            });
        } catch(e) { /* offline */ }
    }

    // --- Logs ---
    async function loadLogs() {
        try {
            const data = await API.get('/api/logs');
            const list = document.getElementById('log-list');
            list.innerHTML = '';
            data.logs.forEach(l => {
                list.innerHTML += `
                <div class="log-item">
                    <div class="log-date">${l.date}</div>
                    <div class="log-stats">Duration: ${l.duration}min &bull; Avg: ${l.avgTemp}&deg;F</div>
                </div>`;
            });
        } catch(e) { /* offline */ }
    }

    // --- Settings ---
    async function loadSettings() {
        try {
            const data = await API.get('/api/settings');
            if (data.wifi_ssid) document.getElementById('wifi-ssid').value = data.wifi_ssid;
            if (data.mqtt_host) document.getElementById('mqtt-host').value = data.mqtt_host;
            if (data.mqtt_port) document.getElementById('mqtt-port').value = data.mqtt_port;
            if (data.idle_timeout) document.getElementById('idle-timeout').value = data.idle_timeout;
            document.getElementById('fw-version').textContent = 'v' + (data.fw_version || '--');
        } catch(e) { /* offline */ }
    }

    document.getElementById('btn-save-wifi').addEventListener('click', () => {
        API.post('/api/settings/wifi', {
            ssid: document.getElementById('wifi-ssid').value,
            password: document.getElementById('wifi-pass').value
        });
    });

    document.getElementById('btn-save-mqtt').addEventListener('click', () => {
        API.post('/api/settings/mqtt', {
            host: document.getElementById('mqtt-host').value,
            port: parseInt(document.getElementById('mqtt-port').value)
        });
    });

    document.getElementById('btn-save-safety').addEventListener('click', () => {
        API.post('/api/settings/safety', {
            idle_timeout: parseInt(document.getElementById('idle-timeout').value)
        });
    });

    // --- OTA ---
    document.getElementById('btn-ota').addEventListener('click', () => {
        document.getElementById('fw-file').click();
    });

    document.getElementById('fw-file').addEventListener('change', async (e) => {
        const file = e.target.files[0];
        if (!file) return;
        const progress = document.getElementById('ota-progress');
        const bar = document.getElementById('ota-bar');
        progress.classList.remove('hidden');

        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/api/ota');
        xhr.upload.onprogress = (ev) => {
            if (ev.lengthComputable) {
                bar.style.width = Math.round(ev.loaded / ev.total * 100) + '%';
            }
        };
        xhr.onload = () => {
            bar.style.width = '100%';
            bar.style.background = 'var(--success)';
            setTimeout(() => location.reload(), 3000);
        };
        xhr.onerror = () => {
            bar.style.background = 'var(--danger)';
        };
        xhr.send(file);
    });

    // --- Utility ---
    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    // --- Init ---
    async function init() {
        try {
            const info = await API.get('/api/info');
            numChannels = info.channels || 1;
            document.getElementById('model-badge').textContent = info.model || 'ESP-Nail';
        } catch(e) {
            numChannels = 1;
        }
        initChannels(numChannels);
        connectWS();

        // Uptime counter
        let uptimeS = 0;
        setInterval(() => {
            uptimeS++;
            const h = Math.floor(uptimeS / 3600);
            const m = Math.floor((uptimeS % 3600) / 60);
            document.getElementById('uptime').textContent =
                h + ':' + String(m).padStart(2, '0');
        }, 1000);
    }

    // Register service worker
    if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('/sw.js').catch(() => {});
    }

    init();
})();
