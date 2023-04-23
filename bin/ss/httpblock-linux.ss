{
    "connections": [
        {
            "receiver": "tcpBlock1",
            "sender": "pcapDevice1",
            "signal": "captured(GPacket*)",
            "slot": "block(GPacket*)"
        }
    ],
    "nodes": [
        {
            "_class": "GPcapDevice",
            "_x": -54,
            "_y": -63,
            "autoParse": true,
            "enabled": true,
            "filter": "port 80 and tcp[((tcp[12:1] & 0xf0) >> 2):4] = 0x47455420",
            "flags": "1",
            "mtu": "0",
            "objectName": "pcapDevice1",
            "readTimeout": "-1",
            "snapLen": "32768",
            "waitTimeout": "1"
        },
        {
            "_class": "GTcpBlock",
            "_x": -43,
            "_y": -10,
            "backwardBlockType": "Fin",
            "backwardFinMsg": [
                "HTTP/1.1 200 Ok",
                "Content-Length: 22",
                "",
                "You have been blocked."
            ],
            "bufSize": "32768",
            "enabled": true,
            "forwardBlockType": "Rst",
            "forwardFinMsg": [
            ],
            "objectName": "tcpBlock1",
            "writer": "rawIpSocketWrite1"
        },
        {
            "_class": "GRawIpSocketWrite",
            "_x": -76,
            "_y": 36,
            "intfName": "",
            "mtu": "1500",
            "objectName": "rawIpSocketWrite1"
        }
    ]
}
