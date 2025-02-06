const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <title>環境データ</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <!-- Chart.js CDN -->
  <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.2.0/chart.min.js"
          integrity="sha512-VMsZqo0ar06BMtg0tPsdgRADvl0kDHpTbugCBBrL55KmucH6hP9zWdLIWY//OTfMnzz6xWQRxQqsUFefwHuHyg=="
          crossorigin="anonymous"></script>
  <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns@next/dist/chartjs-adapter-date-fns.bundle.min.js"></script>
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background-color: #f9f9f9; 
      margin: 0; 
      padding: 20px; 
      text-align: center; 
    }
    h1 { 
      color: #333; 
      margin-bottom: 20px; 
    }
    .chart-container { 
      display: inline-block; 
      vertical-align: top; 
      margin: 10px; 
      background-color: #fff; 
      padding: 10px; 
      border: 1px solid #ddd; 
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
    #tempChartContainer { width: 600px; }
    #discomfortChartContainer { width: 300px; }
    #discomfortText { 
      text-align: center; 
      font-size: 1.2em; 
      margin-top: 10px; 
    }
    #clearBtn {
      margin-top: 20px;
      padding: 10px 20px;
      font-size: 1em;
      background-color: #f88;
      border: none;
      color: #fff;
      cursor: pointer;
      border-radius: 5px;
    }
    #clearBtn:hover {
      background-color: #e66;
    }
  </style>
</head>
<body>
  <h1>環境データ</h1>
  
  <!-- 温度・湿度・大気圧グラフのコンテナ -->
  <div id="tempChartContainer" class="chart-container">
    <canvas id="tempChart"></canvas>
  </div>
  
  <!-- 不快指数グラフのコンテナ -->
  <div id="discomfortChartContainer" class="chart-container">
    <canvas id="discomfortChart"></canvas>
    <div id="discomfortText">不快指数：-</div>
  </div>
  
  <!-- localStorage のデータを削除するボタン -->
  <div>
    <button id="clearBtn" onclick="clearChartData()">データクリア</button>
  </div>
  
  <script>
    const MAX_POINTS = 30;
    
    // 不快指数に応じた色を返す
    function getDiscomfortColor(value) {
      if (value < 55) {
        return '#0000FF';
      } else if (value < 65) {
        let ratio = (value - 55) / 10;
        let r = 0;
        let g = Math.round(255 * ratio);
        let b = Math.round(255 * (1 - ratio));
        return `rgb(${r},${g},${b})`;
      } else if (value < 85) {
        let ratio = (value - 65) / 20;
        let r = Math.round(255 * ratio);
        let g = Math.round(255 * (1 - ratio));
        let b = 0;
        return `rgb(${r},${g},${b})`;
      } else {
        return '#FF0000';
      }
    }
    
    // 不快指数の値に応じた指標を返す
    function getDiscomfortText(value) {
      if (value < 55) {
        return "寒い";
      } else if (value < 60) {
        return "肌寒い";
      } else if (value < 65) {
        return "何も感じない";
      } else if (value < 70) {
        return "快い";
      } else if (value < 75) {
        return "暑くない";
      } else if (value < 80) {
        return "やや暑い";
      } else if (value < 85) {
        return "暑くて汗が出る";
      } else {
        return "暑くてたまらない";
      }
    }
    
    // 温度・湿度・大気圧グラフの初期化
    var tempCtx = document.getElementById('tempChart').getContext('2d');
    var tempChart = new Chart(tempCtx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [
          {
            label: 'Temperature (°C)',
            data: [],
            borderColor: '#f88',
            backgroundColor: 'rgba(255,136,136,0.2)',
            fill: true,
            tension: 0.1,
            yAxisID: 'temp'
          },
          {
            label: 'Humidity (%)',
            data: [],
            borderColor: '#88f',
            backgroundColor: 'rgba(136,136,255,0.2)',
            fill: true,
            tension: 0.1,
            yAxisID: 'humid'
          },
          {
            label: 'Pressure (hPa)',
            data: [],
            borderColor: '#8f8',
            backgroundColor: 'rgba(136,255,136,0.2)',
            fill: true,
            tension: 0.1,
            yAxisID: 'press'
          }
        ]
      },
      options: {
        scales: {
          x: {
            type: 'time',
            time: {
              parser: 'HH:mm',
              unit: 'minute',
              displayFormats: { minute: 'HH:mm' }
            },
            title: { display: true, text: 'Time' }
          },
          temp: {
            type: 'linear',
            position: 'left',
            title: { display: true, text: 'Temperature (°C)' },
            suggestedMin: 15,
            suggestedMax: 30
          },
          humid: {
            type: 'linear',
            position: 'left',
            title: { display: true, text: 'Humidity (%)' },
            suggestedMin: 0,
            suggestedMax: 100,
            grid: { drawOnChartArea: false }
          },
          press: {
            type: 'linear',
            position: 'right',
            title: { display: true, text: 'Pressure (hPa)' },
            suggestedMin: 900,
            suggestedMax: 1100,
            grid: { drawOnChartArea: false }
          }
        }
      }
    });
    
    // 不快指数グラフの初期化
    var dctx = document.getElementById('discomfortChart').getContext('2d');
    var discomfortChart = new Chart(dctx, {
      type: 'bar',
      data: {
        labels: ['Discomfort Index'],
        datasets: [{
          label: 'Discomfort Index',
          data: [0],
          backgroundColor: function(context) {
            const value = context.dataset.data[context.dataIndex];
            return getDiscomfortColor(value);
          },
          borderWidth: 1
        }]
      },
      options: {
        scales: {
          y: {
            min: 0,
            max: 100,
            title: { display: true, text: 'Discomfort Index (0-100)' }
          }
        }
      }
    });
    
    // ローカルストレージからグラフデータを復元する関数
    function loadChartData() {
      const savedLabels = localStorage.getItem('chartLabels');
      const savedTemp = localStorage.getItem('chartTemperature');
      const savedHumid = localStorage.getItem('chartHumidity');
      const savedPress = localStorage.getItem('chartPressure');
      const savedDiscomfort = localStorage.getItem('chartDiscomfort');
      if (savedLabels) {
        try {
          tempChart.data.labels = JSON.parse(savedLabels);
        } catch (e) {
          console.error('Error parsing chartLabels', e);
        }
      }
      if (savedTemp) {
        try {
          tempChart.data.datasets[0].data = JSON.parse(savedTemp);
        } catch (e) {
          console.error('Error parsing chartTemperature', e);
        }
      }
      if (savedHumid) {
        try {
          tempChart.data.datasets[1].data = JSON.parse(savedHumid);
        } catch (e) {
          console.error('Error parsing chartHumidity', e);
        }
      }
      if (savedPress) {
        try {
          tempChart.data.datasets[2].data = JSON.parse(savedPress);
        } catch (e) {
          console.error('Error parsing chartPressure', e);
        }
      }
      if (savedDiscomfort) {
        try {
          discomfortChart.data.datasets[0].data[0] = JSON.parse(savedDiscomfort);
        } catch (e) {
          console.error('Error parsing chartDiscomfort', e);
        }
      }
    }
    
    // グラフデータをローカルストレージに保存する関数
    function saveChartData() {
      localStorage.setItem('chartLabels', JSON.stringify(tempChart.data.labels));
      localStorage.setItem('chartTemperature', JSON.stringify(tempChart.data.datasets[0].data));
      localStorage.setItem('chartHumidity', JSON.stringify(tempChart.data.datasets[1].data));
      localStorage.setItem('chartPressure', JSON.stringify(tempChart.data.datasets[2].data));
      localStorage.setItem('chartDiscomfort', JSON.stringify(discomfortChart.data.datasets[0].data[0]));
    }
    
    // ローカルストレージのデータを削除する関数
    function clearChartData() {
      localStorage.removeItem('chartLabels');
      localStorage.removeItem('chartTemperature');
      localStorage.removeItem('chartHumidity');
      localStorage.removeItem('chartPressure');
      localStorage.removeItem('chartDiscomfort');
      tempChart.data.labels = [];
      tempChart.data.datasets.forEach(ds => ds.data = []);
      discomfortChart.data.datasets[0].data[0] = 0;
      tempChart.update();
      discomfortChart.update();
      document.getElementById('discomfortText').textContent = "不快指数：-";
      console.log('localStorage data cleared.');
    }
    
    async function updateData() {
      try {
        const r = await fetch('/data');
        if (!r.ok) {
          throw new Error(r.statusText);
        }
        const d = await r.json();
        console.log('Fetched data:', d);
        const t = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', hour12: false });
        
        // 温度・湿度・大気圧グラフの更新
        if (d.temperature !== undefined && d.humidity !== undefined && d.pressure !== undefined) {
          tempChart.data.labels.push(t);
          tempChart.data.datasets[0].data.push(d.temperature);
          tempChart.data.datasets[1].data.push(d.humidity);
          tempChart.data.datasets[2].data.push(d.pressure);
          if (tempChart.data.labels.length > MAX_POINTS) {
            tempChart.data.labels.shift();
            tempChart.data.datasets.forEach(ds => ds.data.shift());
          }
          tempChart.update();
        } else {
          console.warn('One or more sensor data (temperature, humidity, pressure) are undefined.');
        }
        
        // 不快指数の更新
        if (d.discomfort_index !== undefined) {
          discomfortChart.data.datasets[0].data.push(d.discomfort_index);
          discomfortChart.update();
          document.getElementById('discomfortText').textContent = "不快指数: " + d.discomfort_index + " (" + getDiscomfortText(d.discomfort_index) + ")";
        } else {
          console.warn('Discomfort index data is undefined.');
        }
        saveChartData();
      } catch (e) {
        console.error('Error fetching data:', e);
      }
    }
    window.addEventListener('DOMContentLoaded', function() {
      loadChartData();
      updateData();
      setInterval(updateData, 60000);
    });
  </script>
</body>
</html>
)rawliteral";
