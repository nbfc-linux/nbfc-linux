_ec_probe_complete_map_register() {
  [[ ${#_EC_PROBE_MAP_FILE[@]} -gt 0 ]] && \
    grep '=' "${_EC_PROBE_MAP_FILE[-1]}" 2>/dev/null | cut -d= -f1
}

_ec_probe_complete_map_method() {
  [[ ${#_EC_PROBE_MAP_FILE[@]} -gt 0 ]] && \
    grep -v '=' "${_EC_PROBE_MAP_FILE[-1]}" 2>/dev/null
}
