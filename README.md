# ZMTP for Particle

This is a basic, _partial_ implementation of [ZMTP][zmtp] for Particle devices. The current goal of this library is to support the [ZRE][zre] library. Unless you want the lower-level control offered by ZMTP, using ZRE is recommended.

Current support:

- All sockets
  - Sending and receiving messages
- DEALER sockets
  - Associating a custom identity
- ROUTER sockets
  - Routing using identity frames, when specified

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

The ZMTP library (and it's bigger sibling, ZeroMQ) are centered around two types: sockets (`ZMTPSocket`) and frames (`ZMTPFrame`). To start, create a new socket, either of type ZMTPDealer (for a DEALER socket) or of type ZMTPRouter (for a ROUTER socket):

```
// ZMTPRouter takes the port to listen on.
ZMTPSocket *router = new ZMTPRouter(1235);

// ZMTPDealer takes the address and port to connect to.
ZMTPSocket *dealer = new ZMTPDealer(IPAddress(192, 168, 29, 123), 1234);
```

Messages in ZMTP are built of frames: zero or more frames with a "MORE" flag, followed by a _single_ frame without that flag:

```
ZMTPFrame *firstFrame = new ZMTPFrame("Hello", ZMTP_FRAME_MORE);

ZMTPFrame *secondFrame = new ZMTPFrame("World");
```

To send those frames:

```
dealer.send(firstFrame);
dealer.send(secondFrame);
```

Finally, once you're finished with the socket, it should be destroyed:

```
delete dealer;
delete router;
```

## Further reading

To get the most out of this library, you should understand how DEALERs and ROUTERs interact, and what they expect. That's best left to the original descriptions and specifications:

- [ZMTP][zmtp]
- [Dealers & Routers][reqrep]

## Contributing

Pull requests are welcome and encouraged. Please adhere to the [Contributor Covenant][covenant] in your interactions.

[zmtp]: https://rfc.zeromq.org/spec:37/ZMTP
[zre]: https://rfc.zeromq.org/spec:36/ZRE/
[reqrep]: https://rfc.zeromq.org/spec:28/REQREP/
[covenant]: https://www.contributor-covenant.org/version/1/4/code-of-conduct.html
