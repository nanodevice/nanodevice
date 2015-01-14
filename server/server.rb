require 'rubygems' 
require 'sinatra'
require 'sass'
require 'haml' 
require 'coffee-script'
require 'sinatra-websocket'
require 'open3'
require 'JSON'

set :server, 'thin'

# Array of websockets, one for each open page
# to stream the data to.
set :sockets, []

# Open the driver

Thread.new do
  puts "Popen!"
  PTY.spawn("../driver/driver") do |stdin, stdout, pid|
    puts "Popened!"
    # Reset the hardware
    stdout.puts "R;"
    delay(1)
    # Set voltage A to 1000000uV (1V)
    stdout.puts "sa1000000;"
    delay(1)
    # Set voltage B to 1010000uV (1010mV)
    stdout.puts "sb1001000;"
    delay(1)
    # Set data acquisition rate to 500Hz
    # See firmware for real acquisition codes;
    # d3; -> 10Hz, d5; -> 30Hz, d9; -> 500Hz, da; -> 1kHz, de; -> 15kHz.
    stdout.puts "d9;"  
    delay(1)
    puts "Started DAQ"
    while 1 do 
      begin
        stdin.each do |line|
          # Line has the format of {"data": [1,2,3,4]}
          json = JSON.parse(line)

          # To each socket send each value
          settings.sockets.each do |s|
            json["data"].each do |val|
              s.send(val.to_s)
            end
          end
          #  puts "sending to socket"
          #s.send(json["data"][0].to_s)
        end
        
      rescue Exception => e
       # puts "Error parsing: #{e}"
      end
    end    
  end
  puts "Popend!"
end

get '/' do 
  haml :index
end 

get '/reset.css' do
  sass :reset
end

get '/main.css' do
  sass :main
end

get '/grapher.js' do
  coffee :grapher
end

get '/masonry.js' do
  coffee :masonry
end

get '/datafeed' do
  # If it's not a websocket connection, just show the index page.
  if !request.websocket?
    haml :index
  else
    # Open the websocket, and send a number. 
    request.websocket do |ws|
      ws.onopen do
        ws.send("-2")
        settings.sockets << ws
      end

      # There should be nothing done with incoming data.
      ws.onmessage do |msg|
        
      end

      ws.onclose do
        warn("websocket closed")
        settings.sockets.delete(ws)
      end
    end
  end
end

