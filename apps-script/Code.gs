// Google Apps Script web app: receives ?temperature=..&humidity=.. and appends a row.
// Deploy: Deploy → New deployment → Web app · Execute as: Me · Who has access: Anyone.
// After editing, redeploy a new version or the web app keeps running the old code.

var SPREADSHEET_ID = "PUT_YOUR_SPREADSHEET_ID_HERE";

function doGet(e) {
  var sheet = SpreadsheetApp.openById(SPREADSHEET_ID).getSheets()[0];

  if (sheet.getLastRow() === 0) {
    sheet.appendRow(["Timestamp", "Temperature (°C)", "Humidity (%)"]);
  }

  var temp = e.parameter.temperature;
  var humid = e.parameter.humidity;
  sheet.appendRow([new Date(), temp, humid]);

  return ContentService.createTextOutput("OK: Temp=" + temp + " Humid=" + humid);
}
