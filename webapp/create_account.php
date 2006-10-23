<?
        //
        // create_account.php - form action for creating accounts in database
        //

	require_once("config.php");
	require_once("dbconnection.php");
	require_once("sqlutil.php");

	$db_conn = new dbconnection($SCREENIE_CONFIG["db.dsn.host"],
		$SCREENIE_CONFIG["db.dsn.username"], $SCREENIE_CONFIG["db.dsn.password"]);
	$db_conn->select_db($SCREENIE_CONFIG["db.dsn.database"]);

	// inserts a new user into a db table.
	// returns -1 on failure
	function insert_user($db, $table, $user)
	{
		$id = -1;

		$sql_query = sql_insert($table, $user);
		$result = $db->query($sql_query);
	
		if ($db->affected_rows())
			$id = $db->insert_id();

		return $id;
	}

	function user_exists($db, $table, $username)
	{
		$sql_query = "SELECT COUNT(*) FROM $table WHERE username = '$username'";
		$result = $db->query($sql_query);

		if ($db->get_result($result))
			return true;

		return false;
	}

	function verify_email($email)
	{
		// parse with regex
		return true;
	}

        echo $acct_username = $_POST["username"];
        echo $acct_password = $_POST["password"];
        echo $acct_password2 = $_POST["password2"];
        echo $acct_email = $_POST["email"];

	if (($acct_username == "") || ($acct_password == "") ||
		($acct_password2 == "") || ($acct_email == "")
	{
		echo "sry, you forgot to fill in a field. try again";
		die;
	}

        if ($acct_password != $acct_password2)
        {
                echo "sry, passwords don't match";
                die;
        }

        if (!verify_email($acct_email))
        {
                echo "sry, email is invalid";
                die;
        }

	if (user_exists($db_conn, $SCREENIE_CONFIG["db.table.users"], $acct_username))
	{
		echo "sry, user already exists";
		die;
	}

	if (file_exists($SCREENIE_CONFIG["path.base"] . "upload/" . $acct_username))
	{
		echo "sry, upload directory already exists";
		die;
	}

	$user = array(
		"username" => $acct_username,
		"passhash" => md5($acct_password),
		"email" => $acct_email,
		"dirname" => "$acct_username"
	);

	if (!insert_user($db_conn, $SCREENIE_CONFIG["db.table.users"], $user))
	{
		echo "sry, user can't be created. error = ";
		echo $db_conn->get_error();
	}

	mkdir($SCREENIE_CONFIG["path.base"] . "upload/" . $acct_username, 0777);
?>

