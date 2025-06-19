function doPost(e) {
  try {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    const usersSheet = ss.getSheetByName("Users");
    const logsSheet = ss.getSheetByName("Logs");

    if (!usersSheet || !logsSheet) {
      return ContentService
        .createTextOutput(JSON.stringify({ status: "error", message: "Sheets not found" }))
        .setMimeType(ContentService.MimeType.JSON);
    }

    const userID = e.parameter.user_id;
    if (!userID) {
      return ContentService
        .createTextOutput(JSON.stringify({ status: "error", message: "Missing user_id" }))
        .setMimeType(ContentService.MimeType.JSON);
    }

    const usersData = usersSheet.getDataRange().getValues();
    let matchedUser = null;

    for (let i = 1; i < usersData.length; i++) {
      if (usersData[i][0] === userID) {
        matchedUser = usersData[i];
        break;
      }
    }

    const now = new Date();
    const date = Utilities.formatDate(now, "GMT+3", "yyyy-MM-dd");
    const time = Utilities.formatDate(now, "GMT+3", "HH:mm:ss");

    if (matchedUser && matchedUser[1]) {
      // User is found and has details
      logsSheet.appendRow([
        userID,
        matchedUser[1], // Firstname
        matchedUser[2], // Lastname
        matchedUser[3], // Position
        matchedUser[4], // Status
        date,
        time
      ]);

      return ContentService
        .createTextOutput(JSON.stringify({
          status: "success",
          name: matchedUser[1],
          position: matchedUser[3],
          status: matchedUser[4],
          message: "User logged successfully"
        }))
        .setMimeType(ContentService.MimeType.JSON);
    } else {
      // Not found or incomplete data
      if (!matchedUser) {
        usersSheet.appendRow([userID, "", "", "", ""]);
      }

      return ContentService
        .createTextOutput(JSON.stringify({
          status: "pending",
          message: "User not registered. Added to Users sheet for admin review."
        }))
        .setMimeType(ContentService.MimeType.JSON);
    }

  } catch (error) {
    return ContentService
      .createTextOutput(JSON.stringify({ status: "error", message: error.message }))
      .setMimeType(ContentService.MimeType.JSON);
  }
}

