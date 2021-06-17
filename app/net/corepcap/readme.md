corepcap
===

## syntax
```
corepcap <input> <output>[<output>...]
```
  * Input must be specified just once, meanwhile, output could be multiple instance.

```
input ::= <schema> <name> [input-options]
```

```
output ::= <schema> <name>
```

```
schema ::= dev|file
```

```
name ::= e.g., eth0, wlan0, test.pcap, -, etc
```
  * "-" is stdin(input) or stdout(output).

```
input-options ::=  | -f <filter> | -l <snap len> | -p <promisc> | -rt <read timeout> | -wt <wait timeout> [input-options]
```
    * filter : default("")
    * snap len : default(32768 bytes)
    * promisc : default(1 PCAP_OPENFLAG_PROMISCUOUS)
    * read timeout : default(-1 msec)
    * wait timeout : default(1 msec)

## example
```
corepcap dev wlan0 file -
corepcap dev wlan0 -filter "tcp" file - file tcp.pcap
corepcap dev mon0 -filter "wlan[0] != 0x80" file -
corepcap file - dev dum0
```

## tcpdump to stdout
```
tcpdump -i wlan0 -w - -U
```

## android tcpdump to stdout
```
adb exec-out "su -c 'tcpdump -i wlan0 -U -w - 2>/dev/null'"
```

## android corepcap to stdout
```
adb exec-out "su -c 'corepcap dev wlan0 file - 2>/dev/null'"
```

## wireshark from stdin
```
| wireshark -k -S -i -
```

## corepcap from stdin
```
| corepcap file - dev wlan0
```

## android corepcap nexmon to wlan1mon
```
modprobe mac80211_hwsim
gmon wlan1 wlan1mon
adb exec-out "su -c 'export export LD_PRELOAD=libfakeioctl.so; nexutil -m2; corepcap dev wlan0 file - 2>/dev/null'"
```
