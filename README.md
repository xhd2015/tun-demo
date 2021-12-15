# What is tun/tap?
Refer to here: [https://www.kernel.org/doc/Documentation/networking/tuntap.txt](https://www.kernel.org/doc/Documentation/networking/tuntap.txt)

In this simple project we will implement a tun device demo.

# Quick start
On linux, build and start the program:
```bash
gcc -o tun_demo main.c
sudo ./tun_demo
```

Open another terminal, setup the new `tun0` device's ip:
```bash
ip addr add dev tun0 local 10.11.12.13/24
ip link set dev tun0 up
```

After `tun0` is assigned an IP address, a related subnet route is added to ip route table also,check that with:
```bash
ip route|grep tun0
```

Now, ping another non-existing IP, and see the original `tun_demo` terminal's output:
```bash
ping 10.11.12.14
```

# Furthermore, make ICMP Echo Reply
We can simple exchange the IP Packet's source ip and destination ip,and write the packet back to `tun_fd`,this will make the ping client receive ICMP Echo Reply.

