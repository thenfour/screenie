<?
/*
 * $page_title - page title (section name)
 * $page_blurb - small portion of text in the upper right
 */
 
	$blurbs = array(
		"Try the <a href=\"/try.php\">demo version</a> of Screenie for free. It won't expire, so you have plenty of time to see how convenient it really is.",
		"Screenie is great for software developers and those who write documentation for applications. The inline cropping feature helps to streamline your workflow.",
		"Set up a link on your website or blog, so your visitors can see what you're up to! It's easy with Screenie's automatic FTP upload feature!",
		"Use Screenie to quickly show images to your friends over chat networks. Just set up an FTP Upload destination, crop the screenshot, and send the URL!",
		"Need pixel-perfect accuracy when cropping a screenshot? No problem! Screenie's cropping window can has two precision levels to satisfy perfectionists."
	);

	$page_blurb = $blurbs[rand(0, count($blurbs) - 1)];
?>

<html>
	<head>
		<title>Screenie - Screen Capture Utility - <?= $page_title ?></title>
		<link rel="stylesheet" href="screenie.css"/>

<?
		if ($page_refresh != "")
		{
?>
		<meta http-equiv="refresh" content="1; URL=<?= $page_refresh; ?>"/>
<?
		}
?>
	</head>

	<body>
		<table width="65%" align="center">
			<tr>
				<td class="content">
					<table cellpadding="5">
						<tr>
							<td class="header-logo">
								<img src="/images/logo.gif"/>
							</td>

							<td class="header-blurb">
								<?= $page_blurb ?>
							</td>
						</tr>
					</table>

					<br/>

					<div class="menu">
						<a href="/index.php">Home</a> - <a href="/try.php">Try It!</a> -
						<a href="/features.php">Features</a> - <a href="/buy.php">Buy It!</a>
					</div>
					
					<p>
					<!-- end header -->