import json
import os
import boto3


def getData(primaryKey, primaryKeyVal, sortKey, sortKeyVal, table):
    key = {primaryKey: primaryKeyVal, sortKey: sortKeyVal}

    response = table.get_item(Key=key)
    return response


def updateItem(primaryKey, primaryVal, table, attribute, newVal, sortKey, sortVal):
    key = {primaryKey: primaryVal, sortKey: sortVal}

    response = table.update_item(
        Key=key,
        UpdateExpression=f"SET {attribute} = :newval",
        ExpressionAttributeValues={":newval": newVal},
        ReturnValues="UPDATED_NEW",  # Returns the updated attributes
    )
    return response


def listStudents(table):
    response = table.scan()
    data = response["Items"]

    return {"statusCode": 200, "body": json.dumps(data)}


def analytics(sessionsTable, registrationTable, summaryTable):
    sessions_response = sessionsTable.scan()
    sessions_data = sessions_response["Items"]

    registration_response = registrationTable.scan()
    registration_data = registration_response["Items"]

    summary_response = summaryTable.scan()
    summary_data = summary_response["Items"]

    print(sessions_data)
    print(registration_data)
    print(summary_data)

    return {
        "statusCode": 200,
        "body": json.dumps(
            {
                "sessions": sessions_data,
                "registrations": registration_data,
                "summaries": summary_data,
            }
        ),
    }


def dismissAttendanceTime(sessionsTable):
    updateItem(
        "SessionID", "0", sessionsTable, "AttendanceTime", "0", "Date", "03122024"
    )
    return {"statusCode": 200, "body": json.dumps("OK")}


def endSession(attendanceTable, registrationTable, attendanceNeeded):
    scan = attendanceTable.scan()
    with attendanceTable.batch_writer() as batch:
        for each in scan["Items"]:
            batch.delete_item(
                Key={"CardUID": each["CardUID"], "Timestamp": each["Timestamp"]}
            )
    scan2 = registrationTable.scan()
    registrationData = scan2["Items"]
    return {
        "statusCode": 200,
        "body": json.dumps(
            {"attendanceNumber": attendanceNeeded, "registrations": registrationData}
        ),
    }


def lambda_handler(event, context):
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

    if event["path"] == "/checked-in":
        return listStudents(attendanceTable)
    elif event["path"] == "/registered":
        return listStudents(registrationTable)
    elif event["path"] == "/analytics":
        return analytics(sessionsTable, registrationTable, summaryTable)
    elif event["path"] == "/dismiss":
        return dismissAttendanceTime(sessionsTable)
    elif event["path"] == "/end":
        data = getData("SessionID", "0", "Date", "03122024", sessionsTable)
        print(data)
        attendanceNeeded = data["Item"]["AttendanceNumber"]
        return endSession(attendanceTable, registrationTable, attendanceNeeded)
    else:
        return {"statusCode": 404, "body": json.dumps("Not Found")}


""" PseudoCode

1. List all checked in students:
        Return the whole attendance dataset
2. List all registered students:
        Return the whole registration dataset
3. Analytics:
        Return the whole session dataset
        Return the whole registration dataset
        Return the whole summary dataset
4. Dismiss attendance:
        Modify the session dataset so the attendance time becomes 0
5. End session:
        Return the dataset of the registered students """
