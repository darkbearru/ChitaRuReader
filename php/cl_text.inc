<?
/**
 * Получение списка текстов
 */
require_once ("cl_command.inc");

class TextClass extends Command
{
    private $_level;
    private $_photos;

    public function Run ()
    {
        $_id = intval ($this->_parameters[0]);
        if (empty ($_id)) $this->Error ("Не указан ID текста");

        $record = $this->_db->qry_c (
            "SELECT id, name, txt_typo, tags, news_date, news_time, author, photo_id, opinions FROM news WHERE flags&1 and id='{$_id}'"
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
        // Получаем все данные по титульной фотографии
        if (!empty ($record->photo_id)) {
            $record->titlePhoto = $this->Photo ($record->photo_id);
            unset ($record->photo_id);
        }

        // Обрабатываем внутренние тэги проекта
        $record = $this->MetaTags ($record, "txt_typo");
        $record->text = $record->txt_typo;
        unset ($record->txt_typo);        

        // Деколируем список тэгов
        if (!empty ($record->tags)) {
            $record->tags = json_decode ($record->tags);
        }

        // Формируем ссылку на получение комментариев, если они есть
        if (!empty ($record->opinions)) {
            $record->commentsURL = "/api-v2/comments/{$record->id}/{$record->opinions}/";
        }
        unset ($record->opinions);

        // Получение timestamp на основе даты и времени новости
        $record->timeStamp = $this->GetTimeStamp ($record->news_date . ' ' . $record->news_time);
        unset ($record->news_date);
        unset ($record->news_time);
        return $record;
    }

    
    /**
     * Обработка мета-тегов в тексте статьи.
     * В тексте статьи присутствуют различного назначения тэги
     * вида [% TagName %](...[%/TagName%])
     * Нас интересуют только теги gallery или media, используемые для вставки картинок,
     * Остальные теги удаляем
     */
    protected function MetaTags ($record, $textField = 'txt')
    {
        $this->_photos = [];

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
        if (!empty ($this->_photos)) {
            $record->photos = $this->_photos;
        }
        return $record;
    }

    // Расстановка параграфов
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
    protected function GalleryTag ($params, $text)
    {
        if (empty ($params)) return '';
        if (empty ($params->id)) return '';

        // Определяем что нам было передано iD папки или id фотографий
        $records = $this->getGalleryRecs ($params);
        if (count ($records) == 1) {
            if ($records[0]->kind == 1) {
                $params->upid = $params->id;
                unset ($params->id);
                $records = $this->getGalleryRecs ($params);
            }
        }

        foreach ($records as $record) {
            //$record->{$size}   = "//r.zbp.ru/{$size}/{$record->image}";
            $record->image          = $this->PhotoInfo ($record->image);
            $record->preview_big    = $this->PhotoInfo ($record->preview_big);
        }

        $this->_photos[] = $records;

        $_idx = count ($this->_photos);
        $params->source = str_replace ('"', '\"', $params->source);

        return "{{ gallery_idx{$_idx} }} {$text}";
        //$text = preg_replace ("/\[\%(gallery|media)\s{0,}{$params->source}.*?\%\]/uism", "{{ gallery_idx{$_idx} }}", $text);

        return $text;
    }

    protected function getGalleryRecs ($params)
    {
        $_db = DB_COMMON ();
        if (!empty ($params->id)) {
            $_where = "a.id in ({$params->id})";
            $_order = "locate(concat(',',a.id,','),',{$params->id},')";
        } elseif (!empty ($params->upid)) {
            $_where = "a.upid = '$params->upid'";
            $_order = "ord";
        } else {
            return [];
        }
        
        $records = $_db->qry_c ("SELECT a.id, a.upid, a.kind, a.name as title, 
                if(length(trim(a.author)), trim(a.author), trim(b.author)) author, a.image,
                a.preview_big, a.rating 
            FROM mgal a left join mgal b on a.upid=b.id
            WHERE a.flags&1 and {$_where}
            ORDER BY {$_order}
        ");

        return $records;
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