require 'libusb'

usb = LIBUSB::Context.new


usb.devices(:idVendor => 5824).each do |device|
  # puts "#{device.idVendor} #{device.idProduct}"
  # puts "Speed: #{device.device_speed}"
  # puts "Manufacturer: #{device.manufacturer}"
  # device.configurations.each do |configuration|
  #   puts " Configuration #{configuration.description}"
  #   configuration.endpoints.each do |endpoint|
  #     begin #if endpoint.direction == :in  
  #       puts "  Endpoint: "
  #       puts "   maxPacketSize #{endpoint.wMaxPacketSize}"
  #       puts "   Transfer Type #{endpoint.transfer_type}"
  #       puts "   Usage Type #{endpoint.usage_type}"
  #       puts "   Direction #{endpoint.direction}"
  #       puts "   Endpoint Number #{endpoint.endpoint_number}"
  #       puts "   Extra #{endpoint.extra}" 
  #     end
  #   end
  # end

  # device.interfaces.each do |interface|
  #   puts "Interface #{interface.bInterfaceNumber}"
  #   interface.endpoints.each do |endpoint|
  #     begin #if endpoint.direction == :in  
  #       puts "  Endpoint: "
  #       puts "   maxPacketSize #{endpoint.wMaxPacketSize}"
  #       puts "   Transfer Type #{endpoint.transfer_type}"
  #       puts "   Usage Type #{endpoint.usage_type}"
  #       puts "   Direction #{endpoint.direction}"
  #       puts "   Endpoint Number #{endpoint.endpoint_number}"
  #       puts "   Extra #{endpoint.extra}" 
  #     end
  #   end
  # end

  device.open_interface(0) do handle
    #    handle.control_transfer(:bmRequestType => 0x40, :bRequest => 0xa0, :wValue => 0xe600, :wIndex => 0x0000, :dataOut => 1.chr)
  end
end
