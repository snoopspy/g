{
    "connections": [
        {
            "receiver": "pcapDeviceWrite1",
            "sender": "pcapPipe1",
            "signal": "captured(GPacket*)",
            "slot": "write(GPacket*)"
        }
    ],
    "nodes": [
        {
            "_class": "GPcapPipe",
            "_x": -45,
            "_y": -41,
            "autoParse": true,
            "bufSize": "32768",
            "command": "adb exec-out su -c \"cd /data/data/com.snoopspy/files; export LD_LIBRARY_PATH=/data/data/com.snoopspy/files/../lib; ./corepcap dev wlan0 -f '' file -\"",
            "enabled": true,
            "objectName": "pcapPipe1",
            "readTimeout": "100",
            "removeCr": true
        },
        {
            "_class": "GPcapDeviceWrite",
            "_x": -64,
            "_y": 10,
            "intfName": "dum0",
            "objectName": "pcapDeviceWrite1"
        }
    ]
}
