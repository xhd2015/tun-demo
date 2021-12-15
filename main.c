#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

// 
//int open_tun(){
//	int tapfd = open("/dev/net/tun", O_RDWR);
//	if (tapfd < 0) {
//		perror("open");
//		return; // Or otherwise handle the error.
//	}
//
//	struct ifreq ifr;
//	// Set up the ioctl request
//	memset(&ifr, 0, sizeof(ifr));
//	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
//
//	// Call the ioctl
//	int err = ioctl(tapfd, TUNSETIFF, (void *)&ifr);
//	if (err < 0) {
//		perror("ioctl");
//		return; // Or otherwise handle the error.
//	}
//}

// Write a packet to the network.
//
// Returns 0 on success, or a negative error number on error.
int send_data(int tapfd, char *packet, int len) {
	int wlen;

	wlen = write(tapfd, packet, len);
	if (wlen < 0) {
		perror("write");
	}

	return wlen;
}

int tun_alloc(char *dev, int flags) {
	struct ifreq ifr;
	int fd, err;
	char *clonedev = "/dev/net/tun";

	/* Arguments taken by the function:
	 *
	 * char *dev: the name of an interface (or '\0'). MUST have enough
	 *   space to hold the interface name if '\0' is passed
	 * int flags: interface flags (eg, IFF_TUN etc.)
	 */

	/* open the clone device */
	if( (fd = open(clonedev, O_RDWR)) < 0 ) {
		return fd;
	}

	/* preparation of the struct ifr, of type "struct ifreq" */
	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

	if (dev) {
		/* if a device name was specified, put it in the structure; otherwise,
		 * the kernel will try to allocate the "next" device of the
		 * specified type */
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	/* try to create the device */
	if( (err = ioctl(fd,TUNSETIFF, (void *) &ifr)) < 0 ) {
		close(fd);
		return err;
	}

	// if dev not specified, the name is picked by the kernel
	if(dev){
		/* if the operation was successful, write back the name of the
		 * interface to the variable "dev", so the caller can know
		 * it. Note that the caller MUST reserve space in *dev (see calling
		 * code below) */
		strcpy(dev, ifr.ifr_name);
	}


	/* this is the special file descriptor that the caller will use to talk
	 * with the virtual interface */
	return fd;
}

char hex(int i){
	if(i<10){
		return (char)('0'+i);
	}
	return (char)('a'+(i-10));
}
void print_ip_str(char ip[]){
    printf("%u.%u.%u.%u", (unsigned int)(unsigned char)ip[0],(unsigned int)(unsigned char)ip[1],(unsigned int)(unsigned char)ip[2],(unsigned int)(unsigned char)ip[3]);
}

void do_tun(){
        printf("start do_tun\n");
	char buffer[1500];
        int nread;
	const char *tun_name = (const char *)0;
	int tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);  /* tun interface */
	if(tun_fd < 0){
		perror("Allocating interface");
		exit(1);
	}
        printf("allocated tun device, fd=%d\n",tun_fd);
	/* Now read data coming from the kernel */
	while(1) {
		/* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
		nread = read(tun_fd,buffer,sizeof(buffer));
		if(nread < 0) {
			perror("Reading from interface");
			close(tun_fd);
			exit(1);
		}
                char source_ip[4];
                char dest_ip[4];
                memcpy(source_ip,&buffer[12],4);
                memcpy(dest_ip,&buffer[16],4);
                printf("src ip:");
                print_ip_str(source_ip);
                printf(",dst ip:");
                print_ip_str(dest_ip);
                printf("\n");

		/* Do whatever with the data */
		printf("Read %d bytes from device %s\n", nread, tun_name);
		printf(">>|");
		for(int i=0;i<nread;i++){
			printf("%c%c ",hex((buffer[i]>>4)&0xf), hex(buffer[i]&0xf));
		}
		printf("|\n");
                memset(buffer,0,sizeof(buffer));
	}
}

int main(){
	do_tun();
	return 0;
}
