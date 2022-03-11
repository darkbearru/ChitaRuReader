<?
/**
 * Получение списка текстов
 */
require_once ("cl_command.inc");

class ListClass extends Command
{
    private $_on_page = 20;

    public function Run ()
    {
        $_page  = (empty ($this->_parameters) ? 0 : intval ($this->_parameters[0]));
        $_shift = $_page * $this->_on_page; 

        $records = $this->_db->qry_c (
            "SELECT id, name, anons, tags, news_date, news_time, author, photo_id 
            FROM news WHERE flags&1 
            ORDER BY news_date desc, news_time desc
            LIMIT {$_shift}, {$this->_on_page}"
        );
        $records = $this->PostProcess ($records);
        $this->JSON ($records);
    }

    /**
     * Обработка записей списка для дальнейшего вывода
     */
    protected function PostProcess ($records)
    {
        foreach ($records as $record) {
            $record->id        = (int) $record->id;
            $record->photo_id  = (int) $record->photo_id;
            if (!empty ($record->tags)) {
                $record->tags = array_keys (json_decode ($record->tags, 'array'));
            }
            $record->photo = $this->Photo ($record->photo_id);
        }
        return $records;
    }
}