<?

error_reporting (E_ALL);
include_once ('cl_api.inc');

// Подключение к DB
include_once ($_SERVER['DOCUMENT_ROOT'].'/../core/core.inc');
init (false, false);


header ('Content-type: application/json; charset=utf-8');
header ('Last-Modified: '.date ('Y-m-d H:i:s'));


$api = new API ();

?>