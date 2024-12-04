import sys
import requests
import os
from dotenv import load_dotenv
import json
import smtplib
from email.message import EmailMessage


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
    response = requests.get(url, json=payload)
    if response.status_code == 200:
        print("Attendance for the session has been dismissed.")
    else:
        print("Failed to dismiss attendance.")


def endSession(url):
    payload = {"path": "/end"}
    response = requests.get(url, json=payload)
    print(response)
    if response.status_code == 200:
        data = json.loads(response.json()["body"])
        attendanceNumber = data["attendanceNumber"]
        registrations = data["registrations"]
        summaries = data["summary"]

        emails = {}
        for student in registrations:
            emails[student["CardUID"]] = student["Email"]

        for student in summaries:
            currAttendance = data["summary"][0]["Sessions"]
            if currAttendance < attendanceNumber:
                sendEmail(emails[student["CardUID"]])

        print("Email sent. Session ended.")
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


def increaseAttendance(url):
    payload = {"path": "/increase"}
    response = requests.get(url, json=payload)
    if response.status_code == 200:
        print("Attendance for the session has been increased.")
    else:
        print("Failed to increase attendance.")


def sendEmail(address):
    # Email credentials and details
    senderEmail = "bence.matajsz12@gmail.com"
    receiverEmail = address
    appPassword = os.getenv("GOOGLE_APP_PASSWORD")

    # Create the email content
    msg = EmailMessage()
    msg.set_content(
        "This is a warning regarding attendance. Not attending sessions in the future could impact your grade heavily."
    )
    msg["Subject"] = "[SOEN 422] Attendance Warning"
    msg["From"] = senderEmail
    msg["To"] = receiverEmail

    # Send the email
    try:
        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
            server.login(senderEmail, appPassword)
            server.send_message(msg)
        print("Email sent successfully")
    except Exception as e:
        print(f"Failed to send email: {e}")


def addTag(url):
    payload = {"path": "/tag"}
    response = requests.get(url, json=payload)
    if response.status_code == 200:
        print("Tag added to registrations.")
    else:
        print("Failed to add tag to registrations.")


def menu():
    load_dotenv()
    baseUrl = os.getenv("API_BASE_URL")
    print(baseUrl)

    while True:
        print("\nSmart Attendance Tracking System")
        print("1. List current students in class")
        print("2. List all registered students")
        print("3. Create attendance analytics")
        print("4. Dismiss attendance time for session")
        print("5. Increase attendance time for session")
        print("6. End of session")
        print("7. Add tag to registrations")
        print("8. Exit")
        print("--------------------------------------------------------")

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
            increaseAttendance(baseUrl)
        elif choice == "6":
            endSession(baseUrl)
        elif choice == "7":
            addTag(baseUrl)
        elif choice == "8":
            sys.exit()
        else:
            print("Invalid choice. Please try again.")


if __name__ == "__main__":
    menu()
