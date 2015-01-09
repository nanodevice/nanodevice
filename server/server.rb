require 'rubygems' 
require 'sinatra'
require 'sass'
require 'haml' 
require 'coffee-script'
require 'sinatra-websocket'
require 'open3'
require 'JSON'
require 'pty'

set :server, 'thin'
set :sockets, []

@i=0

def send_tick
  for x in 1..100 do 
    settings.sockets.each do |s| 
      s.send(@i.to_s)
    end 
    @i = @i + (rand(6)-2.5).ceil
  end
end

# Open the driver

Thread.new do
  puts "Popen!"
  PTY.spawn("../driver/driver") do |stdin, stdout, pid|
    puts "Popened!"
    # Acquire data at speed 2
    stdout.puts "R;"
    delay(1)
    stdout.puts "sa100;"
    delay(1)
    stdout.puts "sb600;"
    delay(1)
    stdout.puts "d1;" # See firmware for codes
    delay(1)
    puts "Started DAQ"
    while 1 do 
      begin
        sumCount = 0
        sum = 0
        min = 10000
        max = 0
        sumMax = 100
        stdin.each do |line|
          #puts line
          json = JSON.parse(line)
          #puts "Have #{settings.sockets.count} sockets!"
          settings.sockets.each do |s|
            json["data"].each do |val|
              sum = sum + val
              sumCount = sumCount + 1
              if val > max 
                max = val
              end
              if val < min  
                min = val
              end
              if sumCount >= sumMax
                s.send((sum/sumMax).to_s)
                #s.send(min.to_s)
                #s.send(max.to_s)
                sumCount = 0
                min = 10000
                max = 0
                sum = 0
              end
            end
            #  puts "sending to socket"
            #s.send(json["data"][0].to_s)
          end
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
  if !request.websocket?
    haml :index
  else
    request.websocket do |ws|
      ws.onopen do
        ws.send("-2")
        settings.sockets << ws
      end
      ws.onmessage do |msg|
        
      end
      ws.onclose do
        warn("websocket closed")
        settings.sockets.delete(ws)
      end
    end
  end
end

