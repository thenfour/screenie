<?
        //
        // sql.php - some very simple utility functions for constructing SQL queries
        // Copyright (c) 2004 Roger Clark
        //

        function sql_insert($tablename, $array)
        {
                foreach ($array as $row => $value)
                {
                        if ($sql1 != "")
                                $sql1 .= ", ";

                        if ($sql2 != "")
                                $sql2 .= ", ";

                        $sql1 .= $row;
                        $sql2 .= "'" . $value . "'";
                }

                return "INSERT INTO $tablename ($sql1) VALUES ($sql2)";
        }

        function sql_update($tablename, $id, $array)
        {
                $sql = "";

                foreach ($array as $row => $value)
                {
                        if ($sql != "")
                                $sql .= ", ";

                        $sql .= "$row = '$value'";
                }

                // NOTE: this assumes your index field is named "id"
                return "UPDATE $tablename SET $sql WHERE id = '$id'";
        }
?>
