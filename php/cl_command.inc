<?
/**
 * База, для всех комманд API
 */
abstract class Command
{
    protected $_db;
    protected $_parameters;
    protected $_urlParameters;
    protected $_imageTypes = ['gif' => 1, 'jpg' => 2, 'jpeg' => 2, 'png' => 3, 'tiff' => 7];


    public function __construct ($parameters, $urlParameters = false)
    {
        $this->_db = DB_PORTAL();
        $this->_parameters      = $parameters;
        $this->_urlParameters   = $urlParameters;
    }

    /**
     * Общий метод запуска команды, вызываемый через API
     */
    abstract public function Run ();

    /**
     * Общий метод постобработки данных
     */
    abstract protected function PostProcess ($records);

    /**
     * Отображение ошибок
     */
    protected function Error ($message, $code = 1)
    {
        $this->JSON ((object) [
            'error'     => $code,
            'errorText' => $message 
        ]);
    }

    /**
     * Отображение результатов работы в JSON
     */
    protected function JSON ($result)
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

    /**
     * Получение фотографии из базы по её ID
     */
    protected function Photo ($id, $size = '300x180')
    {
        if (empty ($id)) return null;

        $_db = DB_COMMON ();


		$record = $_db->qry_c (
            "SELECT image, preview_big as preview, name as title, author FROM mgal WHERE id='{$id}'"
        );

        if (empty ($record)) return null;
        $record = $record[0];

        if (empty ($record->image) || empty ($record->preview)) return null;
        
        $record->{$size}   = "//r.zbp.ru/{$size}/{$record->image}";
        $record->image     = $this->PhotoInfo ($record->image);
        $record->preview   = $this->PhotoInfo ($record->preview);
        
        return $record;

    }

    /**
     * Извлечение информации из наименования файла изображения
     */
    protected function PhotoInfo ($filename)
    {
        $result = (object) [];
        if (!empty ($filename)) {
			$s = explode ('.', $filename);
			if (count ($s)==6 && isset ($this->_imageTypes[ $s[5]])) {
                $result->name   = "//r.zbp.ru/{$filename}";
                $result->width  = @base_convert ($s[3], 36, 10);
                $result->height = @base_convert ($s[4], 36, 10);
				$result->imageType  = $s[5];
			}
        }
        return $result;

    }
}