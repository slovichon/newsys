<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Thursday, March 27, 2003 06:41:11 PM
	 *
	 * isr.inc - internal structure representation
	 * communication routines
	 */

	function newsys_isr_has_field($table,$field)
	{
		switch ($table)
		{
			case "newsys_users":
			{
				switch ($field)
				{
					case "pref_time_format":
					case "pref_result_limit":
					case "pref_page_limit":
					case "pref_overview_break":
						return TRUE;
						
					default:
						return FALSE;
				}
			}

			default:
			{
				return FALSE;
			}
		}
	}
?>
