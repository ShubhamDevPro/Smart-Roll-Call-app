import mysql.connector
from datetime import date

def manual_entry():
    mac_address = input("Enter the MAC address of the device: ")
    enrollment_number = input(f"Enter the enrollment number for MAC address {mac_address}: ")
    current_date = date.today().strftime("%Y-%m-%d")

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
            data = (enrollment_number, mac_address, current_date)
            cursor.execute(insert_query, data)
            print(f"Attendance marked for {enrollment_number} with MAC address {mac_address} on {current_date}")
        else:
            print(f"MAC address {mac_address} already exists in the table. Attendance not recorded.")

        connection.commit()

    except mysql.connector.Error as err:
        print(f"Error: {err}")

    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

if __name__ == "__main__":
    manual_entry()
