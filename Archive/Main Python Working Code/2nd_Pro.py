import mysql.connector
from datetime import date
from scapy.layers.l2 import ARP, Ether
from scapy.sendrecv import srp

def scan(ip, iface):
    arp_request = ARP(pdst=ip)
    ether = Ether(dst="ff:ff:ff:ff:ff:ff")
    packet = ether / arp_request
    result = srp(packet, timeout=10, verbose=0, iface=iface)[0]
    devices = [received.hwsrc for _, received in result]
    return devices

def mark_attendance(mac_address, current_date):
    try:
        # Connect to the MySQL database
        connection = mysql.connector.connect(
            host="localhost",
            user="root",
            password="12345678",
            database="wifi_attendance"
        )
        cursor = connection.cursor()

        # Check if the MAC address is in the table
        query = "SELECT * FROM device_attendance WHERE mac_address = %s"
        cursor.execute(query, (mac_address,))
        result = cursor.fetchall()

        if result:
            # If the MAC address is in the table, mark attendance
            update_query = "UPDATE device_attendance SET date = %s WHERE mac_address = %s"
            data = (current_date, mac_address)
            cursor.execute(update_query, data)
            print(f"Attendance marked for MAC address {mac_address} on {current_date}")
        else:
            print(f"MAC address {mac_address} not found in the table. Attendance not recorded.")

        connection.commit()

    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

if __name__ == "__main__":
    target_ip = "192.168.0.1/24"
    network_interface = "en0"
    devices = scan(target_ip, network_interface)

    current_date = date.today().strftime("%Y-%m-%d")

    for mac_address in devices:
        mark_attendance(mac_address, current_date)
