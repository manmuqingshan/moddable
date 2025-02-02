import "system"		// system initializes globalThis.device. this ensures it runs before this module.

import TCP from "embedded:io/socket/tcp";
import UDP from "embedded:io/socket/udp";
import Resolver from "embedded:network/dns/resolver/udp";
import TLSSocket from "embedded:io/socket/tcp/tls";

import HTTPClient from "embedded:network/http/client";

const dns = {
	io: Resolver,
	servers: [
		"1.1.1.1", //...
	],
	socket: {
		io: UDP
	}
};
globalThis.device = Object.freeze({
	...globalThis.device,
	network: {
		...globalThis.device?.network,
		http: {
			io: HTTPClient,
			dns,
			socket: {
				io: TCP
			}
		},
		https: {
			io: HTTPClient,
			dns,
			port: 443,
			socket: {
				io: TLSSocket,
				TCP: device.network.http.socket,
				secure: {
					applicationLayerProtocolNegotiation: "http/1.1"
				}
			}
		}
	},
}, true);

