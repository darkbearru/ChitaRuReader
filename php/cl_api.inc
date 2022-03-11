<?

class API 
{
    protected $_db;
    protected $_settings;


    public function __construct ()
    {
        $this->_db = DB_PORTAL ();
        $this->_settings = (object) [
            "on_page"   => 20
        ];

        // Роутинг
        $this->Route ();
    }

    public function Route ()
    {
        if (empty ($_SERVER['REQUEST_URI'])) return false;

        // Выделяем параметры URL
        $_url       = explode ('?', $_SERVER['REQUEST_URI']);
        $_urlParams = $_url[1];
        $_command   = $this->parseURL ($_url, $_urlParams);

        $_commandFilename = "cl_{$_command}.inc";

        if (file_exists ($_commandFilename)) {
            include ($_commandFilename);
            $_command = "{$_command}Class";
            if (class_exists ($_command)) {
                $_cmd = new $_command ($_url, $_urlParams);
                $_cmd->Run ();
            } else {
                $this->Error ("Command not found", 3);
            }
        } else {
            $this->Error ("Command not found", 3);
        }
    }

    /**
     * Разбор URL
     */
    private function parseURL (&$url, &$params)
    {
        if (!empty ($params)) {
            $params = $this->decodeUrlParams ($params);
        }

        // Убираем лишние слэши и рабочую папку
        $url = preg_replace('/(^\/|api-v2\/|\/$)/uism', '', $url[0]);
        // Получаем команду и её параметры
        if (empty ($url))  $this->Error ("Command not found");

        $url = explode ('/', $url);

        if (!count ($url)) $this->Error ("Command not found", 2);

      
        $_command = array_splice ($url, 0, 1);
        $_command = $_command[0];

        return $_command;
    }

    protected function Json ($result)
    {
        if (empty ($result->error)) {
            $result = (object) [
                'result'    => $result,
                'error'     => 0,
                'errorText' => ''
            ];
        }
        echo json_encode ($result, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
        exit;
    }

    protected function Error ($message, $code = 1)
    {
        $this->Json ((object) [
            'error'     => $code,
            'errorText' => $message 
        ]);
    }

    /**
     * Разбор URL по типу параметр = значение
     */
    protected function decodeUrlParams ($url)
    {
        $_list = explode ('&', $url);
        $_return = [];

        foreach ($_list as $param) {
            $_param = explode ('=', $param);
            $_value = (!empty ($_param[1]) ? $_param[1] : '');
            $_return[] = (object) [
                $_param[0] => $_value
            ];
        }
        return $_return;
    }
}