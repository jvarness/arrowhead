(function() {
	loadOptions();
	submitHandler();
})();

function loadOptions() {
  var $background = $('#backgroundColor');
  var $hour = $('#hourHandColor');
  var $minute = $('#minuteHandColor');
  
  if (localStorage.options) {
    $background[0].value = localStorage.options.backgroundColor;
    $hour[0].value = localStorage.options.hourColor;
    $minute[0].value = localStorage.options.minuteColor;
  }
}

function submitHandler() {
	var $submitButton = $('#submit');
	
	$submitButton.on('click', handleSubmit);
}

function handleSubmit() {
	var return_to = getQueryParam('return_to', 'pebblejs://close#');
	document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfig()));
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}

function getAndStoreConfig() {
	var $background = $('#backgroundColor');
  var $hour = $('#hourHandColor');
  var $minute = $('#minuteHandColor');
  
  var options = {
    backgroundColor: $background.val(),
    minuteColor: $minute.val(),
    hourColor: $hour.val()
  };
  
  localStorage.options = options;
  
  return options;
}