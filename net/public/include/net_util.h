/*

*/

#ifndef NET_UTIL_H_
#define NET_UTIL_H_

#include "net.h"
#include "net_export.h"

#include <vector>
#include <string>

#if defined(PLATFORM_API_WINDOWS)
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# error "Net is unknown"
#endif

BEGIN_NAMESPACE_NET

//////////////////////////////////////////////////////////////////////////
// IPAddressNumber is used to represent an IP address's numeric value as an
// array of bytes, from most significant to least significant. This is the
// network byte ordering.
//
// IPv4 addresses will have length 4, whereas IPv6 address will have length 16.
typedef std::vector<unsigned char> IPAddressNumber;
typedef std::vector<IPAddressNumber> IPAddressList;

typedef IPAddressNumber     MACAddressNumber;
static const size_t kIPv4AddressSize = 4;
static const size_t kIPv6AddressSize = 16;

NET_EXPORT uint32_t IPAddressToString(char_t* strIP, const uint8_t* address, size_t len);

// Same as IPAddressToString() but for an IPAddressNumber.
NET_EXPORT std::string IPAddressToString(const IPAddressNumber& addr);

// Extracts the IP address and port portions of a sockaddr. |port| is optional,
// and will not be filled in if NULL.
bool GetIPAddressFromSockAddr(const struct sockaddr* sock_addr,
                              socklen_t sock_addr_len,
                              const unsigned char** address,
                              size_t* address_len,
                              uint16_t* port);

// Returns the string representation of an IP address.
// For example: "192.168.0.1" or "::1".
NET_EXPORT std::string IPAddressToString(const uint8_t* address,
                                         size_t address_len);

// Returns the string representation of an IP address along with its port.
// For example: "192.168.0.1:99" or "[::1]:80".
NET_EXPORT std::string IPAddressToStringWithPort(const uint8_t* address,
                                                 size_t address_len,
                                                 uint16_t port);

// Same as IPAddressToString() but for a sockaddr. This output will not include
// the IPv6 scope ID.
NET_EXPORT std::string NetAddressToString(const struct sockaddr* sa,
                                          socklen_t sock_addr_len);

// Same as IPAddressToStringWithPort() but for a sockaddr. This output will not
// include the IPv6 scope ID.
NET_EXPORT std::string NetAddressToStringWithPort(const struct sockaddr* sa,
                                                  socklen_t sock_addr_len);

// Same as IPAddressToString() but for an IPAddressNumber.
NET_EXPORT std::string IPAddressToString(const IPAddressNumber& addr);

// Same as IPAddressToStringWithPort() but for an IPAddressNumber.
NET_EXPORT std::string IPAddressToStringWithPort(
  const IPAddressNumber& addr, uint16_t port);


// Returns the hostname of the current system. Returns empty string on failure.
NET_EXPORT std::string GetHostName();


// struct that is used by GetNetworkList() to represent a network
// interface.
struct NET_EXPORT NetworkInterface {
  NetworkInterface();
  NetworkInterface(const std::string& name, const IPAddressNumber& address);
  ~NetworkInterface();

  std::string name;
  IPAddressNumber address;

  NetworkInterface(const std::string& name, const IPAddressNumber& address, const IPAddressNumber& mac);
  MACAddressNumber mac;
};

typedef std::vector<NetworkInterface> NetworkInterfaceList;

// Returns list of network interfaces except loopback interface. If an
// interface has more than one address, a separate entry is added to
// the list for each address.
// Can be called only on a thread that allows IO.
NET_EXPORT bool_t GetNetworkList(NetworkInterfaceList* networks);

END_NAMESPACE_NET

#endif // NET_UTIL_H_
