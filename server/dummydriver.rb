require 'socket'                # Get sockets from stdlib
require 'json'

count = 0
server = TCPServer.open(3430)   # Socket to listen on port 
loop {                          # Servers run forever
  Thread.start(server.accept) do |client|
    while true do
      # Base simulator
      base = 30*(rand(4)+1)

      data = {"data" => 20.times.map{ base + Random.rand(25) } }
	    client.puts  JSON.generate data# random data determined by dice roll.
      sleep 0.2
      puts count
      count = count + 1
    end
  end
}