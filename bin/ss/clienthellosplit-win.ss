{
    "connections": [
        {
            "receiver": "tcpFlowMgr1",
            "sender": "winDivert1",
            "signal": "captured(GPacket*)",
            "slot": "process(GPacket*)"
        },
        {
            "receiver": "clientHelloSplit1",
            "sender": "tcpFlowMgr1",
            "signal": "processed(GPacket*)",
            "slot": "split(GPacket*)"
        },
        {
            "receiver": "convertEthAutoMac1",
            "sender": "clientHelloSplit1",
            "signal": "writeNeeded(GPacket*)",
            "slot": "convert(GPacket*)"
        },
        {
            "receiver": "pcapDeviceWrite1",
            "sender": "convertEthAutoMac1",
            "signal": "converted(GPacket*)",
            "slot": "write(GPacket*)"
        }
    ],
    "nodes": [
        {
            "_class": "GWinDivert",
            "_x": 48,
            "_y": -129,
            "autoParse": true,
            "bufSize": "32768",
            "correctIpChecksum": true,
            "enabled": true,
            "filter": "true",
            "flags": "0",
            "layer": "0",
            "objectName": "winDivert1",
            "priority": "0",
            "queueLen": "8192",
            "queueTime": "1024",
            "tos": "0"
        },
        {
            "_class": "GTcpFlowMgr",
            "_x": 41,
            "_y": -85,
            "checkInterval": "1",
            "finTimeout": "20",
            "fullTimeout": "180",
            "halfTimeout": "60",
            "objectName": "tcpFlowMgr1",
            "rstTimeout": "10"
        },
        {
            "_class": "GClientHelloSplit",
            "_x": 33,
            "_y": -39,
            "bufSize": "32768",
            "objectName": "clientHelloSplit1",
            "tcpFlowMgr": "tcpFlowMgr1"
        },
        {
            "_class": "GConvertEthAutoMac",
            "_x": 20,
            "_y": 6,
            "bufSize": "32768",
            "mtu": "0",
            "objectName": "convertEthAutoMac1",
            "type": "2048"
        },
        {
            "_class": "GPcapDeviceWrite",
            "_x": 27,
            "_y": 50,
            "mtu": "0",
            "objectName": "pcapDeviceWrite1"
        }
    ]
}
