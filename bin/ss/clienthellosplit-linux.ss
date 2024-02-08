{
    "connections": [
        {
            "receiver": "tcpFlowMgr1",
            "sender": "netFilter1",
            "signal": "captured(GPacket*)",
            "slot": "manage(GPacket*)"
        },
        {
            "receiver": "clientHelloSplit1",
            "sender": "tcpFlowMgr1",
            "signal": "managed(GPacket*)",
            "slot": "split(GPacket*)"
        }
    ],
    "nodes": [
        {
            "_class": "GNetFilter",
            "_x": -46,
            "_y": -55,
            "objectName": "netFilter1"
        },
        {
            "_class": "GTcpFlowMgr",
            "_x": -58,
            "_y": -3,
            "objectName": "tcpFlowMgr1"
        },
        {
            "_class": "GClientHelloSplit",
            "_x": -68,
            "_y": 50,
             "objectName": "clientHelloSplit1",
            "tcpFlowMgr": "tcpFlowMgr1"
         }
    ]
}
