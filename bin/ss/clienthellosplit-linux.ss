{
    "connections": [
        {
            "receiver": "clientHelloSplit1",
            "sender": "tcpFlowMgr1",
            "signal": "managed(GPacket*)",
            "slot": "split(GPacket*)"
        },
        {
            "receiver": "tcpFlowMgr1",
            "sender": "netFilter1",
            "signal": "captured(GPacket*)",
            "slot": "manage(GPacket*)"
        },
        {
            "receiver": "rawIpSocketWrite1",
            "sender": "clientHelloSplit1",
            "signal": "writeNeeded(GPacket*)",
            "slot": "write(GPacket*)"
        }
    ],
    "nodes": [
        {
            "_class": "GTcpFlowMgr",
            "_x": -47,
            "_y": -156,
            "checkInterval": "1",
            "finTimeout": "20",
            "fullTimeout": "180",
            "halfTimeout": "60",
            "objectName": "tcpFlowMgr1",
            "rstTimeout": "10"
        },
        {
            "_class": "GClientHelloSplit",
            "_x": -58,
            "_y": -105,
            "bufSize": "32768",
            "objectName": "clientHelloSplit1",
            "tcpFlowMgr": "tcpFlowMgr1"
        },
        {
            "_class": "GRawIpSocketWrite",
            "_x": -67,
            "_y": -53,
            "objectName": "rawIpSocketWrite1"
        },
        {
            "_class": "GNetFilter",
            "_x": -36,
            "_y": -211,
            "acceptVerdict": "ACCEPT",
            "autoParse": true,
            "bufSize": "32768",
            "command": {
                "closeCommands": [
                    {
                        "commandType": "Execute",
                        "commands": [
                            "su -c \"iptables -D INPUT -j NFQUEUE\"",
                            "su -c \"iptables -D OUTPUT -j NFQUEUE\"",
                            "su -c \"iptables -D INPUT -d 127.0.0.1 -j ACCEPT\"",
                            "su -c \"iptables -D OUTPUT -d 127.0.0.1 -j ACCEPT\""
                        ],
                        "objectName": ""
                    }
                ],
                "objectName": "",
                "openCommands": [
                    {
                        "commandType": "Execute",
                        "commands": [
                            "su -c \"iptables -F\"",
                            "su -c \"iptables -A OUTPUT -d 127.0.0.1 -j ACCEPT\"",
                            "su -c \"iptables -A INPUT -d 127.0.0.1 -j ACCEPT\"",
                            "su -c \"iptables -A OUTPUT -j NFQUEUE\"",
                            "su -c \"iptables -A INPUT -j NFQUEUE\""
                        ],
                        "objectName": ""
                    }
                ]
            },
            "enabled": true,
            "mark": "0",
            "nonBlock": false,
            "objectName": "netFilter1",
            "queueNum": "0",
            "waitTimeout": "1"
        }
    ]
}
