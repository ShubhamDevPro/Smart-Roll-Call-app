from scapy.layers.l2 import ARP, Ether
from scapy.sendrecv import srp
def scan(ip, iface):
    arp_request = ARP(pdst=ip)
    ether = Ether(dst="ff:ff:ff:ff:ff:ff")
    packet = ether / arp_request
    result = srp(packet, timeout=10, verbose=0, iface=iface)[0]
    devices = []
    for sent, received in result:
        devices.append({'ip': received.psrc, 'mac': received.hwsrc})
    return devices

def print_result(devices_list):
    print("IP Address\t\tMAC Address")
    print("-----------------------------------------")
    for device in devices_list:
        print(f"{device['ip']}\t\t{device['mac']}")

if __name__ == "__main__":
    target_ip = "192.168.0.1/24"  # Adjust this to match your network's IP range
    network_interface = "en0"  # Adjust this to match your network interface
    devices = scan(target_ip, network_interface)
    print_result(devices)
