import sys
import requests
import os
from dotenv import load_dotenv
import json


def listCurrentStudents(url):
    payload = {"path": "/checked-in"}
    response = requests.get(url, json=payload)
    if response.status_code == 200:
        students = json.loads(response.json()["body"])
        print("Current Students in Class:")
        for student in students:
            print(f"- {student["Name"]}, {student["StudentID"]}")
    else:
        print("Failed to retrieve current students.")


def listAllStudents(url):
    payload = {"path": "/registered"}
    response = requests.get(url, json=payload)
    if response.status_code == 200:
        students = json.loads(response.json()["body"])
        print("All Registered Students:")
        for student in students:
            print(f"- {student['Name']}, {student["StudentID"]}")
    else:
        print("Failed to retrieve all students.")


def createAnalytics(url):
    payload = {"path": "/analytics"}
    response = requests.get(url, json=payload)
    if response.status_code == 200:
        analytics = json.loads(response.json()["body"])
        calculateAnalytics(analytics)
    else:
        print("Failed to retrieve analytics.")


def dismissAttendance(url):
    payload = {"path": "/dismiss"}
    response = requests.post(url, json=payload)
    if response.status_code == 200:
        print("Attendance for the session has been dismissed.")
    else:
        print("Failed to dismiss attendance.")


def endSession(url):
    payload = {"path": "/end"}
    response = requests.post(url, json=payload)
    print(response)
    if response.status_code == 200:
        print("Attendance for the session has been dismissed.")
    else:
        print("Failed to end session.")


def calculateAnalytics(response):
    totalSessions = len(response["sessions"])
    registeredStudents = 0
    avgAttendance = 0.0

    for student in response["registrations"]:
        registeredStudents += 1
    for student in response["summaries"]:
        avgAttendance += float(student["Sessions"]) / float(totalSessions)

    print(f"Total number of sessions: {totalSessions}")
    print(f"Total number of registered students: {registeredStudents}")
    print(f"Attendance rate: {avgAttendance * 100:.1f}%")


def menu():
    load_dotenv()
    baseUrl = os.getenv("API_BASE_URL")
    print(baseUrl)

    while True:
        print("\nSmart Attendance Tracking System")
        print("1. List current students in class")
        print("2. List all registered students with attendance number")
        print("3. Create attendance analytics")
        print("4. Dismiss attendance time for session")
        print("5. Exit")
        print("--------------------------------------------------------")
        print("Simulation commands:")
        print("6. End of session")
        choice = input("Enter your choice: ")
        if choice == "1":
            listCurrentStudents(baseUrl)
        elif choice == "2":
            listAllStudents(baseUrl)
        elif choice == "3":
            createAnalytics(baseUrl)
        elif choice == "4":
            dismissAttendance(baseUrl)
        elif choice == "5":
            sys.exit()
        elif choice == "6":
            endSession(baseUrl)
        else:
            print("Invalid choice. Please try again.")


if __name__ == "__main__":
    menu()
