# AirPlay receiver tests

Results of playing to real receivers, beyond the fake receiver in
`tests/fakeairplayreceiver.cpp`.

## Mac (AirTunes 980.77.5)

A Mac14,2 with AirPlay Receiver set to allow anyone on the network, tested
2026-10-30 and 2026-11-01. It advertises both `_airplay._tcp` and
`_raop._tcp`, under the same MAC.

### Discovery and pairing

| Check | Result |
| --- | --- |
| Discovery | Found over `_airplay._tcp`. |
| Status flags | 0x84 (or 0x4); 0x284 with "Require password" on, so 0x200 means a password. |
| Transient pairing | Works with PIN 3939, without /pair-pin-start. |
| /pair-pin-start | Asks to Allow, then shows a full-screen PIN: 3939 on 2026-10-30, a random one on 2026-11-01. |
| Allow prompt | Holds the first SETUP until answered; unanswered, the Mac answers 400 after about 15 s. |
| Asking again | It remembers allowing a sender for a while, but asks again at times for reasons unknown, even a minute later or after a clean TEARDOWN. |
