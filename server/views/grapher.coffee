graphInit = ->
  dps = [] # dataPoints
  chart = new CanvasJS.Chart("chartContainer",
    data: [
      type: "line"
      dataPoints: dps
    ]
  )
  xVal = 0
  yVal = 100
  updateInterval = 20
  dataLength = 300 # number of dataPoints visible at any point

  datafeed = new WebSocket("ws://localhost:4567/datafeed")
  datafeed.onmessage = (e) ->
    server_message = e.data
    console.log server_message
    dps.push
      x: xVal
      y: parseInt(server_message)
    xVal++
    dps.shift()  if dps.length > dataLength
    chart.render()
    return
  datafeed.onopen = -> console.log 'Datafeed open!'
  datafeed.onclose = -> console.log 'Datafeed closed!'
  datafeed.onerror = (error) ->
    console.log "Error detected: " + error
    return  


  updateChart = (count) ->
    count = count or 1
    # count is number of times loop runs to generate random dataPoints.
    j = 0
    while j < count
      yVal = 1
      dps.push
        x: xVal
        y: yVal
      xVal++
      j++
    dps.shift()  if dps.length > dataLength
    chart.render()
    return


  # generates first set of dataPoints
  updateChart dataLength
  

  return

window.graphInit = graphInit