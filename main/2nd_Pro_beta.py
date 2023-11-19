import mysql.connector
from datetime import date

def insert_data(enrollment_number, mac_address, current_date):
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
            insert_query = "INSERT INTO device_attendance (enrollment_number, mac_address, {}) VALUES (%s, %s, %s)".format(current_date)
            data = (enrollment_number, mac_address, "Present")
            cursor.execute(insert_query, data)
            print(f"Attendance marked for {enrollment_number} with MAC address {mac_address} on {current_date}")
        else:
            # If the MAC address is already in the table, update the existing row
            update_query = "UPDATE device_attendance SET {} = %s WHERE mac_address = %s".format(current_date)
            data = ("Present", mac_address)
            cursor.execute(update_query, data)
            print(f"Attendance updated for {enrollment_number} with MAC address {mac_address} on {current_date}")

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

    for device in devices:
        mac_address = device['mac']
        enrollment_number = input(f"Enter the enrollment number for MAC address {mac_address}: ")
        insert_data(enrollment_number, mac_address, current_date)
