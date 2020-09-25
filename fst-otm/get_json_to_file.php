<?php
$start_time = microtime(true);
echo "Begin generation: ".date(DATE_RFC822);

require __DIR__ . '/../src/credentials.php';
require __DIR__ . '/../src/telegram_api.php';
require __DIR__ . '/vendor/autoload.php';

// +-------------------+
// |  Main definitions |
// +-------------------+
$generator_name = "Спортсмены";

// +-------------------+
// |  Results path     |
// +-------------------+
$json_output_file = "static_json_data.js";
$remote_file = "/www/fst-otm.yurboc.ru/static_json_data.js";

// +-------------------+
// |  Identifiers      |
// +-------------------+
$spreadsheetId = '1bkbojWWjPXLkOXGd6yZRqDVlsyXurWxL3ePQzvTM3Jc';
$range = 'Сводный!A2:K';

// +------------------------+
// |  Access to Google API  |
// +------------------------+
putenv( 'GOOGLE_APPLICATION_CREDENTIALS=' . $googleAccountKeyFilePath );

/**
 * Returns an authorized API client.
 * @return Google_Client the authorized client object
 */
function getClient()
{
    $client = new Google_Client();
    $client->useApplicationDefaultCredentials();
    $client->setApplicationName('Google Sheets API test for FST OTM');
    $client->addScope( 'https://www.googleapis.com/auth/spreadsheets.readonly' );
    $client->addScope( 'https://www.googleapis.com/auth/drive.readonly' );
    return $client;
}

// ----------------------------------------------------
//  Extract data from Google Sheets to JavaScript file
// ----------------------------------------------------

// Get the API client and construct the service object.
$client = getClient();
$service_sheets = new Google_Service_Sheets($client);
$service_drive = new Google_Service_Drive($client);

// Read data from spreadsheet
$response = $service_sheets->spreadsheets_values->get($spreadsheetId, $range);
$values = $response->getValues();
$result_data = array();
if (empty($values)) {
    print "No data found.";
} else {
    foreach ($values as $row) {
        array_push($result_data, array(
          'id'          => array_key_exists( 0, $row) ? $row[ 0] : "",
          'name'        => array_key_exists( 1, $row) ? $row[ 1] : "",
          'club'        => array_key_exists( 2, $row) ? $row[ 2] : "",
          'rank'        => array_key_exists( 3, $row) ? $row[ 3] : "",
          'date_apply'  => array_key_exists( 4, $row) ? $row[ 4] : "",
          'order_no'    => array_key_exists( 5, $row) ? $row[ 5] : "",
          'order_link'  => array_key_exists( 6, $row) ? $row[ 6] : "",
          'rank_state'  => array_key_exists( 7, $row) ? $row[ 7] : "",
          'date_expire' => array_key_exists( 8, $row) ? $row[ 8] : "",
          'prog_type'   => array_key_exists( 9, $row) ? $row[ 9] : "",
          'sport_type'  => array_key_exists(10, $row) ? $row[10] : ""
          ));
    }
}

// Take spreadsheet modification data
$response = $service_drive->files->get($spreadsheetId, array('fields' => 'name, modifiedTime'));
$document_name = $response->getName();
$modified_date = new DateTime($response->getModifiedTime());
$generated_date = new DateTime();

// Generate JavaScript
$result_string = "var php_data = " . json_encode($result_data, JSON_UNESCAPED_UNICODE|JSON_UNESCAPED_SLASHES) . ";\n";
$result_string .= "var modified_date=\"" . $modified_date->format("d.m.Y H:i:s") . "\";\n";
$result_string .= "var generated_date=\"" . $generated_date->format("d.m.Y H:i:s") . "\";\n";

// Save to file
file_put_contents($json_output_file, $result_string);

// Profiling file generation
$end_time = microtime(true);
$execution_time = ($end_time - $start_time)*1000;
echo "<br/>Generated in ".number_format($execution_time, 2, ',', '')." ms";

// -------------------------------
//  Upload JavaScript file to FTP
// -------------------------------

$start_uploading_time = microtime(true);

// Establish connection or exit
$conn_id = ftp_connect($ftp_server) or die("<br/>Can't connect to $ftp_server");

// Login to FTP server
$login_result = ftp_login($conn_id, $ftp_user_name, $ftp_user_pass);

// Enable passive mode
ftp_pasv($conn_id, true);

// Put file
if (ftp_put($conn_id, $remote_file, $json_output_file, FTP_ASCII)) {
    echo "<br/>File $json_output_file successfully uploaded!";
} else {
    echo "<br/>Can't upload file $json_output_file to server!";
    notifySendFail($generator_name, "сбой загрузки по FTP");
}

// Close connection
ftp_close($conn_id);

// Profiling FTP
$end_time = microtime(true);
$execution_time = ($end_time - $start_time)*1000;
$uploading_time = ($end_time - $start_uploading_time)*1000;
echo "<br/>Done in ".number_format($execution_time, 2, ',', '')." ms (upload ".number_format($uploading_time, 2, ',', '')." ms)";

// Send notification to Telegram
if(!isset($_GET["notify"]) || $_GET["notify"] !== "none") {
    notifySendDone($generator_name, $execution_time);
}

?>
