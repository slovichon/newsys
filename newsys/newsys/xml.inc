<?php
	/*
	 * Newsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Friday, March 14, 2003 02:53:46 PM
	 *
	 * xml.inc - xml manipulation
	 */

	# XML control types
	define("NEWSYS_XML_STORIES",1,TRUE);

	function newsys_xml_update($type,$data,$doc = NULL)
	{
		global $php_errormsg;

		$xml_files	= newsys_conf("xml_files");
		$xml_file	= $xml_files[$type];

		if (!$doc)
		{
			$doc = @domxml_open_file($xml_file);

			# Altering something that doesn't exist isn't supposed to happen
			if (!$doc)
				newsys_handle_error("XML file can't be loaded; file: $xml_file");
		}

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$obj_rdf = $doc->document_element();

				foreach ($obj_rdf->child_nodes() as $obj_item)
				{
					# We *should* only need to update the item,
					# since the URL *shouldn't* have changed
					if ($obj_item->node_name() == "item")
					{
						$attr = $obj_item->get_attribute('rdf:about');
// START FLAKY DOMXML HACK
if (!$attr)
	$attr = $obj_item->get_attribute("about");
// END FLAKY DOMXML HACK

						if (!preg_match("/\?story_id=(\d+)$/",$attr,$matches))
							newsys_handle_error("Can't find story ID from XML file during story update; file: $xml_file");

						if ($matches[1] == $data["story_id"])
						{
							foreach ($obj_item->child_nodes() as $child)
							{
								$children = $child->child_nodes();

								switch ($child->node_name())
								{
									#case "newsys:timestamp":
									case 'timestamp':
										$children = $child->child_nodes();
										$child->remove_child($children[0]); /* Should be text node */
										$child->append_child($doc->create_text_node($data["date"]));
										break;

									case "description":
										$children = $child->child_nodes();
										$child->remove_child($children[0]); /* Should be text node */
										$child->append_child($doc->create_text_node($data["overview"]));
										break;

									case "title":
										$children = $child->child_nodes();
										$child->remove_child($children[0]); /* Should be text node */
										$child->append_child($doc->create_text_node($data["headline"]));
										break;
								}
							}

							break;
						}
					}
				}

				break;
			}

			default:
			{
				newsys_handle_error("Unrecognized newsys XML type; type: $type");
			}
		}

		return $doc;
	}

	function newsys_xml_write($type,$doc)
	{
		global $php_errormsg;

		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$xml_file	= $xml_files[$type];

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$data		= $doc->dump_mem(TRUE);

		file_dump($xml_file,$data);

		return;
	}

	function newsys_xml_add($type,$data,$doc = NULL)
	{
		global $php_errormsg;

		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$xml_file	= $xml_files[$type];

		if (!$doc)
		{
			$doc = @domxml_open_file($xml_file);

			# Create a new document if no such one exists
			if (!$doc)
				$doc = newsys_xml_setup($type);
		}

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$link		= newsys_build_path(NEWSYS_PATH_ABS) . "/view.php?story_id=" . $data["story_id"];

				$obj_rdf	= $doc->document_element();

				$obj_item	= $doc->create_element("item");
				$obj_item->set_attribute("rdf:about",	$link);

				$obj_title	= $doc->create_element("title");
				$obj_link	= $doc->create_element("link");
				$obj_ns_ts	= $doc->create_element("newsys:timestamp");
				$obj_desc	= $doc->create_element("description");

				$obj_title->append_child($doc->create_text_node($data['headline']));
				$obj_link->append_child($doc->create_text_node($link));
				$obj_ns_ts->append_child($doc->create_text_node($data['date']));
				$obj_desc->append_child($doc->create_text_node($data['overview']));

				$obj_item->append_child($obj_title);
				$obj_item->append_child($obj_link);
				$obj_item->append_child($obj_ns_ts);
				$obj_item->append_child($obj_desc);

				$obj_rdf->append_child($obj_item);

				# Remove the old stories
				$children		= $obj_rdf->child_nodes();
				$oldest_date		= date("YmdHis");	# Every story should be before now
				$num_stories		= 0;
				$removed_stories	= array();

				# Count them up
				foreach ($children as $obj_item)
					if ($obj_item->node_name() == "item")
						$num_stories++;

				while ($num_stories > newsys_conf("result_limit"))
				{
					$oldest_story = NULL;

					# Then find the oldest and remove it
					foreach ($children as $obj_item)
					{
						if ($obj_item->node_name() == "item")
						{
							# Check if this is the oldest story
							foreach ($obj_item->child_nodes() as $child)
							{
/*
								$grand_children = $child->child_nodes();

								...
									# Should be text node
									$grand_children[0]->node_value() < $oldest_date
*/
								if
								(
									/* DOMXML is a bit flaky*/
									(/*$child->node_name() == "timestamp" ||*/
									$child->node_name() == "newsys:timestamp") &&
									$child->get_content() < $oldest_date
								)
								{
									$oldest_story	= $obj_item;

//									$oldest_date	= $grand_children[0]->node_value();

									$oldest_date	= $child->get_content();

									$attr		= $obj_item->get_attribute("rdf:about");

// FLAKY DOMXML FIX
if (!$attr)
	$attr = $obj_item->get_attribute("about");
// END FLAKY DOMXML FIX
									if (!preg_match("/\?story_id=(\d+)$/",$attr,$matches))
										newsys_handle_error("Can't find oldest story ID from XML file during story add; file: $xml_file");

									array_push($removed_stories,$matches[1]);

									break;
								}
							}
						}
					}

					if (!$oldest_story)
						newsys_handle_error("Couldn't find oldest story from XML file during story add; file: $xml_file");

					$obj_rdf->remove_child($oldest_story);

					$num_stories--;
				}

				$obj_rdf_seq = NULL;

				# Now add our entry to the <channel> element, and remove the others
				foreach ($children as $channel)
				{
					if ($channel->node_name() == "channel")
					{
						# Find the <items> elements
						foreach ($channel->child_nodes() as $obj_items)
						{
							if ($obj_items->node_name() == "items")
							{
								foreach ($obj_items->child_nodes() as $obj_rdf_seq)
								{
									if ($obj_rdf_seq->node_name() == "rdf:Seq" /*||
									    $obj_rdf_seq->node_name() == "Seq"*/) /* Flake DOMXML fix */
									{
										if ($obj_rdf_seq->has_child_nodes())
										{
											$seq_children = $obj_rdf_seq->child_nodes();

											foreach ($seq_children as $obj_rdf_li)
											{
												if (!preg_match("/\?story_id=(\d+)$/",$obj_rdf_li->get_attribute("resource"),$matches))
													newsys_handle_error("Could not find oldest story ID from XML file during story add; file: $xml_file");

												$id = $matches[1];

												if (in_array($id,$removed_stories))
													$obj_rdf_seq->remove_child($obj_rdf_li);
											}
										}

										break;
									}
								}

								break;
							}
						}

						break;
					}
				}

				if (!$obj_rdf_seq)
					newsys_handle_error("Could not find oldest story from XML file during story add; file: $xml_file");

				$obj_rdf_li = $doc->create_element("rdf:li");
				$obj_rdf_li->set_attribute("resource",$link);
				$obj_rdf_seq->append_child($obj_rdf_li);

				break;
			}

			default:
			{
				newsys_handle_error("Unrecognized newsys XML type; type=$type");
			}
		}

		return $doc;
	}

	function newsys_xml_remove($type,$data,$doc = NULL)
	{
		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$xml_file	= $xml_files[$type];

		if (!$doc)
		{
			$doc = @domxml_open_file($xml_file);

			if (!$doc)
				newsys_handle_error("XML file can't be loaded; file: $xml_file");
		}

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$obj_rdf = $doc->document_element();

				foreach ($obj_rdf->child_nodes() as $child)
				{
					if ($child->node_name() == "channel")
					{
						foreach ($child->child_nodes() as $obj_items)
						{
							if ($obj_items->node_name() == "items")
							{
								foreach ($obj_items->child_nodes() as $obj_rdf_seq)
								{
									if ($obj_rdf_seq->node_name() == "rdf:Seq" && is_array($obj_rdf_seq->child_nodes()))
									{
										foreach ($obj_rdf_seq->child_nodes() as $obj_rdf_li)
										{
											if (!preg_match("/\?story_id=(\d+)$/",$obj_rdf_li->get_attribute("resource"),$matches))
												newsys_handle_error("Can't find story ID from XML file during story removal; file: $xml_file");

											if ($matches[1] == $data["story_id"])
											{
												$obj_rdf_seq->remove_child($obj_rdf_li);

												break;
											}
										}

										break;
									}
								}

								break;
							}
						}
					} elseif ($child->node_name() == "item") {

						$attr = $child->get_attribute("rdf:about");

// START FLAKY DOMXML HACK
if (!$attr)
	$attr = $child->get_attribute("about"); // no namespace
// END FLAKY DOMXML HACK

						if (!preg_match("/\?story_id=(\d+)$/",$attr,$matches))
							newsys_handle_error("Can't find story ID from XML file during story removal; file: $xml_file");

						if ($matches[1] == $data["story_id"])
							$obj_rdf->remove_child($child);
					}
				}

				break;
			}

			default:
			{
				newsys_handle_error("Unknown XML type; type: $type");
			}
		}

		return $doc;
	}

	function newsys_xml_setup($type)
	{
		$xml_files	= newsys_conf("xml_files");

		if (!array_key_exists($type,$xml_files))
			newsys_handle_error("Invalid XML file type; type: $type");

		$doc = domxml_new_doc("1.0");

		switch ($type)
		{
			case NEWSYS_XML_STORIES:
			{
				$obj_rdf	= $doc->create_element("rdf:RDF");
				$obj_rdf->set_attribute("xmlns:rdf",		"http://www.w3.org/1999/02/22-rdf-syntax-ns#");
				$obj_rdf->set_attribute("xmlns",		"http://purl.org/rss/1.0/");
				$obj_rdf->set_attribute("xmlns:newsys",		"http://www.easyphp.net/projects/newsys/pub/newsys-ns/newsys-1.0.rdf");

				$obj_channel	= $doc->create_element("channel");
				$obj_channel->set_attribute("rdf:about",	"http://www.xml.com/xml/news.rss");

				$obj_title	= $doc->create_element("title");
				$obj_title->append_child($doc->create_text_node(newsys_conf("site_name")));

				$obj_link	= $doc->create_element("link");
				$obj_link->append_child($doc->create_text_node(newsys_conf("site_uri") . "/"));

				$obj_desc	= $doc->create_element("description");
				$obj_desc->append_child($doc->create_text_node(newsys_conf("site_desc")));

				$obj_image	= $doc->create_element("image");
				$obj_image->set_attribute("rdf:resource",newsys_conf("rss_icon"));

				$obj_items	= $doc->create_element("items");

				$obj_rdf_seq	= $doc->create_element("rdf:Seq");

				$obj_items->append_child($obj_rdf_seq);

				$obj_channel->append_child($obj_title);
				$obj_channel->append_child($obj_link);
				$obj_channel->append_child($obj_desc);
				$obj_channel->append_child($obj_image);
				$obj_channel->append_child($obj_items);

				$obj_rdf->append_child($obj_channel);

				$doc->append_child($obj_rdf);

				break;
			}

			default:
			{
				newsys_handle_error("Unrecognized XML type; type: $type");
			}
		}

		return $doc;
	}
?>
