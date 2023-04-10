/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __CNSS_GW_UPDATE_H_
#define __CNSS_GW_UPDATE_H_

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/errqueue.h>
#include <linux/pkt_sched.h>
#include <linux/nl80211.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/filter.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/object-api.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink-types.h>

#include <sys/time.h>

/* Buffer size for netlink socket */
#define CNSS_WIFI_CMD_SOCK_PORT 650
#define CNSS_BUFSIZE 4096
#define CNSS_ERR(x) do { perror(x); return EXIT_FAILURE; } while (0);

#define CNSS_STRING_LEN 1023
#define CNSS_BUFFER_LEN (CNSS_STRING_LEN + 1)

/* QCA vendor command to update GW parameters */
#define QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG 102  /* qca-vendor.h */
#define OUI_QCA 0x001374
#define MAX_TABLE_SIZE 64

#define MAX_COUNT 64
#define IPV4_NUM_BYTES 4
#define IPV6_NUM_BYTES 16

/* Command port */
#define CNSS_CMD_SOCK_PORT(port, flags) \
	(0 == (port)) ? (flags) & ~NL_OWN_PORT : (flags) | NL_OWN_PORT;

/* parameters for vendor command */
enum qca_wlan_vendor_attr_gw_param_config {
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_GW_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV4_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV6_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_AFTER_LAST - 1,
};

struct cnss_gw_info {
	/* gw ip addr in network order
         * ip_addr[0] is used for ipv4
         * ip_addr[0] to ip_addr[3] is used for ipv6
	 */
	uint32_t ip_addr[4];
	/* gw ipv4 addr required by nl80211 vendor cmd */
	uint8_t ipv4_addr[4];
	/* gw ipv6 addr required by nl80211 vendor cmd */
	uint8_t ipv6_addr[16];
	/* interface idx */
	int ifidx;
	/* gw mac addr */
	uint8_t mac_addr[ETH_ALEN];
	/* interface name */
	char if_name[IF_NAMESIZE];
	/* ipv4 */
	uint8_t ipv4_type;
	/* ipv6 */
	uint8_t ipv6_type;
	/* ip length */
	uint8_t ip_len;
};

/* structure for gateway update */
struct cnss_gw_update {
	/* kernel netlink socket */
	int route_sockfd;
	int neigh_sockfd;
	/* kernel sock addr handle */
	struct sockaddr_nl rsockaddr;
	struct sockaddr_nl nsockaddr;
	/* netlink cmd socket to driver */
	struct nl_sock *wifi_sockfd;
	/* driver sock addr handle */
	struct sockaddr_nl dsockaddr;
	/* current gw info */
	struct cnss_gw_info curr_gw_info;
	/* new gw info */
	struct cnss_gw_info new_gw_info;
	/* gw updated status */
	unsigned char gw_updated;
};

struct arp_entry {
	unsigned long ip_addr;
	/* place holder for ip addr string */
	char a_ipstr[INET6_ADDRSTRLEN];
	/* one mac address per ip addr */
	unsigned char mac_addr[ETH_ALEN];
	/* every entry will have a corresponding device */
	char ifname[IF_NAMESIZE];
};

struct arp_table {
	struct arp_entry a_entry[MAX_TABLE_SIZE];
};

struct route_entry_tag {
	/* source address */
	uint32_t src_addr;
	/* dest address */
	uint32_t dst_addr;
	/* gw address */
	uint32_t gw_addr[4];
	/* gw address len */
	uint32_t gw_addr_len;
	/* ifname */
	char ifname[IF_NAMESIZE];
};

struct route_table {
	struct route_entry_tag r_entry[MAX_TABLE_SIZE];
};

struct cnss_gw_data {
	struct arp_table a_table;
	struct route_table r_table;
	unsigned int r_table_idx;
	/* used for neighbor request */
	int neighbor_success;
	int new_route;
	/* handle for NL CB */
	struct nl_cb *g_cb;
};

/* Gateway update APIs */
int cnss_gw_update_init();
int cnss_gw_update_loop(int sock);
int cnss_update_wlan_drv();
int cnss_get_gw_update_sockfd();
int cnss_gw_update_deinit();

#endif  /* __CNSS_GW_UPDATE_H_ */
