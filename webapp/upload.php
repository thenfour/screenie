<?
        require_once("config.php");
        require_once("dbconnection.php");
        require_once("sqlutil.php");

        $db_conn = new dbconnection($SCREENIE_CONFIG["db.dsn.host"],
                $SCREENIE_CONFIG["db.dsn.username"], $SCREENIE_CONFIG["db.dsn.password"]);
        $db_conn->select_db($SCREENIE_CONFIG["db.dsn.database"]);

	$acct_password = $_POST["password"];
	$acct_username = $_POST["username"];

	function fetch_user($db, $table, $username)
	{
		$result = $db->query("SELECT * FROM $table WHERE username = '$username'");

		if ($db->num_rows($result))
		{
			$user = $db->fetch_array($result);
			return $user;
		}

		return false;
	}

	if ($user = fetch_user($db_conn, $SCREENIE_CONFIG["db.table.users"], $acct_username))
	{
		if (md5($acct_password) == $user["passhash"])
		{
			$upload_dir = $SCREENIE_CONFIG["path.base"] . "upload/" . $user["dirname"] . "/";
			$upload_file = $upload_dir . $_FILES["screenshot"]["name"];

			if (move_uploaded_file($_FILES["screenshot"]["tmp_name"], $upload_file))
			{
				echo $SCREENIE_CONFIG["url.base"] . "upload/" . $user["dirname"] . "/" . $_FILES["screenshot"]["name"];
			}
			else
			{
				echo "sry, upload failured";
			}
		}
		else
		{
			echo "sry, password incorrect";
		}
	}
	else
	{
		echo "sry, can't find ur user";
	}
?>
