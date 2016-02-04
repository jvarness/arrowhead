Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL('https://jvarness.github.io/arrowhead/');  
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  
  console.log('Recieved from app: ' + JSON.stringify(configData));
  
  if(configData.backgroundColor){
    Pebble.sendAppMessage({
      "backgroundColor": parseInt(configData.backgroundColor, 16),
      "minuteColor": parseInt(configData.minuteColor, 16),
      "hourColor": parseInt(configData.hourColor, 16)
    }); 
  }
});