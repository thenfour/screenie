<?
	$page_title = "Buy It";
	$page_blurb = "...";

	include("header.inc");
?>

        <p>
			You can purchase the full version of Screenie for only $15.00 USD.
			With the purchase of your Screenie license, you will be entitled to
			any future updates or changes to Screenie.
		</p>

        <p>
			To reduce your costs, Screenie is delivered electronically. Soon
			after your purchase has been completed, you will receive instructions
			on obtaining your copy of Screenie -- no shipping charges required.
		</p>

		<p>
			If you have any questions, feel free to contact us at
			<a href="mailto:fromahuman@hotmail.com">fromahuman@hotmail.com</a>.
		</p>

		<form action="buy2.php" method="post">
			<p align="center">
				<input type="hidden" name="cmd" value="_xclick" ID="Hidden1"/>
				<input type="hidden" name="business" value="carlco@gmail.com" ID="Hidden2"/>
				<input type="hidden" name="item_name" value="Screenie (1 user license)" ID="Hidden3"/>
				<input type="hidden" name="amount" value="15.00" ID="Hidden4"/>
				<input type="hidden" name="no_note" value="1" ID="Hidden5"/>
				<input type="hidden" name="currency_code" value="USD" ID="Hidden6"/>
				<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-butcc.gif" border="0"
					name="submit" alt="Make payments with PayPal - it's fast, free and secure!" ID="Image1"/>
	
				<br/>
	
				$15.00 USD
			</p>
		</form>

<?
	include("footer.inc");
?>