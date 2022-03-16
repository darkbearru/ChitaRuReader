<?
/**
 * Получение списка текстов
 */
require_once ("cl_command.inc");

class CommentsClass extends Command
{
    private $_id;

    public function Run ()
    {
        $_id    = intval ($this->_parameters[0]);
        $_upid  = (!empty ($this->_parameters[1]) ? intval ($this->_parameters[1]) : 0);

        $this->_id = $_id;
        if (empty ($_upid) || empty ($_id)) $this->Error ('Ошибка в переданных ID');

        $records = $this->_db->qry_c (
            "SELECT id, name, msg, gal, kind, created, cnt_nodes, cnt_leafs 
            FROM opinions 
            WHERE flags&1 and upid='{$_upid}' ORDER BY created"
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
            $record = $this->PostProcessRecord ($record);
        }
        return $records;
    }

    private function PostProcessRecord ($record)
    {
        $record->answerURL = false;
        if (($record->kind == 1) && (!empty ($record->cnt_nodes) || !empty ($record->cnt_leafs))) {
            $record->answersURL = "/api-v2/comments/{$this->_id}/{$record->id}/";
        }
        $record->timeStamp = $this->GetTimeStamp ($record->created);
        
        unset ($record->created);
        unset ($record->cnt_nodes);
        unset ($record->cnt_leafs);
        unset ($record->kind);
        return $record;
    }
}