## Theory Assignment Three

1) What is the main difference between Pure Aloha and Slotted Aloha? Are there any
circumstances where Pure Aloha would perform better than Slotted Aloha? If so, give
such circumstances/conditions. If no, explain why Pure Aloha could never perform better
that Slotted Aloha.

```
The main disadvantage of pure Aloha is the high likelyhood of collision. In an enviroment
were transmission is very infrequent and very small segement size with little no no hosts.
Then its very unlikely there would be collisions, in such waiting for the next transmission
timeframe which has large gaps it would be more costly then send whenever.
```

2) Given a network with 9 routers as shown in the figure below. Assume the utilization of
center-based spanning tree construction, where router D is assigned as the center (or
root) router. Assume routers join the tree in the following order: C, A, H, B, F, G, E, and
I. Show the final constructed spanning tree for that given network.

```
Insert same graph with fewer lines =)
```

3) Consider two substitution ciphers. One adds a value of i to the ASCII code of the
plaintext character. The other adds a value of j to the plaintext character. All additions
are modulo 256. Now consider a double-encryption method that adds i to each plaintext
character and then adds j to the resulting ciphertext character to get another ciphertext
character. Again, all calculations are modulo 256 How much more secure is this double
encryption when compared with either single-encryption method? Explain your answer.

```
No safer, the ciphers shift all plain text by a constant amount, well so does the double-
encryption it just happens to apply it in two steps instead of one.
```

4) Consider the bit string 001011010101000011111101001101 and the key 10110. Use the
key to encrypt and then decrypt the string using bit level ciphering.

```
001011010101000011111101001101 # Plain bits
101101011010110101101011010110 # Key
------------------------------
100110001111110100010110011010 # Ciphered
101101011010110101101011010110 # Key
------------------------------
001011010101000011111101001101 # Plain bits reassembled
```

5) Suppose you were trying to crack an encryption method that used a 64-bit key. Assuming
a brute force attack, how many keys per second must you try to crack the code in 30
days?

```
2^64 keys / 30 * 24 * 60 * 60 * 60 = A LOT keys/s lol
```

6) Three broad classes to multiple access techniques exist, which are: channel partitioning,
random access and taking turns. In general, it is assumed that any network would use one
of these techniques or the other. However, cable Internet access networks utilize all three
techniques. Describe how this is done by such networks, and explain how the utilization
of any of these techniques does not conflict with the utilization of the other two.

```
Mutiplexing between internet usage and TV channels
Ethernet is random access to the wire
taking turns ???
```

7) In general, fully-connected topology is exhaustive and very unlikely to be used for the
construction of a network.
   i) Are there any clear and significant advantages of such topology? Explain clearly.

```
Work beautifully in garaunteeing maximum bandwidth between hosts, the server rack was a\
beautiful examples. [ insert word for word verbadom ]
```
   
   ii) Explain why it is unlikely that such topology be used for network construction.
You should clearly indicate the major disadvantages of such topology.

```
Adding new switches to the network would require N + 1 more connection, this does not
scale at all for more dynmic and growing networks.
```

   iii) While this topology seems to make little sense for the construction of networks, in
the general terms, it is actually used as part of Data Centers to connect Tier-1 and
Tier-2 switches. Explain clearly the major advantages of such utilization in data
centers. You should provide some example that shows, through some numerical
values, the obvious advantages of such utilization.

```
[ insert word for word verbadom ]
```

8) Assume the utilization of Hamming Codes for single-bit error correction.
   i) What is the total number of bits that need to be transmitted if the original data
string has 11 bits? Which positions will be covered by the different parity bits?
   ii) What is the total number of bits that need to be transmitted if the original data
string has 19 bits? Which positions will be covered by the different parity bits?

```
Do math here
```

9) Token Ring LANs have the clear advantage of avoiding collisions, which is surely not
the case for networks, i.e. Ethernet, that use buses as the main segment to connect the
different devices. In spite of that, Ethernet LANs are capable today of providing a
superior performance in comparison to Token Ring LANs. Explain how the Ethernet was
able to achieve that regardless of the collision issues. In particular, your answer should
consider: 
   1) the disadvantages of Token Rings
   2) Switched Ethernet.
   
```
Token ring requires massive overhead management in order to have the token move. This is
extremely costly and significantly reduce the max bandwidth from the time waste negoiating.
Token ring is heavy when its working, when hosts come and go with the token this becomes a
nightmare of complexity.

Switches save the day for ethernet, who's main delemia is collisions, this is completely
avoid by having a single wire between the switch anf the host. its the the switch who manages
internally to avoid collision on its bus or output ports.
```
