function ping()
{
  var hostname = $("#host").val();
  var index;
  const pattern = /^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$/;
  if (!pattern.test(hostname)) {
    return false;
  }
  $("#ping_result").val("Testing...");
  $.post("/ping", "host=" + hostname, function(data) {
      $("#ping_result").val(data);
    }
  );
  return true;
}

function refresh_status()
{
  $.get("/status", function(data) {
    var parsed = JSON.parse(data);
    $("#overview").html(parsed["status"]);
    if (parsed["status"] == "ok") {
      $("#lan_ip").html(parsed["lan_ip"]);
      $("#cpu_usage").html(parsed["cpu_usage"]);
      $("#bandwidth").html(parsed["bandwidth"]);
    }
  });
  setTimeout(refresh_status, 30000);
}

$(document).ready(function() {
  refresh_status();
});
