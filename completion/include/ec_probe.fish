function _ec_probe_complete_map_register
  grep '=' "$_EC_PROBE_MAP_FILE[-1]" 2>/dev/null | cut -d= -f1
end

function _ec_probe_complete_map_method
  grep -v '=' "$_EC_PROBE_MAP_FILE[-1]" 2>/dev/null
end
