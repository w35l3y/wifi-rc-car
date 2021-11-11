#ifndef APSSID
#define APSSID "WiFi-RC-Car"
#define APPSWD "wifirccar"
#endif

const char* REMOTE_CONTROL PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
    <title>
        WIFI RC CAR
    </title>
    <meta name="viewport" content="user-scalable=no">
</head>
<body  style="position: fixed; font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
color:rgb(128, 128, 128);
text-align:center;
font-size: xx-large;">
    <h1>WIFI RC CAR</h1>
    <p>
      Speed: <span id="speed"> </span> %
      Angle: <span id="angle"> </span>
    </p>
    <p id="state"> </p>
    <canvas id="canvas" name="game"></canvas>
    <script>
      var canvas, ctx, width, height, radius, x_orig, y_orig, lastDataSent = {x:0,y:0,angle:90,speed:0}
      var connection = new WebSocket("ws://" + location.hostname + ":81/", ["arduino"])
      printState()

      connection.onopen = function () {
        printState()
        connection.send("Connect " + new Date())
      }
      connection.onclose = function () {
        printState()
      }
      connection.onerror = function (error) {
        printState()

        console.log("WebSocket Error ", error)
        alert("WebSocket Error ", error)
      }
      connection.onmessage = function (e) {
        console.log("Server: ", e.data)
      }

      function send (x, y, speed, angle) {
        var data = { speed, angle }
        if (connection.readyState === 1) {
          if (data.speed !== lastDataSent.speed || data.angle !== lastDataSent.angle) {
            lastDataSent = data
            connection.send(JSON.stringify(data))
          }
        } else {
          console.log(connection.readyState, data)
        }
      }

      function printState() {
        document.getElementById("state").innerText = ["Connecting...", "Connected", "Closing...", "Closed"][connection.readyState] || "Unknown connection state"
      }

      window.addEventListener("load", () => {
        canvas = document.getElementById("canvas")
        ctx = canvas.getContext("2d")          
        resize() 

        document.addEventListener("mousedown", startDrawing)
        document.addEventListener("mouseup", stopDrawing)
        document.addEventListener("mousemove", Draw)

        document.addEventListener("touchstart", startDrawing)
        document.addEventListener("touchend", stopDrawing)
        document.addEventListener("touchcancel", stopDrawing)
        document.addEventListener("touchmove", Draw)
        window.addEventListener("resize", resize)

        document.getElementById("speed").innerText = 0
        document.getElementById("angle").innerText = 90
      })

      function resize() {
        width = window.innerWidth
        radius = 210
        height = radius * 6.5
        ctx.canvas.width = width
        ctx.canvas.height = height
        background()
        joystick(width / 2, height / 3)
      }

      function background() {
        x_orig = width / 2
        y_orig = height / 3

        ctx.beginPath()
        ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true)
        ctx.fillStyle = "#ECE5E5"
        ctx.fill()
      }

      function joystick(width, height) {
        ctx.beginPath()
        ctx.arc(width, height, radius, 0, Math.PI * 2, true)
        ctx.fillStyle = "#F08080"
        ctx.fill()
        ctx.strokeStyle = "#F6ABAB"
        ctx.lineWidth = 8
        ctx.stroke()
      }

      let coord = { x: 0, y: 0 }
      let paint = false

      function getPosition (event) {
        var mouse_x = event.clientX || event.touches[0].clientX
        var mouse_y = event.clientY || event.touches[0].clientY
        coord.x = mouse_x - canvas.offsetLeft
        coord.y = mouse_y - canvas.offsetTop
      }

      function is_it_in_the_circle() {
        var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2))
        return (radius >= current_radius)
      }


      function startDrawing(event) {
        paint = true
        getPosition(event)
        if (is_it_in_the_circle()) {
          ctx.clearRect(0, 0, canvas.width, canvas.height)
          background()
          joystick(coord.x, coord.y)
          Draw(event)
        }
      }


      function stopDrawing() {
        paint = false
        ctx.clearRect(0, 0, canvas.width, canvas.height)
        background()
        joystick(width / 2, height / 3)
        document.getElementById("speed").innerText = 0
        document.getElementById("angle").innerText = 90
        send(0, 0, 0, 90)
      }

      function Draw(event) {
        if (paint) {
          ctx.clearRect(0, 0, canvas.width, canvas.height)
          background()
          var angle_in_degrees, x, y, speed
          var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig))

          angle_in_degrees = 180 - Math.abs(30 * Math.round((-angle * 180 / Math.PI) / 30))

          if (is_it_in_the_circle()) {
              joystick(coord.x, coord.y)
              x = coord.x
              y = coord.y
          } else {
              x = radius * Math.cos(angle) + x_orig
              y = radius * Math.sin(angle) + y_orig
              joystick(x, y)
          }

      
          getPosition(event)

          var x_relative = Math.round((x - x_orig) / (radius / 3))
          var y_relative = Math.round((y - y_orig) / (radius / 3))

          var divider = 100 / 3
          var speed =  (0 < y_relative?-1:1) * Math.ceil(divider * Math.round((100 * Math.sqrt(Math.pow(x - x_orig, 2) + Math.pow(y - y_orig, 2)) / radius) / divider))

          document.getElementById("speed").innerText = speed
          document.getElementById("angle").innerText = angle_in_degrees

          send(x_relative, y_relative, speed, angle_in_degrees)
        }
      }
    </script>
</body>
</html>
)";
