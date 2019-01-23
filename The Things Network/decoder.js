// check if a bit flag in a byte is set
function isBitSet(byte, bitOffset) {
    if (byte & (0x01 << (bitOffset - 1)))
      return true;
    else
      return false;
  }
  
  // convert a byte array to a float value
  function processFloat(bytes, offset) {
    if (bytes[offset] & 0x80)
      retVal = ((0xffff << 16) + (bytes[offset] << 8) + bytes[offset+1]) / 100;
    else
      retVal = ((bytes[offset] << 8) + bytes[offset+1]) / 100;
    return retVal;
  }
  
  // main decode function
  function Decoder(bytes, port) {
    var decoded = {};
    var events = {
      1: 'setup',
      2: 'interval',
      3: 'motion',
      4: 'button'
    };
    decoded.event = events[port];
    decoded.battery = (bytes[0] << 8) + bytes[1];
    decoded.light = (bytes[2] << 8) + bytes[3];
    decoded.temperature = processFloat(bytes, 4);
    decoded.accelerationX = processFloat(bytes, 6);
    decoded.accelerationY = processFloat(bytes, 8);
    decoded.accelerationZ = processFloat(bytes, 10);
    
    decoded.isMoving = isBitSet(bytes[12], 1)?'moving':'stopped';
    
    decoded.tempAlert = isBitSet(bytes[12], 2)?'alert':'normal';
    if (isBitSet(bytes[12], 3)) {
      decoded.tempAlert = 'critical';
    }
    
    return decoded;
  }