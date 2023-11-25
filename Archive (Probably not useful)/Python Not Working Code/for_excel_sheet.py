import mysql.connector
from datetime import date
from scapy.layers.l2 import ARP, Ether
from scapy.sendrecv import srp

def scan(ip, iface):
    arp_request = ARP(pdst=ip)
    ether = Ether(dst="ff:ff:ff:ff:ff:ff")
    packet = ether / arp_request
    result = srp(packet, timeout=10, verbose=0, iface=iface)[0]
    devices = []
    for sent, received in result:
        devices.append({'mac': received.hwsrc})
    return devices

def insert_data(enrollment_number, mac_address, date):
    try:
        # Connect to the MySQL database
        connection = mysql.connector.connect(
            host="localhost",
            user="root",
            password="12345678",
            database="wifi_attendance"
        )
        cursor = connection.cursor()

        # Check if the MAC address is already in the table
        query = "SELECT * FROM device_attendance WHERE mac_address = %s"
        cursor.execute(query, (mac_address,))
        result = cursor.fetchall()

        if not result:
            # If the MAC address is not in the table, insert a new row
            insert_query = "INSERT INTO device_attendance (enrollment_number, mac_address, date) VALUES (%s, %s, %s)"
            data = (enrollment_number, mac_address, date)
            cursor.execute(insert_query, data)
            print(f"Attendance marked for {enrollment_number} with MAC address {mac_address} on {date}")
        else:
            # If the MAC address is already in the table, update the existing row
            update_query = "UPDATE device_attendance SET date = %s WHERE mac_address = %s"
            data = (date, mac_address)
            cursor.execute(update_query, data)
            print(f"Attendance updated for {enrollment_number} with MAC address {mac_address} on {date}")

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

    for device in devices:
        mac_address = device['mac']
        enrollment_number = input(f"Enter the enrollment number for MAC address {mac_address}: ")
        current_date = date.today().strftime("%Y-%m-%d")
        insert_data(enrollment_number, mac_address, current_date)
