import sys
import requests

API_BASE_URL = "https://your-api-endpoint.com"  # Replace with your actual API base URL


def list_current_students():
    response = requests.get(f"{API_BASE_URL}/current_students")
    if response.status_code == 200:
        students = response.json()
        print("Current Students in Class:")
        for student in students:
            print(f"- {student['name']}")
    else:
        print("Failed to retrieve current students.")


def list_all_students():
    response = requests.get(f"{API_BASE_URL}/all_students")
    if response.status_code == 200:
        students = response.json()
        print("All Registered Students:")
        for student in students:
            print(f"- {student['name']}: {student['attendance_number']} attendances")
    else:
        print("Failed to retrieve all students.")


def create_analytics():
    response = requests.get(f"{API_BASE_URL}/attendance_analytics")
    if response.status_code == 200:
        analytics = response.json()
        print("Attendance Analytics:")
        print(f"Attendance Rate: {analytics['attendance_rate']}%")
    else:
        print("Failed to retrieve analytics.")


def dismiss_attendance():
    response = requests.post(f"{API_BASE_URL}/dismiss_attendance")
    if response.status_code == 200:
        print("Attendance for the session has been dismissed.")
    else:
        print("Failed to dismiss attendance.")


def menu():
    while True:
        print("\nSmart Attendance Tracking System")
        print("1. List current students in class")
        print("2. List all registered students with attendance number")
        print("3. Create attendance analytics")
        print("4. Dismiss attendance for session")
        print("5. Exit")
        choice = input("Enter your choice: ")
        if choice == "1":
            list_current_students()
        elif choice == "2":
            list_all_students()
        elif choice == "3":
            create_analytics()
        elif choice == "4":
            dismiss_attendance()
        elif choice == "5":
            sys.exit()
        else:
            print("Invalid choice. Please try again.")


if __name__ == "__main__":
    menu()
