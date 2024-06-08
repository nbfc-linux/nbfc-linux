The protocol of NBFC
====================

My protocol involves exchanging JSON objects between a client and a server. In both directions, the protocol format is `<JSON>\nEND`, meaning that the data is read continuously until a newline followed by the string "END" (\nEND) is encountered. Each message begins with a JSON object and ends with the delimiter \nEND, which signifies the end of the transmission for that particular message. This structure ensures that both the client and the server can correctly parse and process the JSON data encapsulated within the message boundaries.

Example usage using socat:

```
 ~ > socat - UNIX-CONNECT:/var/run/nbfc_service.socket
{"command": "status"}
END
```

See [tools/test-protocol.py](/tools/test-protocol.py) if you want an example of
an python client for NBFC.

Commands
========

**status**

Get a JSON that describes the server status

`{"command": "status"}`

**set-fan**

Set the speed for all fans:

`{"command": "set-fan", "speed": <SPEED>}`

Set the speed for a specific fan:

`{"command": "set-fan", "fan": <NUMBER>, "speed": <SPEED>}`

Set the fan mode to "auto" for a specific fan:

`{"command": "set-fan", "fan": <NUMBER>, "speed": "auto"}`
