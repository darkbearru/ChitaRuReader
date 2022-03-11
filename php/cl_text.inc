<?
/**
 * Получение списка текстов
 */
require_once ("cl_command.inc");

class TextClass extends Command
{
    private $_level;

    public function Run ()
    {
        $_id = intval ($this->_parameters[0]);
        if (empty ($_id)) $this->Error ("Не указан ID текста");

        $record = $this->_db->qry_c (
            "SELECT id, name, txt_typo, tags, news_date, news_time, author, photo_id FROM news WHERE flags&1 and id='{$_id}'"
        );
        if (empty ($record)) $this->Error ("Текст с указанным ID не существует");

        $record = $record[0];
        $record = $this->PostProcess ($record);

        $this->JSON ($record);
    }

    /**
     * Обработка записей списка для дальнейшего вывода
     */
    protected function PostProcess ($record)
    {
        $record = $this->MetaTags ($record, "txt_typo");
        if (!empty ($record->tags)) {
            $record->tags = json_decode ($record->tags);
        }
        return $record;
    }

    
    /**
     * Обработка мета-тегов в тексте статьи
     */
    protected function MetaTags ($record, $textField = 'txt')
    {
        $_text  = $record->{$textField};
        $_text  = $this->TextFormat ($_text);

        $this->_level = 0;
        $_text  = preg_replace_callback (
            "/\[\%\s*([\$]{0,1}[a-zA-Zа-яА-Я0-9-]+)\s*(.*?)\s*\%\]((.*?)(\[%\s*\/\g{1}\s*\%\]))?/uis", 
            function ($matches) {
                return $this->TagReplace ($matches);
            }, 
            $_text);

        $_text  = str_replace ('</div></p>', '</p></div>', $_text);
        $_text  = str_replace ('<p></p>', '', $_text);
        
        $record->{$textField} = $_text;
        return $record;
    }

    protected function TextFormat ($text)
    {
		$text	= preg_replace ('/\r\n/', "\n", $text);
		$text	= preg_replace ('/^\r\n/sm', "\n", $text);

		// Расставляем параграфы
		$text	= preg_replace ('/^([^\n\<\[\]].*?)$/sm', '<p>\1</p>', $text);
		$text	= preg_replace ('/^([^\n\<\[\]].*?)(?=\n\n)/sm', '<p>\1</p>', $text);
		$text	= preg_replace ('/(?<=\n\n)([^\n\<\[\]].*?)(?=\n\n)/s', '<p>\1</p>', $text);
		$text	= preg_replace ('/(?<=\n\n)([^\n\<\[\]].*?)$/s', '<p>\1</p>', $text);
		$text	= preg_replace ('/([^\n\>\[\]])\n([^\n])/', "$1<br />\n$2", $text);

        return $text;
    }

    protected function TagReplace ($matches)
    {
        $_text  = false;
        $_tag   = strtolower ($matches[1]);
        $_func  = $_tag.'Tag';
        if (isset ($matches[4])) $_text = $matches[4];
        // Извлекаем параметры
        $_params = $this->ExtractParams ($matches[2]);
        $this->_level++;
        if (($this->_level<2) && !empty ($_text)) {
            $_text  = preg_replace_callback (
                "/\[\%\s*([\$]{0,1}[a-zA-Zа-яА-Я0-9-]+)\s*(.*?)\s*\%\]((.*?)(\[%\s*\/\g{1}\s*\%\]))?/uis", 
                function ($matches) {
                    return $this->TagReplace ($matches);
                }, 
                $_text);
        }
        $this->_level--;
        
        // Проверяем случай, если передана переменная
        if (method_exists ($this, $_func)) {
            // Проверяем есть ли фукнция обработки тэга
            return $this->$_func ((object) $_params, $_text);
        }    
    }

    protected function ExtractParams ($str) 
    {
        preg_match_all('/([a-zA-Z_0-9]+)?[[:space:]]*(=[[:space:]]*["“\']{1}(([.]|[^"])*)?["”\']{1})?/uis', $str, $p, PREG_PATTERN_ORDER);
        $params = [];
        foreach ($p[1] as $k => $v) {
            if (!empty($v)) {
                $params[$v] = $p[3][$k] ? $p[3][$k] : 1;
            }
        }
        if (empty($params['source'])) {
            $params['source'] = $str;
        }
        
        return $params;
    }
    


    /**
     * Извлекаем галереи из текста
     */
    protected function GalleryTag ($params)
    {
        return $record;
    }
    
    function newsExtractParams ($str)
    {
        preg_match_all('/([a-zA-Z_0-9]+)?[[:space:]]*(=[[:space:]]*["“\']{1}(([.]|[^"])*)?["”\']{1})?/uis', $str, $p, PREG_PATTERN_ORDER);
        $_params = [];
        foreach ($p[1] as $k => $v) {
            if (!empty($v)) {
                $_params[$v] = $p[3][$k] ? $p[3][$k] : 1;
            }
        }	
        return $_params;
    }
    
}