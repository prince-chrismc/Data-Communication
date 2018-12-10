## Theory Assignment 1

1. Circuit Switching aims at providing a better service through the reservation of the circuit
(i.e. circuit is dedicated). Now, considering only the perspective of the communicating
users over a Circuit Switching network (i.e. you should not be concerned with the entire
utilization of the network or the advantages to other users), is it possible that Circuit
Switching may actually end up harming its users instead of providing a better service to
them. If yes, provide a scenario/case that shows that. If no, explain why this service will
indeed provide the best service to its users at all times.

```
The link between nodes is very costly to setup. This means in the case of a node going down
or a route no longer being viable the cost to re-establish a new path is very costly. Similarly
after a path is established it will be used indefinately even if a new better route becomes
avaiable. Cuircuit switch users are at a disadvantage when the amounth of communicationis very
limited, setup time would be longer than the actual communication. Circuit switch users could not
benifit from newer better routes being established.
```

2. With DSL ISPs, dedicated lines usually connect to the local offices to the location where
the service is provided (i.e. residential homes). In contrast, with Cable ISPs, the
connection is shared between multiple homes in a neighborhood. In spite of this
configuration that is clearly to the benefit of DSL users, service provided by Cable ISPs
may still be superior to the one provided by DSL providers. Explain the reasons behind
this. Further, if you are hired by a DSL provider, and taking into account that changes to
the company’s infrastructure (i.e. wiring) is quite costly, what would you propose in
order to speed up the provided service while balancing the cost.

```
DSL users suffer from the poor quality of the "last mile", the qualitity of the wire is too low
to carry higher frequencies. Cable users already share high-bandwith cable which is multiplexed
between channels. This allows Cable ISP to provide higher speeds than DSL ISPs.

In order to offer higher speends to costumers without drastically changing infrastucture, it is
possible to remove some of the analog to digital conversions at the regional stations which can
have a direct digital connection to the internet.

Offering client the choice of half versus full duplex is also a possibility. Most systems do not
have intense bi-directional communication, as such one-way communication ( at a time ) is more than
sufficent for most costumers.
```

3. HTTP provides two connections methods: persistent and non-persistent. While there are
clear advantages of persistent-HTTP, it is rather unclear whether non-persistent can be of
any use. Conduct a small research to find out why these two modes of connections are
provided. In specific, you should find out some of the advantages and disadvantages of
each of them, hence justifying their concurrent existence.

```
Persistent connections allow for multiple requests to be made over the same connection. If may
requests are made to the same server the slow TCP startup could cause a performance bottleneck.
However if accessing a resource is a one time affair, there is no need to have a persistent
connection in such a case non-persistent access is more then sufficent.

It is always important to note that new technologies take time to be adopted, for instance HTTP/2
which depiste having advantages for browsing the web internal or private implementations often do
not support this new feature making backwards compatability a nessecity.
```

4. A successful attack to the Internet DNS would be devastating. Explain what type of
attacks can be made towards DNS. Why, to-date, such attacks in practice have not been
successful? In your answer, you should consider caching in particular. Why such
technique has not only proven to provide better performance, which is its original goal,
but also protection against security attacks.

```
DNS is most suseptable to bandwidth flooding DoS attacks. this is due to the fact that it works
over UDP with a small number of root servers. Flooding a root server with requests could overwhelm
the system and distrupt web browsing. Attacks of this style have yet to be successful because DNS
requests rarely make it to the root servers, this is because many DNS server cache results to avoid
duplicate lookups. Regional ISP tend to provide their own chached DNS servers to help midigate
traffic that would otherwise hit root servers.
```

5. Perform a Traceroute between your machine and any other host/server on the Internet,
preferably an overseas server. Provide snapshots of what was returned and analyze the
returned information. In specific, you should comment on any behavior that looks either
different or unusual. Indicate the number of routers between your machine and the
targeted host/server.

```
Tracing route to www.bbc.net.uk [212.58.249.213]
over a maximum of 30 hops:

  1     1 ms     2 ms    <1 ms  EMG2926 [192.168.0.1]
  2    19 ms    42 ms    24 ms  modemcable001.134-62-166.mc.videotron.ca [166.62.134.1]
  3    18 ms    16 ms    16 ms  10.170.166.229 # A Private address =?
  4    13 ms    15 ms    15 ms  216.113.124.61
  5    15 ms    14 ms    11 ms  216.113.122.210 # Last IP owned by videotron!
  6    16 ms    14 ms    16 ms  motl-b1-link.telia.net [62.115.148.108] # Welcome to Sweeden! With little latency !
  7    89 ms    90 ms    89 ms  nyk-bb4-link.telia.net [62.115.134.52] # Latency spike ! IXP point?
  8    90 ms    89 ms    91 ms  ldn-bb4-link.telia.net [62.115.136.184]
  9    91 ms    93 ms    94 ms  ldn-b5-link.telia.net [213.155.132.197]
 10    90 ms    92 ms    95 ms  atos-ic-315186-ldn-b5.c.telia.net [62.115.144.161] # Last stop in sweeden
 11     *        *        *     Request timed out. # No data crossing EU =(
 12     *        *        *     Request timed out. # No data crossing EU =(
 13    95 ms    91 ms    89 ms  ae1.er02.lbh.bbc.co.uk [132.185.254.142] # Finally we arrive in London =D
 14   115 ms   111 ms   123 ms  132.185.252.130 # High latency, looks like a reverse proxy
 15    92 ms    88 ms    92 ms  bbc-vip151.lbh.bbc.co.uk [212.58.249.213] # Finally server by a webserver!

Trace complete.
```

6. Suppose two hosts, A and B, are 12,000km apart and are connected by a direct link of
rate R = 5 Mbps. Assume further that the propagation speed over the link is 2.5 x 10<sup>8</sup>
, and that the packets to be transferred are of size 4 Mbits.

   a. What is the propagation delay to send one packet from A to B?
   
   b. What is the transmission delay to send 8 packets from A to B?

```
Math is hard. Missing Units on 2.5x10^8 makes it a crab shoot... m/s? nano-m/s =? km/s mm/min ???
```

7. Now assume some modifications to the network specified in Question 6 above, where 2
routers are installed between A and B at more or less equal distances. Further assume the
link from A to the first router has been replaced by another link of rate R=4 Mbps, the
link between the two routers was kept as before, and the link between the second router
and B was replaced by a link of rate R=6 Mbps. Assume negligible processing and
queuing delays in the routers.

a. What is the propagation delay to send one packet from A to B?

b. What is the total transmission delay to send 2 packets from A to B?

c. How much time is needed to send one file of size 20 Mbits from A to B?

```
Math is hard. Missing Units on 2.5x10^8 makes it a crab shoot... m/s? nano-m/s =? km/s mm/min ???
```

8. Consider distributing a file _F_ of size 15Gb/15360Mb to _N_ peers. The server has an upload
rate of u<sub>s</sub>= 30 Mbps, and each peer has a download rate of d<sub>i</sub> = 2 Mbps and an
upload rate of _u_. For N = 10, 100, and 1000 and u = 300 Kbps, 700 Kbps, and 2 Mbps, fill the
following table by the minimum distribution time for each of the combinations of N and u for both
client-server distribution and P2P distribution. 

Client-Server

bps     | 10 | 100 | 100
--------|----|-----|-----
300Kbps | 5120s |  51200s|  512000s
700Kbps | 5120s |  51200s|  512000s
2Mbps   | 5120s |  51200s|  512000s

Peer-to-peer

bps     | 10 | 100 | 100
--------|----|-----|-----
300Kbps |
700Kbps |
2Mbps   |

9. BitTorrent uses a trading scheme referred to as tit-for-tat. While some researchers argued
particular deficiencies in that scheme, others argued that if it was not for tit-for-tat, it is
likely that BitTorrent would not even exist today. Explain both points of view. In other
words, you should indicate the disadvantages of the scheme, as well as why it is still
crucial to have such a scheme, or an alternative one, for BitTorrent to exist. 
