<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Thursday, March 27, 2003 08:39:51 PM
	 *
	 * cache.inc - caching functionality
	 */

	function newsys_cache_expire()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");

		if (!@touch("$cache_dir/info-$id"))
			newsys_handle_error("Cannot touch() file; file: $cache_dir/info-$id");

		return;
	}

	function newsys_cache_delete()
	{
		$args		= func_get_args();
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");

		/* This data might not be cached and thus might not exist */
		if (file_exists("$cache_dir/cache-$id"))
			newsys_unlink("$cache_dir/cache-$id");

		if (file_exists("$cache_dir/info-$id"))
			newsys_unlink("$cache_dir/info-$id");

		return;
	}

	function newsys_cache_add_info()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$data		= array_shift($args);
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");

		$fp = @fopen("$cache_dir/info-$id","a");

		if (!$fp)
			newsys_handle_error("Cannot open file; file: $cache_dir/info-$id");

		fputs($fp,$data . "\n");

		fclose($fp);

		return;
	}

	function newsys_cache_remove_info()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$data		= array_shift($args);
		$id		= join('-',$args);
		$cache_dir	= newsys_conf("cache_dir");
		$temp_dir	= newsys_conf("temp_dir");

		# For allowing removal of multiple pieces of data
		if (!is_array($data))
			$data = array($data);

		# Incoming data
		$fp_in	= @fopen("$cache_dir/info-$id","r");

		if (!$fp_in)
			newsys_handle_error("Cannot open file; file=$cache_dir/info-$id");

/*
 * This section requires some extra attention. It
 * is rare that this function will be called with
 * non-existant data, so this lookahead is really
 * potentially wasteful.
 */
		# Make sure some data exists, else we're wasting a lot
		$found		= FALSE;

		while ($line = fgets($fp_in,NEWSYS_BUF_SIZE))
			if (in_array(chop($line),$data))
			{
				$found = TRUE;
				break;
			}

		if (!$found)
		{
			# Data doesn't exist, just quit
			fclose($fp_in);
			return;
		}

		fseek($fp_in,0,SEEK_SET);

		# Temporary storage
		$f_temp	= tempnam($temp_dir,"ns-info");

		if (!$f_temp)
			newsys_handle_error("Cannot create temporary filename with tempnam()");

		# Outgoing data
		$fp_out	= @fopen($f_temp,"w");

		if (!$fp_out)
			newsys_handle_error("Cannot open file; file: $f_temp");

		# Keep all data except our exclusion
		while ($line = fgets($fp_in,NEWSYS_BUF_SIZE))
			if (!in_array(chop($line),$data))
				fputs($fp_out,$line);

		fclose($fp_out);
		fclose($fp_in);

		newsys_move_file($f_temp,"$cache_dir/info-$id");

		return;
	}

	function newsys_cache_is_fresh()
	{
		static $cache		= array();

		$args			= func_get_args();
		$type			= array_shift($args);
		$id			= join('-',$args);
		$cache_dir		= newsys_conf("cache_dir");
		$templates_dir		= newsys_conf("templates_dir");

		# Before we do anything, make sure the file is cached
		if (!file_exists("$cache_dir/cache-$id"))
			return FALSE;

		# Grab the cache's last modification timestamp
		$mod_time		= @filemtime("$cache_dir/cache-$id");

		# Cache last mod time for templates
		if (!array_key_exists("mtime_struct_$type",$cache))
		{
			$cache["mtime_struct_$type"]	= filemtime("$templates_dir/$type.inc");
			$cache["mtime_content_$type"]	= filemtime("$templates_dir/$type.t");
			$cache["dep_$type"]		= @file("$templates_dir/$type.dep");

			$deps				= array();

			# Compile dependencies
			if (is_array($cache["dep_$type"]))
			{
				foreach ($cache["dep_$type"] as $dep)
				{
					# Skip comments/blank lines
					if (preg_match("/^\s*#|^\s*$/",$dep))
						continue;

					# Syntax: rule:arg
					preg_match("/^(\w+):(.*)$/",$dep,$matches);

					# Expand variables
					$matches[2] =	preg_replace
							(
								"/
									{?			# Optional { ... } delimiter
									(
										\\\$		# Dollar
										[a-zA-Z_]	# First char
										[a-zA-Z_0-9]+	# Subsequent chars
										(?:
											\[	# Subscript
											([\"'])	# Quotes
											.*?	# Index
											\\2	# Matching quote
											\]
										)*
									)
									}?
								/xe",
								"$1",
								$matches[2]
							);

					array_push($deps,array('type' => $matches[1],'arg' => $matches[2]));
				}
			}

			$cache["dep_$type"]		= $deps;
		}

		# Check if the templates have changed
		if
		(
			$cache["mtime_struct_$type"] > $mod_time
			||
			$cache["mtime_content_$type"] > $mod_time
		)
			return FALSE;

		# Make sure the dependencies haven't expired
		foreach ($cache["dep_$type"] as $dep)
		{
			switch ($dep["type"])
			{
				# Check last mod time
				case "file":
					if (@filemtime($dep['arg']) > $mod_time)
						return FALSE;

					break;
			}
		}

		# It must be fresh then...
		return TRUE;
	}
?>
