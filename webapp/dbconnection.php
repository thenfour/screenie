<?
        //
        // dbconnection.php - very thin mysql wrapper class
        // Copyright (c) 2004 Roger Clark
        //

        class dbconnection
        {
                private $hostname = "";
                private $username = "";
                private $password = "";
                private $database = "";

                private $link = 0;
                private $lastresult = 0;

                function __construct($hostname, $username, $password)
                {
                        $this->connect($hostname, $username, $password);
                }

                function __destruct()
                {
                        if ($link != 0)
                                $this->disconnect();
                }

                function connect($hostname, $username, $password)
                {
                        $this->hostname = $hostname;
                        $this->username = $username;
                        $this->password = $password;

                        $this->link = @mysql_pconnect($this->hostname, $this->username, $this->password);

                        return ($this->link != false);
                }

                function disconnect($hostname, $username, $password)
                {
                        // since we pconnect()ed, this doesn't do anything. it might later on,
                        // if i ever need to modify this interface for another database API.

                        return true;
                }

                function select_db($database)
                {
                        $this->database = $database;

                        @mysql_select_db($this->database, $this->link);
                }

                function query($query)
                {
                        return ($this->lastresult = @mysql_query($query, $this->link));
                }

                function affected_rows()
                {
                        return @mysql_affected_rows($this->link);
                }

                function insert_id()
                {
                        return @mysql_insert_id($this->link);
                }

                function fetch_array($result = 0)
                {
                        if ($result == 0)
                                $result = $this->lastresult;

                        return @mysql_fetch_array($result);
                }

                function num_rows($result = 0)
                {
                        if ($result == 0)
                                $result = $this->lastresult;

                        return @mysql_num_rows($result);
                }

                function get_result($result = 0, $row = 0)
                {
                        if ($result == 0)
                                $result = $this->lastresult;

                        return @mysql_result($result, $row);
                }

                function get_error()
                {
                        return mysql_error($this->link);
                }

                function connection()
                {
                        return $this->link;
                }

                // this function encapsulates the all-too-common action of querying
                // the database and fetching every single resulting row. for anything
                // potentially complex, though, you should query() manually.

                function query_results($querystring)
                {
                        $result = $this->query($querystring);

                        if ($this->num_rows($result))
                        {
                                $rows = array();

                                while ($row = $this->fetch_array($result))
                                        array_push($rows, $row);

                                return $rows;
                        }

                        return false;
                }
        }

?>

