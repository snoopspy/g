{
    "connections": [
        {
            "receiver": "tcpFlowMgr1",
            "sender": "netFilter1",
            "signal": "captured(GPacket*)",
            "slot": "process(GPacket*)"
        },
        {
            "receiver": "clientHelloSplit1",
            "sender": "tcpFlowMgr1",
            "signal": "processed(GPacket*)",
            "slot": "split(GPacket*)"
        }
    ],
    "nodes": [
        {
            "_class": "GNetFilter",
            "_x": -58,
            "_y": -72,
            "acceptVerdict": "ACCEPT",
            "autoParse": true,
            "command": {
                "closeCommands": [
                    {
                        "commandType": "Execute",
                        "commands": [
                            "su -c \"iptables -D OUTPUT -d 127.0.0.1 -j ACCEPT\"",
                            "su -c \"iptables -D INPUT -d 127.0.0.1 -j ACCEPT\"",
                            "su -c \"iptables -D OUTPUT -j NFQUEUE\"",
                            "su -c \"iptables -D INPUT -j NFQUEUE\""
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
            "objectName": "netFilter1",
            "queueNum": "0"
        },
        {
            "_class": "GTcpFlowMgr",
            "_x": -69,
            "_y": -21,
            "checkInterval": "1",
            "finTimeout": "20",
            "fullTimeout": "180",
            "halfTimeout": "60",
            "objectName": "tcpFlowMgr1",
            "rstTimeout": "10"
        },
        {
            "_class": "GClientHelloSplit",
            "_x": -79,
            "_y": 28,
            "objectName": "clientHelloSplit1",
            "tcpFlowMgr": "tcpFlowMgr1",
            "write": "rawIpSocketWrite1"
        },
        {
            "_class": "GRawIpSocketWrite",
            "_x": -87,
            "_y": 75,
            "objectName": "rawIpSocketWrite1"
        }
    ]
}
