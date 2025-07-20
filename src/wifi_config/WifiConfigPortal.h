#pragma once

constexpr char WIFI_CONFIG_PORTAL[] PROGMEM = R"rawliteral(
<html><head><title>ESP32 WiFi Setup</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>body{font-family: sans-serif; background: #f0f0f0; text-align: center;}
div{background: white; padding: 20px; margin: 20px auto; max-width: 400px; border-radius: 8px;}
select, input, button {width: 90%; padding: 12px; margin: 8px 0; border: 1px solid #ccc; border-radius: 4px;}
button{background: #3498db; color: white; border: none; cursor: pointer;}</style></head>
<body><div><h1>WiFi Setup</h1>
<p>Please select a network and enter the password.</p>
<form action='/save' method='POST'>
<select id='ssid' name='ssid'><option value=''>Scanning...</option></select>
<input type='password' name='password' placeholder='Password'>
<button type='submit'>Save & Connect</button>
</form>
<script>
window.addEventListener('load', function() {
  fetch('/scan')
    .then(response => response.json())
    .then(data => {
      var select = document.getElementById('ssid');
      select.innerHTML = '';
      if (data.length === 0) {
        select.innerHTML = '<option value="">No networks found</option>';
      } else {
        data.sort((a, b) => b.rssi - a.rssi);
        data.forEach(net => {
          var opt = document.createElement('option');
          opt.value = net.ssid;
          opt.textContent = `${net.ssid} (${net.rssi} dBm)`;
          select.appendChild(opt);
        });
      }
    })
    .catch(error => {
      console.error('Error fetching networks:', error);
      document.getElementById('ssid').innerHTML = '<option value="">Scan failed</option>';
    });
});
</script>
</div></body></html>
)rawliteral";
