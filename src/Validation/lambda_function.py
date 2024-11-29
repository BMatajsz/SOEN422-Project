import json
import os
import boto3
import datetime


def convertTime(time):
    secondsInDay = 24 * 60 * 60
    newTime = divmod(time.days * secondsInDay + time.seconds, 60)
    trueTime = newTime[0] * 60 + newTime[1]
    return trueTime


def calculateTimeDiff(firstTime, laterTime):
    difference = laterTime - firstTime
    newTime = convertTime(difference)
    return newTime


def getData(primaryKey, primaryKeyVal, table):
    key = {primaryKey: primaryKeyVal}

    response = table.get_item(Key=key)
    return response


def incrementItem(primaryKey, primaryVal, table, attribute):
    key = {primaryKey: primaryVal}

    response = table.update_item(
        Key=key,
        UpdateExpression=f"SET {attribute} = if_not_exists({attribute}, :start) + :inc",
        ExpressionAttributeValues={
            ":start": 0,  # Initialize Counter to 0 if it doesn't exist
            ":inc": 1,  # Increment value
        },
        ReturnValues="UPDATED_NEW",  # Returns the updated attributes
    )
    return response


def updateItem(primaryKey, primaryVal, table, attribute, newVal):
    key = {primaryKey: primaryVal}

    response = table.update_item(
        Key=key,
        UpdateExpression=f"SET {attribute} = :newval",
        ExpressionAttributeValues={":newval": newVal},
        ReturnValues="UPDATED_NEW",  # Returns the updated attributes
    )
    return response


def addItem(primaryKey, primaryVal, table, attribute, newVal):
    name = ""
    studentID = ""

    if primaryVal == "253cda3f":
        name = "Bence Matajsz"
        studentID = "40322797"
    elif primaryVal == "00000000":
        name = "John Doe"
        studentID = "12345678"

    response = table.put_item(
        Item={
            primaryKey: primaryVal,  # Replace with your Partition Key
            attribute: newVal,  # Replace with your Sort Key (if applicable)
            "Name": name,
            "StudentID": studentID,
        }
    )
    return response


def checkOut(attendanceData, attendanceTime, uid, summaryTable):
    firstTime = attendanceData["Timestamp"]
    diff = calculateTimeDiff(firstTime, datetime.datetime.now())
    if diff >= attendanceTime:
        incrementItem("CardUID", uid, summaryTable, "Sessions")
        return {"statusCode": 200, "body": json.dumps("true")}
    else:
        return {"statusCode": 200, "body": json.dumps("Insufficient time")}


def checkIn(sessionData, attendanceTime, uid, attendanceTable):
    timeLeft = calculateTimeDiff(datetime.datetime.now(), sessionData["End"])
    if timeLeft >= attendanceTime:
        addItem(
            "CardUID",
            uid,
            attendanceTable,
            "Timestamp",
            convertTime(datetime.datetime.now()),
        )
    else:
        return {"statusCode": 200, "body": json.dumps("Insufficient time")}


def lambda_handler(event, context):
    # TODO implement
    # Initialize DynamoDB client
    dynamodb = boto3.resource("dynamodb")

    # Environment variables for table names and initializing the tables
    SESSIONS_TABLE = os.environ.get("SESSIONS_TABLE")
    STUDENT_ATTENDANCE_TABLE = os.environ.get("STUDENT_ATTENDANCE_TABLE")
    STUDENT_SUMMARY = os.environ.get("STUDENT_SUMMARY_TABLE")
    STUDENT_REGISTRATION_TABLE = os.environ.get("STUDENT_REGISTRATION_TABLE")
    sessionsTable = dynamodb.Table(SESSIONS_TABLE)
    attendanceTable = dynamodb.Table(STUDENT_ATTENDANCE_TABLE)
    summaryTable = dynamodb.Table(STUDENT_SUMMARY)
    registrationTable = dynamodb.Table(STUDENT_REGISTRATION_TABLE)

    # Processing the data from the POST request
    uid = event["UID"].replace(" ", "")

    # Getting the session information
    sessionData = getData("SessionID", 0, sessionsTable)

    # Getting the registered student and the required attendance time for the session
    registeredStudent = getData("CardUID", uid, registrationTable)
    attendanceTime = sessionData["AttendanceTime"]

    # Getting the attendance information
    attendanceData = getData("CardUID", uid, attendanceTable)

    # If the student is not registered, validation fails
    if registeredStudent:
        return {"statusCode": 200, "body": json.dumps("Not registered")}

    if attendanceData:
        checkOut(attendanceData, attendanceTime, uid, summaryTable)
    else:
        checkIn(sessionData, attendanceTime, uid, attendanceTable)

    return {"statusCode": 200, "body": json.dumps("Hello from Lambda!")}


# New Whole Pseudo-code
# 1. Process the event (read card UID from the post request)
# 2. Search DynamoDB if the card UID is registered for the session
#       If yes:
#           Search DynamoDB if the card UID is checked-in for the session
#           If yes:
#               Get the required attendance time from the DynamoDB session
#               Subtract the two timestamps
#               If the result is >= the required time:
#                   Return true, get and increment attendance for student in DynamoDB, remove from checked-in students
#               Else:
#                   Return false, and error message (Insufficient attendance)
#           Else:
#               Get the required attendance time from the DynamoDB session
#               See if there is enough time left for the attendance to count
#               If yes:
#                   Update the DynamoDB current session to include the student and record time
#               Else:
#                   Return false, and error message (Insufficient attendance)
#       Else:
#           Return false and an error message (student not registered)
