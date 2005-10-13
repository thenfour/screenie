<?
	$page_title = "Buy It";
	$page_blurb = "...";

	include("header.inc");
?>

		<p>
			Payment for Screenie is accepted via eBay's PayPal service.  Click
			"Buy Now" to be taken to PayPal's website to complete the transaction.
		</p>



		<form action="https://www.paypal.com/cgi-bin/webscr" method="post" ID="Form1">
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
		
        <span class="bigtext">Terms and Conditions</span>

        <p>By downloading and/or using Screenie, you agree that Screenie and all services are provided
        on an "as is" and "as available" basis without warranty of any kind, either expressed or implied.
        You further hereby agree to hold the creators of Screenie, screenie.com, and all affiliates of
        Screenie and screenie.com harmless for any and all damages which may result from use of or
        reliance upon any of the goods or services ordered or secured by you in using this site.
        This waiver includes, but is not limited to, service loss, program malfunction, and business
        or economic loss incurred in connection with the use of any good or service featured on this site.</p>

<?
	include("footer.inc");
?>