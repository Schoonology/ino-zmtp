# ZMTP for Particle

This is a basic, _partial_ implementation of [ZMTP][zmtp] for Particle devices. The current goal of this library is to support the [ZRE][zre] library. Unless you want the lower-level control offered by ZMTP, using ZRE is recommended.

Current support:

- DEALER sockets
  - Associating a custom identity
  - Sending messages

Desired support:

- Receiving messages for all socket types
- ROUTER sockets

## Including in your project

To add the ZMTP library to your Particle project:

```
particle library add ZMTP
```

To include the ZMTP types and functions in your code, add the following toward
the top of your project:

```
#include "ZMTP.h"
```

## Usage

The ZMTP library (and it's bigger sibling, ZeroMQ) are centered around two types: sockets (`zmtp_socket_t`) and frames (`zmtp_frame_t`). To start, create a new socket, providing the type of socket you want to create:

```
zmtp_socket_t *socket = zmtp_socket_new (DEALER);
```

Messages in ZMTP are built of frames: zero or more frames with a "MORE" flag, followed by a _single_ frame without that flag:

```
uint8_t first_data[5] = { 'h', 'e', 'l', 'l', 'o' };
zmtp_frame_t *first_frame = zmtp_frame_new (first_data, 5, ZMTP_FRAME_MORE);

uint8_t second_data[5] = { 'w', 'o', 'r', 'l', 'd' };
zmtp_frame_t *second_frame = zmtp_frame_new (second_data, 5, ZMTP_FRAME_MORE);
```

To send those frames:

```
zmtp_socket_send (socket, first_frame);
zmtp_socket_send (socket, second_frame);
```

Finally, once you're finished with the socket, it should be destroyed:

```
zmtp_socket_destroy (&socket);
```

## Contributing

Pull requests are welcome and encouraged. Please adhere to the [Contributor Covenant][covenant] in your interactions.

[zmtp]: https://rfc.zeromq.org/spec:37/ZMTP
[zre]: https://rfc.zeromq.org/spec:36/ZRE/
[covenant]: https://www.contributor-covenant.org/version/1/4/code-of-conduct.html
